#include "Application.h"

#include "Window.h"

#include <fstream>
#include <iostream>
#include <unordered_map>

namespace gui {

    static const std::string DefaultStyleJson =
#include "generated/DefaultStyle.h"
        ;

    Application::Application(Backend* backend) {
        m_backend = std::unique_ptr<Backend>(backend);
    }

    int Application::Start() {
        m_style = Json::parse(DefaultStyleJson);
        ProcessStyle(m_style);

        if (!m_backend) {
            return 1;
        }
        if (!m_backend->Init()) {
            return 1;
        }

        // Build all windows
        for (auto& win : m_windows) {
            auto* root = win->OnBuild()(*win);
            win->SetRoot(root);
        }

        m_running = true;
        while (m_running) {
            m_backend->PollEvents(*this);

            m_windows.erase(
                std::remove_if(
                    m_windows.begin(),
                    m_windows.end(),
                    [](const auto& win) { return win->m_closeRequested; }
                ),
                m_windows.end()
            );

            for (auto& win : m_windows) {
                win->Update();
            }
        }

        m_windows.clear();
        m_backend->Shutdown();

        return 0;
    }

    Window* Application::CreateWindow(const WindowConfig& config) {
        m_windows.push_back(std::make_unique<Window>(config));
        InitWindow(m_windows.back().get());
        return m_windows.back().get();
    }

    void Application::SetClipboard(const std::string& str) {
        m_backend->SetClipboardText(str);
    }

    std::string Application::GetClipboard() {
        return m_backend->GetClipboardText();
    }

    void Application::ResetStyle() {
        LoadThemeFromString(DefaultStyleJson);
    }

    void Application::LoadTheme(const std::string& themePath) {
        try {
            m_style = Json::parse(std::ifstream(themePath));
            ProcessStyle(m_style);
            for (const auto& win : m_windows) {
                win->RequestRedrawAll();
            }
        } catch (const std::exception& e) {
            // If parsing fails, keep the existing style and print an error.
            std::cerr << "Failed to load theme: " << e.what() << std::endl;
        }
    }

    void Application::LoadThemeFromString(const std::string& themeJson) {
        try {
            m_style = Json::parse(themeJson);
            ProcessStyle(m_style);
            for (const auto& win : m_windows) {
                win->RequestRedrawAll();
            }
        } catch (const std::exception& e) {
            // If parsing fails, keep the existing style and print an error.
            std::cerr << "Failed to load theme: " << e.what() << std::endl;
        }
    }

    Window* Application::FindWindow(WindowId id) {
        for (auto& window : m_windows) {
            if (window->GetId() == id) {
                return window.get();
            }
        }
        return nullptr;
    }

    // BackendEventSink callbacks

    void Application::OnQuit() {
        m_running = false;
    }

    void Application::OnWindowCloseRequested(WindowId id) {
        if (auto* win = FindWindow(id)) {
            win->Close();
        }
    }

    void Application::OnWindowExposed(WindowId id) {
        if (auto* win = FindWindow(id)) {
            win->Update();
        }
    }

    void Application::OnWindowResized(WindowId id) {
        if (auto* win = FindWindow(id)) {
            auto sz = win->GetSize();
            win->Resize(sz.w, sz.h, false);
        }
    }

    void Application::OnMouseButtonDown(WindowId id, int x, int y, MouseButton button) {
        if (auto* win = FindWindow(id)) {
            win->DispatchEvent<MouseEvent>(x, y, button, true);
        }
    }

    void Application::OnMouseButtonUp(WindowId id, int x, int y, MouseButton button) {
        if (auto* win = FindWindow(id)) {
            win->DispatchEvent<MouseEvent>(x, y, button, false);
        }
    }

    void Application::OnMouseMotion(WindowId id, int x, int y, MouseButton button) {
        if (auto* win = FindWindow(id)) {
            win->DispatchEvent<MotionEvent>(x, y, button);
        }
    }

    void Application::OnMouseWheel(WindowId id, float dx, float dy, int mouseX, int mouseY) {
        if (auto* win = FindWindow(id)) {
            win->DispatchEvent<ScrollEvent>(dx, dy, mouseX, mouseY);
        }
    }

    void Application::OnKeyDown(WindowId id, Key key, ModifierState mod) {
        if (auto* win = FindWindow(id)) {
            win->DispatchEvent<KeyEvent>(key, mod, true);
        }
    }

    void Application::OnKeyUp(WindowId id, Key key, ModifierState mod) {
        if (auto* win = FindWindow(id)) {
            win->DispatchEvent<KeyEvent>(key, mod, false);
        }
    }

    void Application::OnTextInput(WindowId id, char c) {
        if (auto* win = FindWindow(id)) {
            win->DispatchEvent<TextInputEvent>(c);
        }
    }

    void Application::ProcessStyle(Json& style) {
        auto fnVisitVariables = [&](auto&& self, Json& node) -> void {
            if (node.is_object()) {
                for (auto& [key, value] : node.items()) {
                    if (value.is_string() && value.get<std::string>().starts_with("$")) {
                        std::string varName = value.get<std::string>().substr(1);
                        auto vars = style["$variables"];
                        if (vars.is_object() && vars.contains(varName)) {
                            value = vars[varName];
                        }
                    }
                    self(self, value);
                }
            } else if (node.is_string()) {
                auto value = node.get<std::string>();
                if (value.starts_with("$")) {
                    auto vars = style["$variables"];
                    auto varName = value.substr(1);
                    if (vars.is_object() && vars.contains(varName)) {
                        node = vars[varName];
                    }
                }
            } else if (node.is_array()) {
                for (auto& item : node) {
                    self(self, item);
                }
            }
        };

        // Allow styles to inherit from a base style using an "inherits" property
        auto fnVisitInheritance = [&](auto&& self, Json& node) -> void {
            if (node.is_object() && node.contains("inherits")) {
                std::string baseStyleName = node["inherits"].get<std::string>();
                if (style.contains(baseStyleName)) {
                    Json baseStyle = style[baseStyleName];
                    node.update(baseStyle);
                }
            } else if (node.is_array()) {
                for (auto& item : node) {
                    self(self, item);
                }
            }
        };

        // recursively resolve references in the style (e.g., "$textPrimary" -> actual color value)
        // from the $variables section of the style
        fnVisitVariables(fnVisitVariables, style);

        // recursively resolve inheritance specified by "inherits" properties in the style
        fnVisitInheritance(fnVisitInheritance, style);
    }

    void Application::InitWindow(Window* win) {
        win->m_application = this;
        WindowHandle parentHandle = win->m_config.parent ? win->m_config.parent->m_handle : nullptr;
        win->m_handle = m_backend->CreateWindow(win->m_config, parentHandle);
    }

} // namespace gui
