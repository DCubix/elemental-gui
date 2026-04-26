#include <Application.h>
#include <Backend.h>
#include <Declarative.h>

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"
#undef None

#include <iostream>
#include <memory>
#include <vector>

using namespace gui;
using namespace gui::declarative;

using namespace gui;
using namespace gui::declarative;

static const std::unordered_map<uint32_t, Key> KeyMap = {
    {SMOLK_A, Key::A},
    {SMOLK_B, Key::B},
    {SMOLK_C, Key::C},
    {SMOLK_D, Key::D},
    {SMOLK_E, Key::E},
    {SMOLK_F, Key::F},
    {SMOLK_G, Key::G},
    {SMOLK_H, Key::H},
    {SMOLK_I, Key::I},
    {SMOLK_J, Key::J},
    {SMOLK_K, Key::K},
    {SMOLK_L, Key::L},
    {SMOLK_M, Key::M},
    {SMOLK_N, Key::N},
    {SMOLK_O, Key::O},
    {SMOLK_P, Key::P},
    {SMOLK_Q, Key::Q},
    {SMOLK_R, Key::R},
    {SMOLK_S, Key::S},
    {SMOLK_T, Key::T},
    {SMOLK_U, Key::U},
    {SMOLK_V, Key::V},
    {SMOLK_W, Key::W},
    {SMOLK_X, Key::X},
    {SMOLK_Y, Key::Y},
    {SMOLK_Z, Key::Z},

    {SMOLK_NUM0, Key::Num0},
    {SMOLK_NUM1, Key::Num1},
    {SMOLK_NUM2, Key::Num2},
    {SMOLK_NUM3, Key::Num3},
    {SMOLK_NUM4, Key::Num4},
    {SMOLK_NUM5, Key::Num5},
    {SMOLK_NUM6, Key::Num6},
    {SMOLK_NUM7, Key::Num7},
    {SMOLK_NUM8, Key::Num8},
    {SMOLK_NUM9, Key::Num9},

    {SMOLK_NUMPAD0, Key::Numpad0},
    {SMOLK_NUMPAD1, Key::Numpad1},
    {SMOLK_NUMPAD2, Key::Numpad2},
    {SMOLK_NUMPAD3, Key::Numpad3},
    {SMOLK_NUMPAD4, Key::Numpad4},
    {SMOLK_NUMPAD5, Key::Numpad5},
    {SMOLK_NUMPAD6, Key::Numpad6},
    {SMOLK_NUMPAD7, Key::Numpad7},
    {SMOLK_NUMPAD8, Key::Numpad8},
    {SMOLK_NUMPAD9, Key::Numpad9},

    {SMOLK_F1, Key::F1},
    {SMOLK_F2, Key::F2},
    {SMOLK_F3, Key::F3},
    {SMOLK_F4, Key::F4},
    {SMOLK_F5, Key::F5},
    {SMOLK_F6, Key::F6},
    {SMOLK_F7, Key::F7},
    {SMOLK_F8, Key::F8},
    {SMOLK_F9, Key::F9},
    {SMOLK_F10, Key::F10},
    {SMOLK_F11, Key::F11},
    {SMOLK_F12, Key::F12},
    {SMOLK_F13, Key::F13},
    {SMOLK_F14, Key::F14},
    {SMOLK_F15, Key::F15},

    {SMOLK_UP, Key::Up},
    {SMOLK_DOWN, Key::Down},
    {SMOLK_LEFT, Key::Left},
    {SMOLK_RIGHT, Key::Right},
    {SMOLK_LSHIFT, Key::LeftShift},
    {SMOLK_RSHIFT, Key::RightShift},
    {SMOLK_LCONTROL, Key::LeftControl},
    {SMOLK_RCONTROL, Key::RightControl},
    {SMOLK_LALT, Key::LeftAlt},
    {SMOLK_RALT, Key::RightAlt},
    {SMOLK_LSYSTEM, Key::LeftMeta},
    {SMOLK_RSYSTEM, Key::RightMeta},

    {SMOLK_ESC, Key::Escape},
    {SMOLK_MENU, Key::Menu},
    {SMOLK_LBRACKET, Key::LeftBracket},
    {SMOLK_RBRACKET, Key::RightBracket},
    {SMOLK_SEMICOLON, Key::Semicolon},
    {SMOLK_COMMA, Key::Comma},
    {SMOLK_PERIOD, Key::Period},
    {SMOLK_QUOTE, Key::Apostrophe},
    {SMOLK_SLASH, Key::Slash},
    {SMOLK_BACKSLASH, Key::Backslash},
    {SMOLK_TILDE, Key::Grave},
    {SMOLK_EQUAL, Key::Equals},
    {SMOLK_DASH, Key::Minus},
    {SMOLK_SPACE, Key::Space},
    {SMOLK_RETURN, Key::Enter},
    {SMOLK_BACKSPACE, Key::Backspace},
    {SMOLK_TAB, Key::Tab},
    {SMOLK_PAGEUP, Key::PageUp},
    {SMOLK_PAGEDOWN, Key::PageDown},
    {SMOLK_END, Key::End},
    {SMOLK_HOME, Key::Home},
    {SMOLK_INSERT, Key::Insert},
    {SMOLK_DELETE, Key::Delete},
    {SMOLK_ADD, Key::NumpadAdd},
    {SMOLK_SUBSTRACT, Key::NumpadSubtract},
    {SMOLK_MULTIPLY, Key::NumpadMultiply},
    {SMOLK_DIVIDE, Key::NumpadDivide},
    {SMOLK_ENTER, Key::NumpadEnter},
    {SMOLK_CAPSLOCK, Key::CapsLock},
    {SMOLK_NUMLOCK, Key::NumLock},
    {SMOLK_PRINTSCREEN, Key::PrintScreen},
    {SMOLK_SCROLLLOCK, Key::ScrollLock},
    {SMOLK_PAUSE, Key::Pause},
    {SMOLK_PLUS, Key::OemPlus},
    {SMOLK_MINUS, Key::OemMinus},
};

