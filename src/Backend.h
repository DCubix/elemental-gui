#pragma once

#include <string>
#include <cstdint>

#include "EventSystem.h"
#include "WindowConfig.h"

namespace gui {

    using WindowHandle = void*;
    using WindowId = uint64_t;

    // Application implements this; Backend calls it with already-translated events.
    // No platform types appear in this interface.
    class BackendEventSink {
    public:
        virtual ~BackendEventSink() = default;

        virtual void OnQuit() = 0;
        virtual void OnWindowCloseRequested(WindowId id) = 0;
        virtual void OnWindowExposed(WindowId id) = 0;
        virtual void OnWindowResized(WindowId id) = 0;

        virtual void OnMouseButtonDown(WindowId id, int x, int y, MouseButton button) = 0;
        virtual void OnMouseButtonUp(WindowId id, int x, int y, MouseButton button) = 0;
        virtual void OnMouseMotion(WindowId id, int x, int y, MouseButton button) = 0;
        virtual void OnMouseWheel(WindowId id, float dx, float dy, int mouseX, int mouseY) = 0;

        virtual void OnKeyDown(WindowId id, Key key, ModifierState mod) = 0;
        virtual void OnKeyUp(WindowId id, Key key, ModifierState mod) = 0;
        virtual void OnTextInput(WindowId id, char c) = 0;
    };

    class Backend {
    public:
        virtual ~Backend() = default;

        // Lifecycle
        virtual bool Init() = 0;
        virtual void Shutdown() = 0;

        // Pump platform events and call sink for each translated event.
        virtual void PollEvents(BackendEventSink& sink) = 0;

        // Window management
        // parentHandle is the handle of config.parent (null if no parent).
        virtual WindowHandle CreateWindow(const WindowConfig& config, WindowHandle parentHandle) = 0;
        virtual void DestroyWindow(WindowHandle handle) = 0;

        virtual void ShowWindow(WindowHandle handle) = 0;
        virtual void HideWindow(WindowHandle handle) = 0;
        virtual void RaiseWindow(WindowHandle handle) = 0;
        virtual void ResizeWindow(WindowHandle handle, uint32_t width, uint32_t height) = 0;
        virtual void SetWindowTitle(WindowHandle handle, const std::string& title) = 0;
        virtual void GetWindowSize(WindowHandle handle, int& width, int& height) const = 0;
        virtual void GetWindowPosition(WindowHandle handle, int& x, int& y) const = 0;
        virtual void SetWindowPosition(WindowHandle handle, int x, int y) = 0;
        virtual WindowId GetWindowId(WindowHandle handle) const = 0;

        // Text input (IME)
        virtual void StartTextInput(WindowHandle handle) = 0;
        virtual void StopTextInput(WindowHandle handle) = 0;

        // Render: upload Cairo ARGB32 pixels and present to screen.
        // data and stride come directly from cairo_image_surface_get_data/stride.
        virtual void CreateRenderBuffer(WindowHandle handle, uint32_t width, uint32_t height) = 0;
        virtual void DestroyRenderBuffer(WindowHandle handle) = 0;
        virtual void PresentFrame(WindowHandle handle, unsigned char* data, int stride,
                                   uint32_t width, uint32_t height) = 0;

        // Clipboard
        virtual void SetClipboardText(const std::string& text) = 0;
        virtual std::string GetClipboardText() const = 0;
    };

}
