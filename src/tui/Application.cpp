#include "Application.h"

#include "Window.h"

#include <SDL3/SDL_keycode.h>
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

	int Application::Start(ApplicationAdapter* adapter) {
		if (adapter == nullptr) {
			return 1;
		}

		adapter->OnCreate(*this);

		bool running = true;
		SDL_Event evt{};

		while (running) {
			while (SDL_PollEvent(&evt)) {
				switch (evt.type) {
					case SDL_EVENT_QUIT: {
						auto win = FindWindow(evt.window.windowID);
						if (win) {
							win->Close();
						}
						running = false;
					} break;
					case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
						auto win = FindWindow(evt.window.windowID);
						if (win) {
							win->Close();
						}
					} break;
					case SDL_EVENT_WINDOW_EXPOSED: {
						auto win = FindWindow(evt.window.windowID);
						if (win) {
							win->Update();
						}
					} break;
					case SDL_EVENT_WINDOW_RESIZED:
					case SDL_EVENT_WINDOW_MAXIMIZED:
					case SDL_EVENT_WINDOW_RESTORED: {
						auto win = FindWindow(evt.window.windowID);
						if (win) {
							auto sz = win->GetSize();
							win->Resize(sz.w, sz.h, false);
						}
					} break;
					case SDL_EVENT_MOUSE_BUTTON_DOWN: {
						auto win = FindWindow(evt.button.windowID);
						if (win) {
							win->DispatchEvent<MouseEvent>(
								(int)evt.button.x,
								(int)evt.button.y,
								SDLButtonToMouseButton(evt.button.button),
								true
							);
						}
					} break;
					case SDL_EVENT_MOUSE_BUTTON_UP: {
						auto win = FindWindow(evt.button.windowID);
						if (win) {
							win->DispatchEvent<MouseEvent>(
								(int)evt.button.x,
								(int)evt.button.y,
								SDLButtonToMouseButton(evt.button.button),
								false
							);
						}
					} break;
					case SDL_EVENT_MOUSE_MOTION: {
						auto win = FindWindow(evt.motion.windowID);
						if (win) {
							win->DispatchEvent<MotionEvent>(
								(int)evt.motion.x,
								(int)evt.motion.y,
								SDLButtonToMouseButton(evt.button.button)
							);
						}
					} break;
					case SDL_EVENT_TEXT_INPUT: {
						auto win = FindWindow(evt.text.windowID);
						if (win) {
							win->DispatchEvent<TextInputEvent>(
								evt.text.text[0]
							);
						}
					} break;
					case SDL_EVENT_KEY_DOWN: {
						auto win = FindWindow(evt.key.windowID);
						if (win) {
							win->DispatchEvent<KeyEvent>(
								SDLKeyToKey(evt.key.key),
								SDLModToModifierState(evt.key.mod),
								true
							);
						}
					} break;
					case SDL_EVENT_KEY_UP: {
						auto win = FindWindow(evt.key.windowID);
						if (win) {
							win->DispatchEvent<KeyEvent>(
								SDLKeyToKey(evt.key.key),
								SDLModToModifierState(evt.key.mod),
								false
							);
						}
					} break;
					case SDL_EVENT_MOUSE_WHEEL: {
						auto win = FindWindow(evt.wheel.windowID);
						if (win) {
							win->DispatchEvent<ScrollEvent>(
								evt.wheel.x,
								evt.wheel.y,
								(int)evt.wheel.mouse_x,
								(int)evt.wheel.mouse_y
							);
						}
					} break;
				}
			}

			// Update all windows
			for (auto& win : m_windows) {
				win->Update();
			}
			// Remove closed windows (erase-remove idiom)
			m_windows.erase(
				std::remove_if(m_windows.begin(), m_windows.end(),
					[](const auto& win) { return win->m_closeRequested; }),
				m_windows.end()
			);
		}

		adapter->OnDestroy();
		delete adapter;

		m_windows.clear();

		SDL_Quit();

		return 0;
	}

	Window* Application::CreateWindow(const WindowConfig& config)
	{
		auto window = std::make_unique<Window>(config);
		m_windows.push_back(std::move(window));
		m_windows.back()->m_application = this;
		return m_windows.back().get();
	}

	void Application::SetClipboard(const std::string& str) {
		SDL_SetClipboardText(str.c_str());
	}

	std::string Application::GetClipboard() {
		return std::string(SDL_GetClipboardText());
	}

	Window* Application::FindWindow(WindowId id)
	{
		for (auto& window : m_windows) {
			if (window->GetId() == id) {
				return window.get();
			}
		}
		return nullptr;
	}

}