static inline Key SMOLKToKey(uint32_t key) {
    auto it = KeyMap.find(key);
    return it != KeyMap.end() ? it->second : Key::None;
}

static inline MouseButton SMOLButtonToMouseButton(int button) {
    switch (button) {
        case 1:
            return MouseButton::Left;
        case 2:
            return MouseButton::Right;
        case 3:
            return MouseButton::Middle;
        case 4:
            return MouseButton::X1;
        case 5:
            return MouseButton::X2;
        default:
            return MouseButton::Unknown;
    }
}

class SmolBackend : public Backend {
  public:
    bool Init() override { return true; }
    void Shutdown() override {}

    void PollEvents(BackendEventSink& sink) {
        if (m_frames.empty()) {
            sink.OnQuit();
            return;
        }

        for (auto& frame : m_frames) {
            WindowId windowId = GetWindowId((WindowHandle)frame);

            smol_frame_update(frame);

            SMOL_FRAME_EVENT_LOOP(frame, ev) {
                switch (ev.type) {
                    case SMOL_FRAME_EVENT_RESIZE:
                        sink.OnWindowResized(windowId);
                        break;
                    case SMOL_FRAME_EVENT_KEY_DOWN:
                        // TODO: Missing modifier state, smol_frame doesn't provide it.
                        sink.OnKeyDown(windowId, SMOLKToKey(ev.key.code), {});
                        break;
                    case SMOL_FRAME_EVENT_KEY_UP:
                        // TODO: Missing modifier state, smol_frame doesn't provide it.
                        sink.OnKeyUp(windowId, SMOLKToKey(ev.key.code), {});
                        break;
                    case SMOL_FRAME_EVENT_MOUSE_MOVE:
                        sink.OnMouseMotion(windowId, ev.mouse.x, ev.mouse.y, MouseButton::Unknown);
                        break;
                    case SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN:
                        sink.OnMouseButtonDown(
                            windowId,
                            ev.mouse.x,
                            ev.mouse.y,
                            SMOLButtonToMouseButton(ev.mouse.button)
                        );
                        break;
                    case SMOL_FRAME_EVENT_MOUSE_BUTTON_UP:
                        sink.OnMouseButtonUp(
                            windowId,
                            ev.mouse.x,
                            ev.mouse.y,
                            SMOLButtonToMouseButton(ev.mouse.button)
                        );
                        break;
                    case SMOL_FRAME_EVENT_MOUSE_VER_WHEEL:
                        sink.OnMouseWheel(
                            windowId,
                            0.0f,
                            (float)ev.mouse.z,
                            ev.mouse.x,
                            ev.mouse.y
                        );
                        break;
                    case SMOL_FRAME_EVENT_MOUSE_HOR_WHEEL:
                        sink.OnMouseWheel(
                            windowId,
                            (float)ev.mouse.w,
                            0.0f,
                            ev.mouse.x,
                            ev.mouse.y
                        );
                        break;
                    case SMOL_FRAME_EVENT_TEXT_INPUT:
                        sink.OnTextInput(windowId, (char)ev.input.codepoint);
                        break;
                    case SMOL_FRAME_EVENT_FOCUS_LOST:
                    case SMOL_FRAME_EVENT_FOCUS_GAINED:
                        // TODO: Handle focus events if needed
                        break;
                }
            }

            if (smol_frame_is_closed(frame)) {
                sink.OnWindowCloseRequested(windowId);
                break;
            }
        }
    }

