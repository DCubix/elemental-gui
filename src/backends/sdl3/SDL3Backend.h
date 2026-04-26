#pragma once

#include "Backend.h"

#include <unordered_map>

#include <SDL3/SDL.h>

namespace gui {

    struct SDL3WindowData {
        SDL_Window* window{nullptr};
        SDL_Renderer* renderer{nullptr};
        SDL_Texture* buffer{nullptr};
    };

    class SDL3Backend : public Backend {
      public:
        bool Init() override;
        void Shutdown() override;

        void PollEvents(BackendEventSink& sink) override;

        WindowHandle CreateWindow(const WindowConfig& config, WindowHandle parentHandle) override;
        void DestroyWindow(WindowHandle handle) override;

        void ShowWindow(WindowHandle handle) override;
        void HideWindow(WindowHandle handle) override;
        void RaiseWindow(WindowHandle handle) override;
        void ResizeWindow(WindowHandle handle, uint32_t width, uint32_t height) override;
        void SetWindowTitle(WindowHandle handle, const std::string& title) override;
        void GetWindowSize(WindowHandle handle, int& width, int& height) const override;
        void GetWindowPosition(WindowHandle handle, int& x, int& y) const override;
        void SetWindowPosition(WindowHandle handle, int x, int y) override;
        void SetWindowResizable(WindowHandle handle, bool resizable) override;
        void SetWindowStyle(WindowHandle handle, WindowStyle style) override;
        void SetWindowParent(WindowHandle handle, WindowHandle parentHandle) override;
        WindowId GetWindowId(WindowHandle handle) const override;

        void StartTextInput(WindowHandle handle) override;
        void StopTextInput(WindowHandle handle) override;

        void CreateRenderBuffer(WindowHandle handle, uint32_t width, uint32_t height) override;
        void DestroyRenderBuffer(WindowHandle handle) override;
        void PresentFrame(
            WindowHandle handle,
            unsigned char* data,
            int stride,
            uint32_t width,
            uint32_t height
        ) override;

        void SetClipboardText(const std::string& text) override;
        std::string GetClipboardText() const override;

      private:
        std::unordered_map<SDL_Window*, SDL3WindowData> m_windows;

        SDL3WindowData& GetData(WindowHandle handle);
        const SDL3WindowData& GetData(WindowHandle handle) const;
    };

} // namespace gui
