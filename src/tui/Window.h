#pragma once

#include <SDL3/SDL.h>
#include <string>

#include "Application.h"
#include "Element.h"
#include "Graphics.h"

namespace tui {
    enum class WindowStyle {
        TopLevel,
        Modal,
        Utility,
        Borderless,
        Fullscreen
    };

    using WindowId = uint64_t;

    class Window;
    struct WindowConfig {
        std::string title;
        uint32_t width{ 800 };
        uint32_t height{ 600 };
        bool resizable{ false };
        WindowStyle style{ WindowStyle::TopLevel };
        Window* parent{ nullptr };
    };

    class Window {
        friend class Application;
    public:
        Window() = default;
        ~Window();

        Window(const WindowConfig& config);

        void Resize(uint32_t width, uint32_t height, bool reshape = true);

        void Show();
        void Hide();
        void Close();

        void RequestRedraw();
        void Focus(Element* e);

        void StartInput();
		void StopInput();

        template <DerivedFromElement E>
		E& Create() {
			m_elements.push_back(std::make_unique<E>());
			m_elements.back()->m_window = this;
            HookEventsUp(m_elements.back().get());
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

		template <DerivedFromElement E>
		E* Find(std::function<bool(E*)> predicate = nullptr) {
			for (auto&& el : m_elements) {
				E* casted = dynamic_cast<E*>(el.get());
				if (casted && (predicate == nullptr || predicate(casted))) {
					return casted;
				}
			}
			return nullptr;
		}

		void ShowPopup(Element* popup);
		void DismissPopup(Element* popup);

		Element& GetRoot() { return *m_root; }
		void SetRoot(Element *root);

        std::string GetTitle() const { return m_config.title; }
        void SetTitle(const std::string& title);

        Size GetSize() const;

        WindowId GetId() const { return SDL_GetWindowID(m_window); }
        Application* GetApp() { return m_application; }
        Graphics& GetGraphics() { return m_graphics; }

    protected:
        Application* m_application{ nullptr };

        SDL_Window* m_window{ nullptr };
        SDL_Renderer* m_renderer{ nullptr };
        SDL_Texture* m_buffer{ nullptr };
        Graphics m_graphics;

        WindowConfig m_config{};

        std::vector<std::unique_ptr<Element>> m_elements;
		std::vector<Element*> m_popups;
		Element* m_focused{ nullptr };
		Element* m_root{ nullptr };

        bool m_shouldRedraw{ false }, m_closeRequested{ false };

        void ResizeScreenBuffer();
        void Redraw();
        void RequestRedrawAll();
        void Update();

        void HookEventsUp(Element* el);

        template <DerivedFromEvent E, typename... Args>
		void DispatchEvent(Args&&... args)
        {
            E event(std::forward<Args>(args)...);
            // Try popups first (topmost = last)
            for (auto it = m_popups.rbegin(); it != m_popups.rend(); ++it) {
                if ((*it)->OnEvent(&event) == EventStatus::Consumed)
                    return;
            }

            // Dispatch only to this window's root elements
            // Container handles propagation to children
            bool consumed = false;
            for (auto&& el : m_elements) {
                if (!el->IsEnabled()) continue;
                if (el->GetParent() != nullptr) continue; // Only root elements

                if (consumed && event.Type() != EventType::MouseMotion) {
                    break;
                }

                EventStatus status = el->OnEvent(&event);
                if (status == EventStatus::Consumed) {
                    consumed = true;
                }
            }
        }
    };
}