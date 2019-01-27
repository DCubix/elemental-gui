#include "Application.h"

namespace tui {

	static const std::string DefaultStyleJson =
		#include "DefaultStyle.h"
			;
	Json Application::DefaultStyle = Json::parse(DefaultStyleJson);

	Application::Application(const std::string& title, uint width, uint height, bool resizable)
		: m_title(title), m_width(width), m_height(height), m_shouldRedraw(true),
		  m_focused(nullptr), m_resizable(resizable)
	{
		m_layout.gap(2);
		m_layout.padding(0);
		m_style = DefaultStyle;
	}

	int Application::start(ApplicationAdapter* adapter) {
		if (adapter == nullptr) {
			SDL_Log("Invalid adapter.");
			return 7;
		}

		int status = SDL_Init(SDL_INIT_EVERYTHING);
		if (status != 0) {
			SDL_Log("%s", SDL_GetError());
			return status;
		}

		int flags = SDL_WINDOW_SHOWN;
		if (m_resizable) {
			flags |= SDL_WINDOW_RESIZABLE;
		}
		m_window = SDL_CreateWindow(
					m_title.c_str(),
					SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
					m_width, m_height,
					flags
		);

		if (m_window == nullptr) {
			SDL_Log("%s", SDL_GetError());
			return 1;
		}

		m_renderer = SDL_CreateRenderer(m_window, 0, 0);
		if (m_renderer == nullptr) {
			SDL_Log("%s", SDL_GetError());
			return 1;
		}

		m_graphics = Graphics(m_renderer);
		m_graphics.setViewport(m_width, m_height);

		adapter->onCreate(this);

		bool running = true;
		SDL_Event evt{};

		while (running) {
			while (SDL_PollEvent(&evt)) {
				switch (evt.type) {
					case SDL_QUIT: running = false; break;
					case SDL_WINDOWEVENT: {
						if (evt.window.event == SDL_WINDOWEVENT_RESIZED ||
							evt.window.event == SDL_WINDOWEVENT_MAXIMIZED ||
							evt.window.event == SDL_WINDOWEVENT_RESTORED ||
							evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
						{
							int w, h;
							SDL_GetWindowSize(m_window, &w, &h);
							m_width = uint(w);
							m_height = uint(h);
							requestRedrawAll();
							m_graphics.setViewport(m_width, m_height);
						}
					} break;
					case SDL_MOUSEBUTTONDOWN: {
						m_eventSystem.broadcast<MouseEvent>(
									evt.button.x,
									evt.button.y,
									evt.button.button,
									true
						);
					} break;
					case SDL_MOUSEBUTTONUP: {
						m_eventSystem.broadcast<MouseEvent>(
									evt.button.x,
									evt.button.y,
									evt.button.button,
									false
						);
					} break;
					case SDL_MOUSEMOTION: {
						m_eventSystem.broadcast<MotionEvent>(
									evt.motion.x,
									evt.motion.y,
									evt.button.button
						);
					} break;
					case SDL_TEXTINPUT: {
						m_eventSystem.broadcast<TextInput>(
									evt.text.text[0]
						);
					} break;
					case SDL_KEYDOWN: {
						m_eventSystem.broadcast<KeyEvent>(
									evt.key.keysym.sym,
									evt.key.keysym.mod,
									true
						);
					} break;
					case SDL_KEYUP: {
						m_eventSystem.broadcast<KeyEvent>(
									evt.key.keysym.sym,
									evt.key.keysym.mod,
									false
						);
					} break;
				}
			}

			if (m_shouldRedraw) {
				redraw();
				m_shouldRedraw = false;
			}
		}

		adapter->onDestroy();
		delete adapter;

		SDL_DestroyWindow(m_window);
		SDL_DestroyRenderer(m_renderer);
		SDL_Quit();

		return 0;
	}

	void Application::requestRedraw() {
		m_shouldRedraw = true;
	}

	void Application::focus(Element* e) {
		if (m_focused != nullptr) {
			m_focused->m_focused = false;
			m_eventSystem.broadcast<BlurEvent>(m_focused);
		}
		if (e != nullptr) {
			e->m_focused = true;
			m_eventSystem.broadcast<FocusEvent>(e);
		}
		m_focused = e;
	}

	void Application::startInput() {
		SDL_StartTextInput();
	}

	void Application::stopInput() {
		SDL_StopTextInput();
	}

	void Application::clipboardSet(const std::string& str) {
		SDL_SetClipboardText(str.c_str());
	}

	std::string Application::clipboardGet() {
		return std::string(SDL_GetClipboardText());
	}

	uint Application::getMod() {
		return SDL_GetModState();
	}

	void Application::redraw() {
		m_layout.perform(0, 0, m_width, m_height);

		//m_graphics.clear();
		m_graphics.draw([&](Graphics& g) {
			for (auto&& e : m_elements) {
				if (e->parent() != nullptr) continue;
				if (!e->visible()) continue;
				if (e->dirty()) {
					e->onDraw(g);
					e->m_dirty = false;
				}
			}
		});
	}

	void Application::requestRedrawAll() {
		for (auto&& e : m_elements) {
			e->invalidate();
		}
	}

}
