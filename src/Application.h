#pragma once

#include "Backend.h"
#include "EventSystem.h"
#include "Graphics.h"

#include <concepts>
#include <memory>
#include <string>
#include <vector>

namespace gui {
    class Window;
    struct WindowConfig;

    template <typename W>
    concept DerivedFromWindow = std::derived_from<W, Window>;

    class Application : private BackendEventSink {
    public:
        Application() = default;
        ~Application() = default;

        Application(Backend* backend);

        int Start();

        Window* CreateWindow(const WindowConfig& config);

        template <DerivedFromWindow W>
        W* CreateWindow() {
            m_windows.push_back(std::make_unique<W>());
            InitWindow(m_windows.back().get());
            return dynamic_cast<W*>(m_windows.back().get());
        }

        void SetClipboard(const std::string& str);
        std::string GetClipboard();

        Backend& GetBackend() { return *m_backend; }
        EventSystem& GetEventSystem() { return m_eventSystem; }
        Json& GetStyle() { return m_style; }

        void LoadTheme(const std::string& themePath);
        void LoadThemeFromString(const std::string& themeJson);

    private:
        std::unique_ptr<Backend> m_backend;
        EventSystem m_eventSystem;
        std::vector<std::unique_ptr<Window>> m_windows;
        bool m_running{false};
        Json m_style;

        Window* FindWindow(WindowId id);

        // BackendEventSink implementation
        void OnQuit() override;
        void OnWindowCloseRequested(WindowId id) override;
        void OnWindowExposed(WindowId id) override;
        void OnWindowResized(WindowId id) override;
        void OnMouseButtonDown(WindowId id, int x, int y, MouseButton button) override;
        void OnMouseButtonUp(WindowId id, int x, int y, MouseButton button) override;
        void OnMouseMotion(WindowId id, int x, int y, MouseButton button) override;
        void OnMouseWheel(WindowId id, float dx, float dy, int mouseX, int mouseY) override;
        void OnKeyDown(WindowId id, Key key, ModifierState mod) override;
        void OnKeyUp(WindowId id, Key key, ModifierState mod) override;
        void OnTextInput(WindowId id, char c) override;

        void ProcessStyle(Json& style);

        void InitWindow(Window* win);
    };
} // namespace gui
