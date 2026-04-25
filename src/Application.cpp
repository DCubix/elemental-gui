#include "Application.h"

#include "Window.h"

#include <unordered_map>

namespace gui {

    static const std::string DefaultStyleJson =
        #include "generated/DefaultStyle.h"
            ;
    Json Application::DefaultStyle = Json::parse(DefaultStyleJson);

    void Application::SetBackend(std::unique_ptr<Backend> backend) {
        m_backend = std::move(backend);
    }

    Backend& Application::GetBackend() {
        return *m_backend;
    }

    int Application::Start(ApplicationAdapter* adapter) {
        if (!adapter || !m_backend) {
            return 1;
        }
        if (!m_backend->Init()) {
            return 1;
        }

        adapter->OnCreate(*this);

        m_running = true;
        while (m_running) {
            m_backend->PollEvents(*this);

            m_windows.erase(
                std::remove_if(m_windows.begin(), m_windows.end(),
                    [](const auto& win) { return win->m_closeRequested; }),
                m_windows.end()
            );

            for (auto& win : m_windows) {
                win->Update();
            }
        }

        adapter->OnDestroy();
        delete adapter;

        m_windows.clear();
        m_backend->Shutdown();

        return 0;
    }

    Window* Application::CreateWindow(const WindowConfig& config) {
        auto window = std::make_unique<Window>(config);
        m_windows.push_back(std::move(window));
        auto* win = m_windows.back().get();
        win->m_application = this;
        WindowHandle parentHandle = config.parent ? config.parent->m_handle : nullptr;
        win->m_handle = m_backend->CreateWindow(config, parentHandle);
        return win;
    }

    void Application::SetClipboard(const std::string& str) {
        m_backend->SetClipboardText(str);
    }

    std::string Application::GetClipboard() {
        return m_backend->GetClipboardText();
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

}
