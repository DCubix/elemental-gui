#include "backends/sdl3/SDL3Backend.h"

#include <unordered_map>

#include <SDL3/SDL.h>

namespace gui {
    SDL3Backend::~SDL3Backend() = default;

    static const std::unordered_map<SDL_Keycode, Key> SDLtoKey = {
        // Letters
        {SDLK_A, Key::A},
        {SDLK_B, Key::B},
        {SDLK_C, Key::C},
        {SDLK_D, Key::D},
        {SDLK_E, Key::E},
        {SDLK_F, Key::F},
        {SDLK_G, Key::G},
        {SDLK_H, Key::H},
        {SDLK_I, Key::I},
        {SDLK_J, Key::J},
        {SDLK_K, Key::K},
        {SDLK_L, Key::L},
        {SDLK_M, Key::M},
        {SDLK_N, Key::N},
        {SDLK_O, Key::O},
        {SDLK_P, Key::P},
        {SDLK_Q, Key::Q},
        {SDLK_R, Key::R},
        {SDLK_S, Key::S},
        {SDLK_T, Key::T},
        {SDLK_U, Key::U},
        {SDLK_V, Key::V},
        {SDLK_W, Key::W},
        {SDLK_X, Key::X},
        {SDLK_Y, Key::Y},
        {SDLK_Z, Key::Z},
        // Numbers
        {SDLK_0, Key::Num0},
        {SDLK_1, Key::Num1},
        {SDLK_2, Key::Num2},
        {SDLK_3, Key::Num3},
        {SDLK_4, Key::Num4},
        {SDLK_5, Key::Num5},
        {SDLK_6, Key::Num6},
        {SDLK_7, Key::Num7},
        {SDLK_8, Key::Num8},
        {SDLK_9, Key::Num9},
        // Numpad
        {SDLK_KP_0, Key::Numpad0},
        {SDLK_KP_1, Key::Numpad1},
        {SDLK_KP_2, Key::Numpad2},
        {SDLK_KP_3, Key::Numpad3},
        {SDLK_KP_4, Key::Numpad4},
        {SDLK_KP_5, Key::Numpad5},
        {SDLK_KP_6, Key::Numpad6},
        {SDLK_KP_7, Key::Numpad7},
        {SDLK_KP_8, Key::Numpad8},
        {SDLK_KP_9, Key::Numpad9},
        {SDLK_KP_MULTIPLY, Key::NumpadMultiply},
        {SDLK_KP_DIVIDE, Key::NumpadDivide},
        {SDLK_KP_PLUS, Key::NumpadAdd},
        {SDLK_KP_MINUS, Key::NumpadSubtract},
        {SDLK_KP_DECIMAL, Key::NumpadDecimal},
        {SDLK_KP_ENTER, Key::NumpadEnter},
        // Function keys
        {SDLK_F1, Key::F1},
        {SDLK_F2, Key::F2},
        {SDLK_F3, Key::F3},
        {SDLK_F4, Key::F4},
        {SDLK_F5, Key::F5},
        {SDLK_F6, Key::F6},
        {SDLK_F7, Key::F7},
        {SDLK_F8, Key::F8},
        {SDLK_F9, Key::F9},
        {SDLK_F10, Key::F10},
        {SDLK_F11, Key::F11},
        {SDLK_F12, Key::F12},
        {SDLK_F13, Key::F13},
        {SDLK_F14, Key::F14},
        {SDLK_F15, Key::F15},
        {SDLK_F16, Key::F16},
        {SDLK_F17, Key::F17},
        {SDLK_F18, Key::F18},
        {SDLK_F19, Key::F19},
        {SDLK_F20, Key::F20},
        {SDLK_F21, Key::F21},
        {SDLK_F22, Key::F22},
        {SDLK_F23, Key::F23},
        {SDLK_F24, Key::F24},
        // Arrow keys
        {SDLK_UP, Key::Up},
        {SDLK_DOWN, Key::Down},
        {SDLK_LEFT, Key::Left},
        {SDLK_RIGHT, Key::Right},
        // Modifiers
        {SDLK_LSHIFT, Key::LeftShift},
        {SDLK_RSHIFT, Key::RightShift},
        {SDLK_LCTRL, Key::LeftControl},
        {SDLK_RCTRL, Key::RightControl},
        {SDLK_LALT, Key::LeftAlt},
        {SDLK_RALT, Key::RightAlt},
        {SDLK_LGUI, Key::LeftMeta},
        {SDLK_RGUI, Key::RightMeta},
        // Navigation and editing
        {SDLK_SPACE, Key::Space},
        {SDLK_TAB, Key::Tab},
        {SDLK_RETURN, Key::Enter},
        {SDLK_BACKSPACE, Key::Backspace},
        {SDLK_DELETE, Key::Delete},
        {SDLK_INSERT, Key::Insert},
        {SDLK_HOME, Key::Home},
        {SDLK_END, Key::End},
        {SDLK_PAGEUP, Key::PageUp},
        {SDLK_PAGEDOWN, Key::PageDown},
        {SDLK_PRINTSCREEN, Key::PrintScreen},
        {SDLK_SCROLLLOCK, Key::ScrollLock},
        {SDLK_PAUSE, Key::Pause},
        {SDLK_CAPSLOCK, Key::CapsLock},
        {SDLK_NUMLOCKCLEAR, Key::NumLock},
        {SDLK_ESCAPE, Key::Escape},
        // Punctuation and symbols
        {SDLK_SEMICOLON, Key::Semicolon},
        {SDLK_EQUALS, Key::Equals},
        {SDLK_COMMA, Key::Comma},
        {SDLK_MINUS, Key::Minus},
        {SDLK_PERIOD, Key::Period},
        {SDLK_SLASH, Key::Slash},
        {SDLK_BACKSLASH, Key::Backslash},
        {SDLK_GRAVE, Key::Grave},
        {SDLK_LEFTBRACKET, Key::LeftBracket},
        {SDLK_RIGHTBRACKET, Key::RightBracket},
        {SDLK_APOSTROPHE, Key::Apostrophe},
        // Media keys
        {SDLK_MUTE, Key::Mute},
        {SDLK_VOLUMEUP, Key::VolumeUp},
        {SDLK_VOLUMEDOWN, Key::VolumeDown},
        {SDLK_MEDIA_PLAY, Key::PlayPause},
        {SDLK_MEDIA_STOP, Key::Stop},
        {SDLK_MEDIA_PREVIOUS_TRACK, Key::PreviousTrack},
        {SDLK_MEDIA_NEXT_TRACK, Key::NextTrack},
        // Application keys
        {SDLK_HELP, Key::Help},
        {SDLK_MENU, Key::Menu},
        {SDLK_SELECT, Key::Select},
        {SDLK_EXECUTE, Key::Execute},
        {SDLK_UNDO, Key::Undo},
        {SDLK_CUT, Key::Cut},
        {SDLK_COPY, Key::Copy},
        {SDLK_PASTE, Key::Paste},
        {SDLK_FIND, Key::Find},
        {SDLK_AGAIN, Key::Again},
        // Browser keys
        {SDLK_AC_BACK, Key::BrowserBack},
        {SDLK_AC_FORWARD, Key::BrowserForward},
        {SDLK_AC_REFRESH, Key::BrowserRefresh},
        {SDLK_AC_STOP, Key::BrowserStop},
        {SDLK_AC_SEARCH, Key::BrowserSearch},
        {SDLK_AC_BOOKMARKS, Key::BrowserFavorites},
        {SDLK_AC_HOME, Key::BrowserHome},
    };

