#include "Application.h"

#include "Panel.h"
#include "FlexLayout.h"

#include <algorithm>

namespace tui {

	static const std::string DefaultStyleJson =
		#include "generated/NeumorphismDark.h"
			;
	Json Application::DefaultStyle = Json::parse(DefaultStyleJson);

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

		m_graphics = Graphics(m_renderer);
		m_graphics.SetViewport(m_width, m_height);

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
							m_graphics.SetViewport(m_width, m_height);
					} break;
					case SDL_EVENT_MOUSE_BUTTON_DOWN: {
						DispatchEvent<MouseEvent>(
									(int)evt.button.x,
									(int)evt.button.y,
									evt.button.button,
									true
						);
					} break;
					case SDL_EVENT_MOUSE_BUTTON_UP: {
						DispatchEvent<MouseEvent>(
									(int)evt.button.x,
									(int)evt.button.y,
									evt.button.button,
									false
						);
					} break;
					case SDL_EVENT_MOUSE_MOTION: {
						DispatchEvent<MotionEvent>(
									(int)evt.motion.x,
									(int)evt.motion.y,
									evt.button.button
						);
					} break;
					case SDL_EVENT_TEXT_INPUT: {
						DispatchEvent<TextInput>(
									evt.text.text[0]
						);
					} break;
					case SDL_EVENT_KEY_DOWN: {
						DispatchEvent<KeyEvent>(
									evt.key.key,
									evt.key.mod,
									true
						);
					} break;
					case SDL_EVENT_KEY_UP: {
						DispatchEvent<KeyEvent>(
									evt.key.key,
									evt.key.mod,
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
		//m_graphics.Clear();
		m_graphics.Draw([&](Graphics& g) {
			for (size_t i = 0; i < m_elements.size(); i++) {
				if (m_elements[i]->GetParent() != nullptr) continue;
				if (!m_elements[i]->IsVisible()) continue;
				if (m_elements[i]->IsDirty()) {
					m_elements[i]->OnDraw(g);
					m_elements[i]->m_dirty = false;
				}
			}
			// Draw popups on top
			for (auto* popup : m_popups) {
				if (!popup->IsVisible()) continue;
				popup->OnDraw(g);
				popup->m_dirty = false;
			}
		});
	}

	void Application::RequestRedrawAll() {
		for (auto&& e : m_elements) {
			e->Invalidate();
		}
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
