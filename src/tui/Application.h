#pragma once

#include <string>
#include <memory>
#include <vector>

#include "EventSystem.h"
#include "Graphics.h"
#include "Backend.h"

namespace tui {
    class Application;
    class ApplicationAdapter {
    public:
        virtual ~ApplicationAdapter() = default;
        virtual void OnCreate(Application& app) = 0;
        virtual void OnDestroy() = 0;
    };

    class Window;
    struct WindowConfig;

    class Application : private BackendEventSink {
    public:
        Application() = default;
        ~Application() = default;

        void SetBackend(std::unique_ptr<Backend> backend);
        Backend& GetBackend();

        int Start(ApplicationAdapter* adapter);

        Window* CreateWindow(const WindowConfig& config);

        void SetClipboard(const std::string& str);
        std::string GetClipboard();

        EventSystem& GetEventSystem() { return m_eventSystem; }

        static Json DefaultStyle;
    private:
        std::unique_ptr<Backend> m_backend;
        EventSystem m_eventSystem;
        std::vector<std::unique_ptr<Window>> m_windows;
        bool m_running{ false };

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
    };
}
