#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <concepts>

namespace tui {
	class Subscriber;

	// Default events
	enum class EventType {
		MouseButton = 0,
		MouseMotion,
		Key,
		TextInput,
		Focus,
		Blur,
		Scroll
	};

	enum class MouseButton : uint8_t {
		Unknown = 0,
		Left,
		Right,
		Middle,
		X1,
		X2
	};

	enum class Key : uint8_t {
		None,
		
		// Letters
		A, B, C, D, E, F, G, H, I, J, K, L, M,
		N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		
		// Numbers
		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
		
		// Numpad keys
		Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
		NumpadMultiply, NumpadDivide, NumpadAdd, NumpadSubtract, NumpadDecimal, NumpadPeriod,
		NumpadEnter,
		
		// Function keys
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,
		
		// Arrow keys
		Up, Down, Left, Right,
		
		// Modifier keys
		LeftShift, RightShift, LeftControl, RightControl, LeftAlt, RightAlt, LeftMeta, RightMeta,
		
		// Navigation and editing keys
		Space, Tab, Enter, Backspace, Delete, Insert, Home, End, PageUp, PageDown,
		PrintScreen, ScrollLock, Pause, CapsLock, NumLock, Escape,
		
		// Punctuation and symbols
		Semicolon, Equals, Comma, Minus, Period, Slash, Backslash, Grave, LeftBracket, RightBracket, Apostrophe,
		
		// Media keys
		Mute, VolumeUp, VolumeDown, PlayPause, Stop, PreviousTrack, NextTrack,
		
		// Application keys
		Help, Menu, Select, Execute, Undo, Cut, Copy, Paste, Find, Again, Save, Print,
		
		// Browser keys
		BrowserBack, BrowserForward, BrowserRefresh, BrowserStop, BrowserSearch, BrowserFavorites, BrowserHome,
		
		// OEM specific keys
		Oem1, Oem2, Oem3, Oem4, Oem5, Oem6, Oem7, Oem8, Oem102, OemPlus, OemMinus, OemPeriod, OemComma,

		// Special case for iteration limits
		EnumEnd
	};

	struct ModifierState {
		bool shift{ false };
		bool control{ false };
		bool alt{ false };
		bool meta{ false };
	};

	struct Event {
		virtual EventType Type() const = 0;
	};

	template <typename E>
	concept DerivedFromEvent = std::derived_from<E, Event>;

	struct MouseEvent : public Event {
		EventType Type() const override { return EventType::MouseButton; }

		MouseEvent() = default;
		MouseEvent(int x, int y, MouseButton button, bool pressed)
			: x(x), y(y), button(button), pressed(pressed)
		{}
		
		int x, y;
		MouseButton button;
		bool pressed;
	};

	struct MotionEvent : public Event {
		EventType Type() const override { return EventType::MouseMotion; }

		MotionEvent() = default;
		MotionEvent(int x, int y, MouseButton button)
			: x(x), y(y), button(button)
		{}

		int x, y;
		MouseButton button;
	};

	struct TextInputEvent : public Event {
		EventType Type() const override { return EventType::TextInput; }

		TextInputEvent() = default;
		TextInputEvent(char c) : inputChar(c) {}

		char inputChar;
	};

	struct FocusEvent : public Event {
		EventType Type() const override { return EventType::Focus; }

		FocusEvent() = default;
		FocusEvent(Subscriber *e) : element(e) {}

		Subscriber *element;
	};

	struct BlurEvent : public Event {
		EventType Type() const override { return EventType::Blur; }

		BlurEvent() = default;
		BlurEvent(Subscriber *e) : element(e) {}

		Subscriber *element;
	};

	struct KeyEvent : public Event {
		EventType Type() const override { return EventType::Key; }

		KeyEvent() = default;
		KeyEvent(Key key, ModifierState mod, bool pressed)
			: key(key), mod(mod), pressed(pressed)
		{}

		Key key;
		ModifierState mod;
		bool pressed;
	};

	struct ScrollEvent : public Event {
		EventType Type() const override { return EventType::Scroll; }

		ScrollEvent() = default;
		ScrollEvent(float scrollX, float scrollY, int mouseX, int mouseY)
			: scrollX(scrollX), scrollY(scrollY), mouseX(mouseX), mouseY(mouseY)
		{}

		float scrollX, scrollY;
		int mouseX, mouseY;
	};

	//
	enum class EventStatus {
		Consumed = 0,
		Active
	};
	class Subscriber {
	public:
		virtual EventStatus OnEvent(Event *event) = 0;

		bool IsEnabled() const { return m_enabled; }
		void SetEnabled(bool v) { m_enabled = v; }

		bool IsHighPriority() const { return m_highPriority; }
		void SetHighPriority(bool v) { m_highPriority = v; }

	private:
		bool m_enabled = true;
		bool m_highPriority = false;
	};
	using EventSubscriberMap = std::map<EventType, std::vector<Subscriber*>>;

	class EventSystem {
	public:
		EventSystem() = default;
		~EventSystem() = default;

		void Subscribe(Subscriber *sub, EventType event);

		template <DerivedFromEvent E, typename... Args>
		void Broadcast(Args&&... args) {
			E *e = new E(std::forward<Args>(args)...);

			auto evt = m_subscribers.find(e->Type());
			if (evt == m_subscribers.end()) return;

			bool consumed = false;
			for (auto&& sub : evt->second) {
				if (!sub->IsEnabled()) continue;

				// Motion events must reach all subscribers so elements
				// can clear their hover state when the mouse leaves.
				if (consumed && e->Type() != EventType::MouseMotion) {
					break;
				}

				EventStatus status = sub->OnEvent(e);
				if (status == EventStatus::Consumed) {
					consumed = true;
				}
			}

			delete e;
		}

	private:
		EventSubscriberMap m_subscribers;
	};
}