    WindowHandle CreateWindow(const WindowConfig& config, WindowHandle parentHandle) {
        uint32_t flags = SMOL_FRAME_DEFAULT_CONFIG;
        switch (config.style) {
            case WindowStyle::Fullscreen:
                break;
            case WindowStyle::TopLevel:
                break;
            case WindowStyle::Modal:
            case WindowStyle::Utility:
                flags |= SMOL_FRAME_CONFIG_ALWAYS_ON_TOP;
                break;
            case WindowStyle::Borderless:
                flags |= SMOL_FRAME_CONFIG_BORDERLESS;
                break;
        }

        if (config.resizable) {
            flags |= SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON;
        }

        smol_frame_config_t conf{};
        conf.width = config.width;
        conf.height = config.height;
        conf.title = config.title.c_str();
        conf.flags = flags;
        conf.parent = (smol_frame_t*)parentHandle;
        conf.gl_spec = nullptr;
        conf.allocator = nullptr;

        smol_frame_t* frame = smol_frame_create_advanced(&conf);
        m_frames.push_back(frame);
        return (WindowHandle)frame;
    }

    void DestroyWindow(WindowHandle handle) {
        // Remove frame from vector
        m_frames.erase(
            std::remove(m_frames.begin(), m_frames.end(), (smol_frame_t*)handle),
            m_frames.end()
        );
        smol_frame_destroy((smol_frame_t*)handle);
    }

    void ShowWindow(WindowHandle handle) {
        // No-op, smol_frame shows itself when created.
    }

    void HideWindow(WindowHandle handle) {
        // No-op, smol_frame doesn't support hiding.
    }

    void RaiseWindow(WindowHandle handle) {
        // No-op, smol_frame doesn't support raising.
    }

    void ResizeWindow(WindowHandle handle, uint32_t width, uint32_t height) {
        // No-op, smol_frame doesn't support resizing.
    }

    void SetWindowTitle(WindowHandle handle, const std::string& title) {
        smol_frame_set_title((smol_frame_t*)handle, title.c_str());
    }

    void GetWindowSize(WindowHandle handle, int& width, int& height) const {
        width = smol_frame_width((smol_frame_t*)handle);
        height = smol_frame_height((smol_frame_t*)handle);
    }

    void GetWindowPosition(WindowHandle handle, int& x, int& y) const {
        // No-op, smol_frame doesn't support querying position.
        x = 0;
        y = 0;
    }

    void SetWindowPosition(WindowHandle handle, int x, int y) {
        // No-op, smol_frame doesn't support setting position.
    }

    void SetWindowResizable(WindowHandle handle, bool resizable) {
        // No-op, smol_frame doesn't support changing resizable after creation.
    }

    void SetWindowStyle(WindowHandle handle, WindowStyle style) {
        // No-op, smol_frame doesn't support changing style after creation.
    }