    static inline Key SDLKeyToKey(SDL_Keycode key) {
        auto it = SDLtoKey.find(key);
        return it != SDLtoKey.end() ? it->second : Key::None;
    }

    static inline MouseButton SDLButtonToMouseButton(Uint8 button) {
        switch (button) {
            case SDL_BUTTON_LEFT:
                return MouseButton::Left;
            case SDL_BUTTON_RIGHT:
                return MouseButton::Right;
            case SDL_BUTTON_MIDDLE:
                return MouseButton::Middle;
            case SDL_BUTTON_X1:
                return MouseButton::X1;
            case SDL_BUTTON_X2:
                return MouseButton::X2;
            default:
                return MouseButton::Unknown;
        }
    }

    static inline ModifierState SDLModToModifierState(Uint16 mod) {
        return {
            .shift = (mod & SDL_KMOD_SHIFT) != 0,
            .control = (mod & SDL_KMOD_CTRL) != 0,
            .alt = (mod & SDL_KMOD_ALT) != 0,
            .meta = (mod & SDL_KMOD_GUI) != 0
        };
    }

    SDL3WindowData& SDL3Backend::GetData(WindowHandle handle) {
        return *m_windows.at(static_cast<SDL_Window*>(handle));
    }

    const SDL3WindowData& SDL3Backend::GetData(WindowHandle handle) const {
        return *m_windows.at(static_cast<SDL_Window*>(handle));
    }

