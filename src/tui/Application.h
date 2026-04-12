#ifndef TUI_APPLICATION_H
#define TUI_APPLICATION_H

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <concepts>

#include "SDL3/SDL.h"

#include "Graphics.h"
#include "EventSystem.h"
#include "Element.h"

using uint = uint32_t;

namespace tui {
	template <typename E>
	concept DerivedFromElement = std::derived_from<E, Element>;

	using ElementPtr = std::unique_ptr<Element>;

	class Application;
	class ApplicationAdapter {
		public:
		virtual void OnCreate(Application *app) = 0;
		virtual void OnDestroy() = 0;
	};
	
	class Panel;
	class Application {
	public:
		Application() = default;
		~Application() = default;

		Application(const std::string& title, uint width, uint height, bool resizable = true);
		int Start(ApplicationAdapter *adapter);

		template <DerivedFromElement E>
		E& Create() {
			m_elements.push_back(std::make_unique<E>());
			m_elements.back()->m_application = this;

			// Subscribe this element to the event system
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::MouseEventType);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::MotionEventType);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::TextInputEventType);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::FocusEventType);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::BlurEventType);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::KeyEventType);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::ScrollEventType);

			return *dynamic_cast<E*>(m_elements.back().get());
		}

		Panel& GetRoot() { return *m_root; }

		void RequestRedraw();
		void Focus(Element *e);

		Json GetStyle() const { return m_style; }
		void SetStyle(Json style) { m_style = style; }

		void StartInput();
		void StopInput();

		void SetClipboard(const std::string& str);
		std::string GetClipboard();

		uint GetWindowWidth() const { return m_width; }
		uint GetWindowHeight() const { return m_height; }

		uint GetMod();

		static Json DefaultStyle;
	private:
		SDL_Window *m_window;
		SDL_Renderer *m_renderer;

		Graphics m_graphics;
		EventSystem m_eventSystem;

		Json m_style;

		std::vector<ElementPtr> m_elements;
		Element *m_focused;
		Panel *m_root;

		std::string m_title;
		uint m_width, m_height;
		bool m_shouldRedraw, m_resizable;

		void Redraw();
		void RequestRedrawAll();
	};
}

#endif // TUI_APPLICATION_H