    void SetWindowParent(WindowHandle handle, WindowHandle parentHandle) {
        // No-op, smol_frame doesn't support changing parent after creation.
    }

    WindowId GetWindowId(WindowHandle handle) const { return (WindowId)(uintptr_t)handle; }

    void StartTextInput(WindowHandle handle) {
        // No-op, smol_frame doesn't support IME.
    }

    void StopTextInput(WindowHandle handle) {
        // No-op, smol_frame doesn't support IME.
    }

    void CreateRenderBuffer(WindowHandle handle, uint32_t width, uint32_t height) {
        // No-op, smol_frame doesn't need a separate render buffer.
    }

    void DestroyRenderBuffer(WindowHandle handle) {
        // No-op, smol_frame doesn't need a separate render buffer.
    }

    void PresentFrame(
        WindowHandle handle,
        unsigned char* data,
        int stride,
        uint32_t width,
        uint32_t height
    ) {
        uint32_t* pixels = reinterpret_cast<uint32_t*>(data);

        // Convert ARGB to RGBA (LSB order)
        for (size_t i = 0; i < width * height; ++i) {
            uint32_t pixel = pixels[i];
            uint8_t a = (pixel >> 24) & 0xFF;
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;
            pixels[i] = (a << 24) | (b << 16) | (g << 8) | r;
        }

        smol_frame_blit_pixels(
            (smol_frame_t*)handle,
            pixels,
            width,
            height,
            0,
            0,
            width,
            height,
            0,
            0,
            width,
            height
        );
    }

    // Clipboard
    void SetClipboardText(const std::string& text) {
        // No-op, smol_frame doesn't support clipboard.
    }

    std::string GetClipboardText() const {
        // No-op, smol_frame doesn't support clipboard.
        return "";
    }

  private:
    std::vector<smol_frame_t*> m_frames;
};

class App : public gui::Window {
  public:
    App()
        : gui::Window(WindowConfig{.title = "SmolBackend Example", .width = 800, .height = 600}) {}

    WidgetDesc OnBuild() override {
        Show();

        // clang-format off
        auto ui = Column({
            .gap = 16,
            .padding = EdgeInsets::All(32),
            .align = FlexAlign::Stretch,
            .justify = FlexJustify::Center
        }, {
            Text("Elemental GUI with SmolBackend", { .base = { .autoSize = true } }),
            Button("Click Me!", { 
                .onClick = [this]() { 
                    std::cout << "Button clicked!" << std::endl; 
                }
            }),
            Slider({ 
                .range = {0.0f, 100.0f}, 
                .value = 50.0f,
                .onValueChange = [this](float value) {
                    std::cout << "Slider value: " << value << std::endl;
                }
            })
        });
        // clang-format on
        return ui;
    }
};

#if 1
int main() {
    Application app{new SmolBackend()};
    app.CreateWindow<App>();
    return app.Start();
}
#else
int main() {
    smol_frame_gl_spec_t spec = smol_init_gl_spec(3, 3, SMOL_FALSE, SMOL_TRUE, 8, SMOL_TRUE);

    smol_frame_config_t config = {
        .width = 800,
        .height = 600,
        .title = "Cryptid Rampage",
        .flags = SMOL_FRAME_DEFAULT_CONFIG | SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON |
                 SMOL_FRAME_CONFIG_BORDERLESS,
        .gl_spec = &spec,
    };

    smol_frame_t* frame = smol_frame_create_advanced(&config);

    // #ifndef SMOL_PLATFORM_WEB
    // gladLoadGL((GLADloadfunc)&smol_gl_get_proc_address);
    // glDebugMessageCallback(&debug_message_callback, nullptr);
    // smol_frame_set_maximized(frame, SMOL_TRUE);
    // #endif

    while (!smol_frame_is_closed(frame)) {
        smol_frame_update(frame);

        SMOL_FRAME_EVENT_LOOP(frame, ev) {
            switch (ev.type) {
                default:
                    break;
            }
        }

        smol_frame_gl_swap_buffers(frame);
    }
    return 0;
}
#endif