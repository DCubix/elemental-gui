#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <concepts>

#include "SDL3/SDL.h"

#include "Graphics.h"
#include "EventSystem.h"
#include "Element.h"

#include "Utils.h"
using namespace tui::utils;

namespace tui {
	using ElementPtr = std::unique_ptr<Element>;

	class Application;
	class ApplicationAdapter {
		public:
		virtual void OnCreate(Application& app) = 0;
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
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::MouseButton);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::MouseMotion);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::TextInput);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::Focus);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::Blur);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::Key);
			m_eventSystem.Subscribe(m_elements.back().get(), EventType::Scroll);

			return *dynamic_cast<E*>(m_elements.back().get());
		}

		template <DerivedFromElement E>
		E* FindByTag(const std::string& tag) {
			for (auto&& el : m_elements) {
				if (el->GetTag() == tag) {
					return dynamic_cast<E*>(el.get());
				}
			}
			return nullptr;
		}

		Element& GetRoot() { return *m_root; }
		void SetRoot(Element *root);

		void RequestRedraw();
		void Focus(Element *e);

		void StartInput();
		void StopInput();

		void SetClipboard(const std::string& str);
		std::string GetClipboard();

		uint GetWindowWidth() const { return m_width; }
		uint GetWindowHeight() const { return m_height; }

		uint GetMod();

		Graphics& GetGraphics() { return m_graphics; }

		static Json DefaultStyle;
	private:
		SDL_Window *m_window;
		SDL_Renderer *m_renderer;

		Graphics m_graphics;
		EventSystem m_eventSystem;

		std::vector<ElementPtr> m_elements;
		Element *m_focused;
		Element *m_root;

		std::string m_title;
		uint m_width, m_height;
		bool m_shouldRedraw, m_resizable;

		void Redraw();
		void RequestRedrawAll();
	};
}
