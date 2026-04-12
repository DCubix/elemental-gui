#pragma once

#include <map>
#include <vector>
#include <concepts>

namespace tui {
	class Subscriber;

	// Default events
	enum class EventType {
		MouseEventType = 0,
		MotionEventType,
		KeyEventType,
		TextInputEventType,
		FocusEventType,
		BlurEventType,
		ScrollEventType
	};

	struct Event {
		virtual EventType Type() = 0;
	};

	template <typename E>
	concept DerivedFromEvent = std::derived_from<E, Event>;

	struct MouseEvent : public Event {
		EventType Type() { return EventType::MouseEventType; }

		MouseEvent() = default;
		MouseEvent(int x, int y, int button, bool pressed)
			: x(x), y(y), button(button), pressed(pressed)
		{}
		
		int x, y, button;
		bool pressed;
	};

	struct MotionEvent : public Event {
		EventType Type() { return EventType::MotionEventType; }

		MotionEvent() = default;
		MotionEvent(int x, int y, int button)
			: x(x), y(y), button(button)
		{}

		int x, y, button;
	};

	struct TextInput : public Event {
		EventType Type() { return EventType::TextInputEventType; }

		TextInput() = default;
		TextInput(char c) : inputChar(c) {}

		char inputChar;
	};

	struct FocusEvent : public Event {
		EventType Type() { return EventType::FocusEventType; }

		FocusEvent() = default;
		FocusEvent(Subscriber *e) : element(e) {}

		Subscriber *element;
	};

	struct BlurEvent : public Event {
		EventType Type() { return EventType::BlurEventType; }

		BlurEvent() = default;
		BlurEvent(Subscriber *e) : element(e) {}

		Subscriber *element;
	};

	struct KeyEvent : public Event {
		EventType Type() { return EventType::KeyEventType; }

		KeyEvent() = default;
		KeyEvent(int key, int mod, bool pressed)
			: key(key), mod(mod), pressed(pressed)
		{}

		int key, mod;
		bool pressed;
	};

	struct ScrollEvent : public Event {
		EventType Type() { return EventType::ScrollEventType; }

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

	private:
		bool m_enabled = true;
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

			for (auto&& sub : evt->second) {
				if (!sub->IsEnabled()) continue;

				EventStatus status = sub->OnEvent(e);
				if (status == EventStatus::Consumed) {
					break;
				}
			}

			delete e;
		}

	private:
		EventSubscriberMap m_subscribers;
	};
}
