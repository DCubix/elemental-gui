#include "Application.h"

#include "EventSystem.h"
#include "FlexLayout.h"

#include <SDL3/SDL_keycode.h>
#include <algorithm>
#include <unordered_map>

namespace tui {

	static const std::string DefaultStyleJson =
		#include "generated/DefaultStyle.h"
			;
	Json Application::DefaultStyle = Json::parse(DefaultStyleJson);

	static const std::unordered_map<SDL_Keycode, Key> SDLtoKey = {
		// Letters
		{ SDLK_A, Key::A }, { SDLK_B, Key::B }, { SDLK_C, Key::C }, { SDLK_D, Key::D },
		{ SDLK_E, Key::E }, { SDLK_F, Key::F }, { SDLK_G, Key::G }, { SDLK_H, Key::H },
		{ SDLK_I, Key::I }, { SDLK_J, Key::J }, { SDLK_K, Key::K }, { SDLK_L, Key::L },
		{ SDLK_M, Key::M }, { SDLK_N, Key::N }, { SDLK_O, Key::O }, { SDLK_P, Key::P },
		{ SDLK_Q, Key::Q }, { SDLK_R, Key::R }, { SDLK_S, Key::S }, { SDLK_T, Key::T },
		{ SDLK_U, Key::U }, { SDLK_V, Key::V }, { SDLK_W, Key::W }, { SDLK_X, Key::X },
		{ SDLK_Y, Key::Y }, { SDLK_Z, Key::Z },
		// Numbers
		{ SDLK_0, Key::Num0 }, { SDLK_1, Key::Num1 }, { SDLK_2, Key::Num2 },
		{ SDLK_3, Key::Num3 }, { SDLK_4, Key::Num4 }, { SDLK_5, Key::Num5 },
		{ SDLK_6, Key::Num6 }, { SDLK_7, Key::Num7 }, { SDLK_8, Key::Num8 },
		{ SDLK_9, Key::Num9 },
		// Numpad
		{ SDLK_KP_0, Key::Numpad0 }, { SDLK_KP_1, Key::Numpad1 }, { SDLK_KP_2, Key::Numpad2 },
		{ SDLK_KP_3, Key::Numpad3 }, { SDLK_KP_4, Key::Numpad4 }, { SDLK_KP_5, Key::Numpad5 },
		{ SDLK_KP_6, Key::Numpad6 }, { SDLK_KP_7, Key::Numpad7 }, { SDLK_KP_8, Key::Numpad8 },
		{ SDLK_KP_9, Key::Numpad9 },
		{ SDLK_KP_MULTIPLY, Key::NumpadMultiply }, { SDLK_KP_DIVIDE, Key::NumpadDivide },
		{ SDLK_KP_PLUS, Key::NumpadAdd }, { SDLK_KP_MINUS, Key::NumpadSubtract },
		{ SDLK_KP_DECIMAL, Key::NumpadDecimal }, { SDLK_KP_ENTER, Key::NumpadEnter },
		// Function keys
		{ SDLK_F1, Key::F1 }, { SDLK_F2, Key::F2 }, { SDLK_F3, Key::F3 }, { SDLK_F4, Key::F4 },
		{ SDLK_F5, Key::F5 }, { SDLK_F6, Key::F6 }, { SDLK_F7, Key::F7 }, { SDLK_F8, Key::F8 },
		{ SDLK_F9, Key::F9 }, { SDLK_F10, Key::F10 }, { SDLK_F11, Key::F11 }, { SDLK_F12, Key::F12 },
		{ SDLK_F13, Key::F13 }, { SDLK_F14, Key::F14 }, { SDLK_F15, Key::F15 }, { SDLK_F16, Key::F16 },
		{ SDLK_F17, Key::F17 }, { SDLK_F18, Key::F18 }, { SDLK_F19, Key::F19 }, { SDLK_F20, Key::F20 },
		{ SDLK_F21, Key::F21 }, { SDLK_F22, Key::F22 }, { SDLK_F23, Key::F23 }, { SDLK_F24, Key::F24 },
		// Arrow keys
		{ SDLK_UP, Key::Up }, { SDLK_DOWN, Key::Down }, { SDLK_LEFT, Key::Left }, { SDLK_RIGHT, Key::Right },
		// Modifiers
		{ SDLK_LSHIFT, Key::LeftShift }, { SDLK_RSHIFT, Key::RightShift },
		{ SDLK_LCTRL, Key::LeftControl }, { SDLK_RCTRL, Key::RightControl },
		{ SDLK_LALT, Key::LeftAlt }, { SDLK_RALT, Key::RightAlt },
		{ SDLK_LGUI, Key::LeftMeta }, { SDLK_RGUI, Key::RightMeta },
		// Navigation and editing
		{ SDLK_SPACE, Key::Space }, { SDLK_TAB, Key::Tab }, { SDLK_RETURN, Key::Enter },
		{ SDLK_BACKSPACE, Key::Backspace }, { SDLK_DELETE, Key::Delete }, { SDLK_INSERT, Key::Insert },
		{ SDLK_HOME, Key::Home }, { SDLK_END, Key::End }, { SDLK_PAGEUP, Key::PageUp },
		{ SDLK_PAGEDOWN, Key::PageDown },
		{ SDLK_PRINTSCREEN, Key::PrintScreen }, { SDLK_SCROLLLOCK, Key::ScrollLock },
		{ SDLK_PAUSE, Key::Pause }, { SDLK_CAPSLOCK, Key::CapsLock },
		{ SDLK_NUMLOCKCLEAR, Key::NumLock }, { SDLK_ESCAPE, Key::Escape },
		// Punctuation and symbols
		{ SDLK_SEMICOLON, Key::Semicolon }, { SDLK_EQUALS, Key::Equals },
		{ SDLK_COMMA, Key::Comma }, { SDLK_MINUS, Key::Minus },
		{ SDLK_PERIOD, Key::Period }, { SDLK_SLASH, Key::Slash },
		{ SDLK_BACKSLASH, Key::Backslash }, { SDLK_GRAVE, Key::Grave },
		{ SDLK_LEFTBRACKET, Key::LeftBracket }, { SDLK_RIGHTBRACKET, Key::RightBracket },
		{ SDLK_APOSTROPHE, Key::Apostrophe },
		// Media keys
		{ SDLK_MUTE, Key::Mute }, { SDLK_VOLUMEUP, Key::VolumeUp },
		{ SDLK_VOLUMEDOWN, Key::VolumeDown },
		{ SDLK_MEDIA_PLAY, Key::PlayPause }, { SDLK_MEDIA_STOP, Key::Stop },
		{ SDLK_MEDIA_PREVIOUS_TRACK, Key::PreviousTrack }, { SDLK_MEDIA_NEXT_TRACK, Key::NextTrack },
		// Application keys
		{ SDLK_HELP, Key::Help }, { SDLK_MENU, Key::Menu },
		{ SDLK_SELECT, Key::Select }, { SDLK_EXECUTE, Key::Execute },
		{ SDLK_UNDO, Key::Undo }, { SDLK_CUT, Key::Cut }, { SDLK_COPY, Key::Copy },
		{ SDLK_PASTE, Key::Paste }, { SDLK_FIND, Key::Find }, { SDLK_AGAIN, Key::Again },
		// SDLK_SAVE and SDLK_PRINT not available in SDL3,
		// Browser keys
		{ SDLK_AC_BACK, Key::BrowserBack }, { SDLK_AC_FORWARD, Key::BrowserForward },
		{ SDLK_AC_REFRESH, Key::BrowserRefresh }, { SDLK_AC_STOP, Key::BrowserStop },
		{ SDLK_AC_SEARCH, Key::BrowserSearch }, { SDLK_AC_BOOKMARKS, Key::BrowserFavorites },
		{ SDLK_AC_HOME, Key::BrowserHome },
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

	Application::Application(const std::string& title, uint width, uint height, bool resizable)
		: m_title(title), m_width(width), m_height(height), m_shouldRedraw(true),
		  m_focused(nullptr), m_resizable(resizable)
	{}

	int Application::Start(ApplicationAdapter* adapter) {
		if (adapter == nullptr) {
			SDL_Log("Invalid adapter.");
			return 7;
		}

		if (!SDL_Init(SDL_INIT_VIDEO)) {
			SDL_Log("%s", SDL_GetError());
			delete adapter;
			return 1;
		}

		SDL_WindowFlags flags = 0;
		if (m_resizable) {
			flags |= SDL_WINDOW_RESIZABLE;
		}
		m_window = SDL_CreateWindow(
			m_title.c_str(),
			m_width, m_height,
			flags
		);

		if (m_window == nullptr) {
			SDL_Log("%s", SDL_GetError());
			delete adapter;
			SDL_Quit();
			return 1;
		}

		m_renderer = SDL_CreateRenderer(m_window, NULL);
		if (m_renderer == nullptr) {
			SDL_Log("%s", SDL_GetError());
			delete adapter;
			SDL_DestroyWindow(m_window);
			SDL_Quit();
			return 1;
		}

		m_graphics = Graphics::CreateGraphics();
		
		ResizeScreenBuffer();

		adapter->OnCreate(*this);

		bool running = true;
		SDL_Event evt{};

		SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
		SDL_RenderClear(m_renderer);

		while (running) {
			while (SDL_PollEvent(&evt)) {
				switch (evt.type) {
					case SDL_EVENT_QUIT: running = false; break;
					case SDL_EVENT_WINDOW_RESIZED:
					case SDL_EVENT_WINDOW_MAXIMIZED:
					case SDL_EVENT_WINDOW_RESTORED: {
						int w, h;
						SDL_GetWindowSize(m_window, &w, &h);
						m_width = uint(w);
						m_height = uint(h);
						RequestRedrawAll();
						ResizeScreenBuffer();
					} break;
					case SDL_EVENT_MOUSE_BUTTON_DOWN: {
						DispatchEvent<MouseEvent>(
							(int)evt.button.x,
							(int)evt.button.y,
							SDLButtonToMouseButton(evt.button.button),
							true
						);
					} break;
					case SDL_EVENT_MOUSE_BUTTON_UP: {
						DispatchEvent<MouseEvent>(
							(int)evt.button.x,
							(int)evt.button.y,
							SDLButtonToMouseButton(evt.button.button),
							false
						);
					} break;
					case SDL_EVENT_MOUSE_MOTION: {
						DispatchEvent<MotionEvent>(
							(int)evt.motion.x,
							(int)evt.motion.y,
							SDLButtonToMouseButton(evt.button.button)
						);
					} break;
					case SDL_EVENT_TEXT_INPUT: {
						DispatchEvent<TextInputEvent>(
							evt.text.text[0]
						);
					} break;
					case SDL_EVENT_KEY_DOWN: {
						DispatchEvent<KeyEvent>(
							SDLKeyToKey(evt.key.key),
							SDLModToModifierState(evt.key.mod),
							true
						);
					} break;
					case SDL_EVENT_KEY_UP: {
						DispatchEvent<KeyEvent>(
							SDLKeyToKey(evt.key.key),
							SDLModToModifierState(evt.key.mod),
							false
						);
					} break;
					case SDL_EVENT_MOUSE_WHEEL: {
						DispatchEvent<ScrollEvent>(
							evt.wheel.x,
							evt.wheel.y,
							(int)evt.wheel.mouse_x,
							(int)evt.wheel.mouse_y
						);
					} break;
				}
			}

			if (m_shouldRedraw) {
				if (m_root) m_root->SetLocalBounds(Rectangle(0, 0, m_width, m_height));
				Redraw();
				m_shouldRedraw = false;
			}
		}

		adapter->OnDestroy();
		delete adapter;

		SDL_DestroyWindow(m_window);
		SDL_DestroyRenderer(m_renderer);
		SDL_Quit();

		return 0;
	}

    void Application::SetRoot(Element *root)
    {
		// check if root is in elements
		auto it = std::find_if(m_elements.begin(), m_elements.end(), [&](const ElementPtr& el) {
			return el.get() == root;
		});
		if (it == m_elements.end()) {
			return;
		}
		m_root = root;
		RequestRedraw();
    }

    void Application::RequestRedraw()
    {
        m_shouldRedraw = true;
    }

    void Application::Focus(Element* e) {
		if (m_focused != nullptr) {
			m_focused->m_focused = false;
			m_eventSystem.Broadcast<BlurEvent>(m_focused);
		}
		if (e != nullptr) {
			e->m_focused = true;
			m_eventSystem.Broadcast<FocusEvent>(e);
		}
		m_focused = e;
	}

	void Application::StartInput() {
		SDL_StartTextInput(m_window);
	}

	void Application::StopInput() {
		SDL_StopTextInput(m_window);
	}

	void Application::SetClipboard(const std::string& str) {
		SDL_SetClipboardText(str.c_str());
	}

	std::string Application::GetClipboard() {
		return std::string(SDL_GetClipboardText());
	}

	uint Application::GetMod() {
		return SDL_GetModState();
	}

	void Application::Redraw() {
		m_graphics.SetupDrawing(m_width, m_height);

		if (m_root) {
			Json windowStyle = m_root->GetStyle()["Window"];
			m_graphics.StyledRect(0, 0, m_width, m_height, windowStyle);
		}

		for (size_t i = 0; i < m_elements.size(); i++) {
			if (m_elements[i]->GetParent() != nullptr) continue;
			if (!m_elements[i]->IsVisible()) continue;
			if (m_elements[i]->IsDirty()) {
				m_elements[i]->OnDraw(m_graphics);
				m_elements[i]->m_dirty = false;
			}
		}
		// Draw popups on top
		for (auto* popup : m_popups) {
			if (!popup->IsVisible()) continue;
			popup->OnDraw(m_graphics);
			popup->m_dirty = false;
		}
		m_graphics.Flush();

		unsigned char* data = cairo_image_surface_get_data(m_graphics.GetCairoSurface());
		int stride = cairo_image_surface_get_stride(m_graphics.GetCairoSurface());

		SDL_UpdateTexture(m_buffer, nullptr, data, stride);
		SDL_RenderTexture(m_renderer, m_buffer, nullptr, nullptr);
		SDL_RenderPresent(m_renderer);
	}

	void Application::RequestRedrawAll() {
		for (auto&& e : m_elements) {
			e->Invalidate();
		}
	}

    void Application::ResizeScreenBuffer()
    {
		if (m_buffer != nullptr) {
			SDL_DestroyTexture(m_buffer);
		}
		m_buffer = SDL_CreateTexture(
					m_renderer,
					SDL_PIXELFORMAT_ARGB8888,
					SDL_TEXTUREACCESS_STREAMING,
					m_width, m_height
		);
		SDL_SetTextureScaleMode(m_buffer, SDL_SCALEMODE_NEAREST);
    }

    void Application::ShowPopup(Element *popup) {
		// Avoid duplicates
		for (auto* p : m_popups) {
			if (p == popup) return;
		}
		popup->m_application = this;
		m_popups.push_back(popup);
		RequestRedraw();
	}

	void Application::DismissPopup(Element *popup) {
		m_popups.erase(
			std::remove(m_popups.begin(), m_popups.end(), popup),
			m_popups.end()
		);
		RequestRedraw();
	}

}