    bool SDL3Backend::Init() {
        return SDL_Init(SDL_INIT_VIDEO);
    }

    void SDL3Backend::Shutdown() {
        SDL_Quit();
    }

    void SDL3Backend::PollEvents(BackendEventSink& sink) {
        SDL_Event evt{};
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
                case SDL_EVENT_QUIT:
                    sink.OnQuit();
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    sink.OnWindowCloseRequested(evt.window.windowID);
                    break;
                case SDL_EVENT_WINDOW_EXPOSED:
                    sink.OnWindowExposed(evt.window.windowID);
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_MAXIMIZED:
                case SDL_EVENT_WINDOW_RESTORED:
                    sink.OnWindowResized(evt.window.windowID);
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    sink.OnMouseButtonDown(
                        evt.button.windowID,
                        (int)evt.button.x,
                        (int)evt.button.y,
                        SDLButtonToMouseButton(evt.button.button)
                    );
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    sink.OnMouseButtonUp(
                        evt.button.windowID,
                        (int)evt.button.x,
                        (int)evt.button.y,
                        SDLButtonToMouseButton(evt.button.button)
                    );
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    sink.OnMouseMotion(
                        evt.motion.windowID,
                        (int)evt.motion.x,
                        (int)evt.motion.y,
                        SDLButtonToMouseButton(evt.button.button)
                    );
                    break;
                case SDL_EVENT_TEXT_INPUT:
                    sink.OnTextInput(evt.text.windowID, evt.text.text[0]);
                    break;
                case SDL_EVENT_KEY_DOWN:
                    sink.OnKeyDown(
                        evt.key.windowID,
                        SDLKeyToKey(evt.key.key),
                        SDLModToModifierState(evt.key.mod)
                    );
                    break;
                case SDL_EVENT_KEY_UP:
                    sink.OnKeyUp(
                        evt.key.windowID,
                        SDLKeyToKey(evt.key.key),
                        SDLModToModifierState(evt.key.mod)
                    );
                    break;
                case SDL_EVENT_MOUSE_WHEEL:
                    sink.OnMouseWheel(
                        evt.wheel.windowID,
                        evt.wheel.x,
                        evt.wheel.y,
                        (int)evt.wheel.mouse_x,
                        (int)evt.wheel.mouse_y
                    );
                    break;
            }
        }
    }

    WindowHandle SDL3Backend::CreateWindow(const WindowConfig& config, WindowHandle parentHandle) {
        SDL_WindowFlags flags = SDL_WINDOW_HIDDEN;
        switch (config.style) {
            case WindowStyle::TopLevel:
                break;
            case WindowStyle::Modal:
                flags |= SDL_WINDOW_MODAL;
                break;
            case WindowStyle::Utility:
                flags |= SDL_WINDOW_UTILITY;
                break;
            case WindowStyle::Borderless:
                flags |= SDL_WINDOW_BORDERLESS;
                break;
            case WindowStyle::Fullscreen:
                flags |= SDL_WINDOW_FULLSCREEN;
                break;
        }

        if (config.resizable) {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        SDL_Window* sdlWindow =
            SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);

        if (parentHandle) {
            SDL_SetWindowParent(sdlWindow, static_cast<SDL_Window*>(parentHandle));
        }
        if (config.style == WindowStyle::Modal) {
            SDL_SetWindowModal(sdlWindow, true);
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(sdlWindow, nullptr);

        std::unique_ptr<SDL3WindowData> data = std::make_unique<SDL3WindowData>();
        data->window = sdlWindow;
        data->renderer = renderer;
        data->buffer = nullptr;
        m_windows[sdlWindow] = std::move(data);

        CreateRenderBuffer(sdlWindow, config.width, config.height);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        return static_cast<WindowHandle>(sdlWindow);
    }

    void SDL3Backend::DestroyWindow(WindowHandle handle) {
        auto* sdlWindow = static_cast<SDL_Window*>(handle);
        auto it = m_windows.find(sdlWindow);
        if (it == m_windows.end())
            return;
        if (it->second->renderer)
            SDL_DestroyRenderer(it->second->renderer);
        if (it->second->window)
            SDL_DestroyWindow(it->second->window);
        m_windows.erase(it);
    }

    void SDL3Backend::ShowWindow(WindowHandle handle) {
        SDL_ShowWindow(static_cast<SDL_Window*>(handle));
    }

    void SDL3Backend::HideWindow(WindowHandle handle) {
        SDL_HideWindow(static_cast<SDL_Window*>(handle));
    }

    void SDL3Backend::RaiseWindow(WindowHandle handle) {
        SDL_RaiseWindow(static_cast<SDL_Window*>(handle));
    }

    void SDL3Backend::ResizeWindow(WindowHandle handle, uint32_t width, uint32_t height) {
        SDL_SetWindowSize(static_cast<SDL_Window*>(handle), width, height);
    }

    void SDL3Backend::SetWindowTitle(WindowHandle handle, const std::string& title) {
        SDL_SetWindowTitle(static_cast<SDL_Window*>(handle), title.c_str());
    }

    void SDL3Backend::GetWindowSize(WindowHandle handle, int& width, int& height) const {
        SDL_GetWindowSize(static_cast<SDL_Window*>(handle), &width, &height);
    }

    void SDL3Backend::GetWindowPosition(WindowHandle handle, int& x, int& y) const {
        SDL_GetWindowPosition(static_cast<SDL_Window*>(handle), &x, &y);
    }

    void SDL3Backend::SetWindowPosition(WindowHandle handle, int x, int y) {
        SDL_SetWindowPosition(static_cast<SDL_Window*>(handle), x, y);
    }

    void SDL3Backend::SetWindowResizable(WindowHandle handle, bool resizable) {
        SDL_SetWindowResizable(static_cast<SDL_Window*>(handle), resizable);
    }

    void SDL3Backend::SetWindowStyle(WindowHandle handle, WindowStyle style) {
        // No-op since style is only used at window creation time in SDL.
    }

    void SDL3Backend::SetWindowParent(WindowHandle handle, WindowHandle parentHandle) {
        SDL_SetWindowParent(
            static_cast<SDL_Window*>(handle),
            static_cast<SDL_Window*>(parentHandle)
        );
    }

    WindowId SDL3Backend::GetWindowId(WindowHandle handle) const {
        return SDL_GetWindowID(static_cast<SDL_Window*>(handle));
    }

    void SDL3Backend::StartTextInput(WindowHandle handle) {
        SDL_StartTextInput(static_cast<SDL_Window*>(handle));
    }

    void SDL3Backend::StopTextInput(WindowHandle handle) {
        SDL_StopTextInput(static_cast<SDL_Window*>(handle));
    }

    void SDL3Backend::CreateRenderBuffer(WindowHandle handle, uint32_t width, uint32_t height) {
        auto& data = GetData(handle);
        if (data.buffer) {
            SDL_DestroyTexture(data.buffer);
        }
        data.buffer = SDL_CreateTexture(
            data.renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height
        );
    }

    void SDL3Backend::DestroyRenderBuffer(WindowHandle handle) {
        auto& data = GetData(handle);
        if (data.buffer) {
            SDL_DestroyTexture(data.buffer);
            data.buffer = nullptr;
        }
    }

    void SDL3Backend::PresentFrame(
        WindowHandle handle,
        unsigned char* data,
        int stride,
        uint32_t width,
        uint32_t height
    ) {
        auto& wd = GetData(handle);
        SDL_UpdateTexture(wd.buffer, nullptr, data, stride);
        SDL_RenderTexture(wd.renderer, wd.buffer, nullptr, nullptr);
        SDL_RenderPresent(wd.renderer);
    }

    void SDL3Backend::SetClipboardText(const std::string& text) {
        SDL_SetClipboardText(text.c_str());
    }

    std::string SDL3Backend::GetClipboardText() const {
        return std::string(SDL_GetClipboardText());
    }

} // namespace gui
