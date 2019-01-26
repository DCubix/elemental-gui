#ifndef TUI_APPLICATION_H
#define TUI_APPLICATION_H

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <type_traits>

#include "SDL2/SDL.h"

#include "Graphics.h"
#include "EventSystem.h"
#include "Element.h"

using uint = uint32_t;

namespace tui {
	using ElementPtr = std::unique_ptr<Element>;

	class Application;
	class ApplicationAdapter {
	public:
		virtual void onCreate(Application *app) = 0;
		virtual void onDestroy() = 0;
	};

	class Application {
	public:
		Application() = default;
		~Application() = default;

		Application(const std::string& title, uint width, uint height);
		int start(ApplicationAdapter *adapter);

		template <typename E>
		E* create(Layout::LayoutDirection dir = Layout::LayoutDirection::None) {
			static_assert(std::is_base_of<Element, E>::value, "E must inherit from Element");
			m_elements.push_back(std::unique_ptr<Element>(new E()));
			m_layout.set(m_elements.back().get(), dir);
			m_elements.back()->m_application = this;

			// Subscribe this element to the event system
			m_eventSystem.subscribe(m_elements.back().get(), EventType::MouseEventType);
			m_eventSystem.subscribe(m_elements.back().get(), EventType::MotionEventType);
			m_eventSystem.subscribe(m_elements.back().get(), EventType::TextInputEventType);
			m_eventSystem.subscribe(m_elements.back().get(), EventType::FocusEventType);
			m_eventSystem.subscribe(m_elements.back().get(), EventType::BlurEventType);
			m_eventSystem.subscribe(m_elements.back().get(), EventType::KeyEventType);

			return dynamic_cast<E*>(m_elements.back().get());
		}

		void requestRedraw();
		void focus(Element *e);

		Json style() const { return m_style; }
		void style(Json style) { m_style = style; }

		void startInput();
		void stopInput();

		static Json DefaultStyle;
	private:
		SDL_Window *m_window;
		SDL_Renderer *m_renderer;

		Graphics m_graphics;
		EventSystem m_eventSystem;

		Json m_style;
		std::vector<ElementPtr> m_elements;
		Element *m_focused;
		Layout m_layout;

		std::string m_title;
		uint m_width, m_height;
		bool m_shouldRedraw;

		void redraw();
		void requestRedrawAll();
	};
}

#endif // TUI_APPLICATION_H
