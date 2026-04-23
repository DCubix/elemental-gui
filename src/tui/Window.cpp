#include "Window.h"

#include "Application.h"

namespace tui {

    Window::~Window()
    {
        if (m_renderer) {
            SDL_DestroyRenderer(m_renderer);
        }
        if (m_window) {
            SDL_DestroyWindow(m_window);
        }
    }

    Window::Window(const WindowConfig& config)
        : m_config(config)
    {
        // is SDL initialized?
        if (!SDL_WasInit(SDL_INIT_VIDEO)) {
            SDL_Init(SDL_INIT_VIDEO);
        }

        SDL_WindowFlags flags = SDL_WINDOW_HIDDEN;

        switch (config.style) {
            case WindowStyle::TopLevel: break;
            case WindowStyle::Modal:
                flags |= SDL_WINDOW_MODAL;
                break;
            case WindowStyle::Utility:
                flags |= SDL_WINDOW_UTILITY;
                break;
            case WindowStyle::Borderless:
                flags |= SDL_WINDOW_BORDERLESS;
                break;
            case WindowStyle::Fullscreen:
                flags |= SDL_WINDOW_FULLSCREEN;
                break;
        }

        m_window = SDL_CreateWindow(
            config.title.c_str(),
            config.width,
            config.height,
            flags
        );
        if (config.parent) {
            SDL_SetWindowParent(m_window, config.parent->m_window);
        }
        if (config.style == WindowStyle::Modal) {
            SDL_SetWindowModal(m_window, true);
        }

        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        m_config = config;

        m_graphics = Graphics::CreateGraphics();
        ResizeScreenBuffer();

        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
		SDL_RenderClear(m_renderer);
		SDL_RenderPresent(m_renderer);
    }

    void Window::Resize(uint32_t width, uint32_t height, bool reshape)
    {
        m_config.width = width;
        m_config.height = height;
        if (reshape) {
            SDL_SetWindowSize(m_window, width, height);
        }
        ResizeScreenBuffer();
        RequestRedrawAll();
    }

    void Window::Show()
    {
        SDL_ShowWindow(m_window);
        SDL_RaiseWindow(m_window);
        if (m_config.parent) {
            // position relative to parent
            int x, y;
            int pw, ph;
            SDL_GetWindowPosition(m_config.parent->m_window, &x, &y);
            SDL_GetWindowSize(m_config.parent->m_window, &pw, &ph);
            SDL_SetWindowPosition(m_window, x + pw/2 - m_config.width/2, y + ph/2 - m_config.height/2);
        }
        RequestRedraw();
    }

    void Window::Hide()
    {
        SDL_HideWindow(m_window);
    }

    void Window::Close()
    {
        m_closeRequested = true;
    }

    void Window::RequestRedraw()
    {
        m_shouldRedraw = true;
    }

    void Window::Focus(Element* e)
    {
        auto& esys = m_application->GetEventSystem();
        if (m_focused != nullptr) {
			m_focused->m_focused = false;
			esys.Broadcast<BlurEvent>(m_focused);
		}
		if (e != nullptr) {
			e->m_focused = true;
			esys.Broadcast<FocusEvent>(e);
		}
		m_focused = e;
    }

    void Window::StartInput()
    {
        SDL_StartTextInput(m_window);
    }

    void Window::StopInput()
    {
        SDL_StopTextInput(m_window);
    }

    void Window::ShowPopup(Element* popup)
    {
        // Avoid duplicates
		for (auto* p : m_popups) {
			if (p == popup) return;
		}
		popup->m_window = this;
		m_popups.push_back(popup);
		RequestRedraw();
    }

    void Window::DismissPopup(Element* popup)
    {
        m_popups.erase(
			std::remove(m_popups.begin(), m_popups.end(), popup),
			m_popups.end()
		);
		RequestRedraw();
    }

    void Window::SetRoot(Element *root)
    {
        m_root = root;
        RequestRedraw();
    }

    void Window::SetTitle(const std::string& title)
    {
        m_config.title = title;
        SDL_SetWindowTitle(m_window, title.c_str());
    }

    Size Window::GetSize() const
    {
        int w, h;
        SDL_GetWindowSize(m_window, &w, &h);
        return { w, h };
    }

    void Window::Update()
    {
        if (m_shouldRedraw) {
            if (m_root) m_root->SetLocalBounds(Rectangle(0, 0, m_config.width, m_config.height));
            Redraw();
            m_shouldRedraw = false;
        }
    }

    void Window::ResizeScreenBuffer()
    {
        if (m_buffer != nullptr) {
			SDL_DestroyTexture(m_buffer);
		}
		m_buffer = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            m_config.width, m_config.height
		);
    }

    void Window::Redraw()
    {
		m_graphics.SetupDrawing(m_config.width, m_config.height);

		if (m_root) {
			Json windowStyle = m_root->GetStyle()["Window"];
			m_graphics.StyledRect(0, 0, m_config.width, m_config.height, windowStyle);
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

    void Window::RequestRedrawAll()
    {
        for (auto&& e : m_elements) {
			e->Invalidate();
		}
    }

    void Window::HookEventsUp(Element* el)
    {
        auto& esys = m_application->GetEventSystem();
        esys.Subscribe(el, EventType::MouseButton);
        esys.Subscribe(el, EventType::MouseMotion);
        esys.Subscribe(el, EventType::TextInput);
        esys.Subscribe(el, EventType::Focus);
        esys.Subscribe(el, EventType::Blur);
        esys.Subscribe(el, EventType::Key);
        esys.Subscribe(el, EventType::Scroll);
    }

}