#pragma once

#include "Application.h"
#include "Element.h"
#include "Graphics.h"
#include "WindowConfig.h"

#include <string>

namespace gui {
    class Window {
        friend class Application;

    public:
        Window() = default;
        ~Window();

        Window(const WindowConfig& config);

        virtual std::function<Element*(Window&)> OnBuild() { return nullptr; }

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
            auto* el = m_elements.back().get();
            el->m_window = this;
            HookEventsUp(el);
            el->OnCreate();
            return *dynamic_cast<E*>(el);
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
        void SetRoot(Element* root);

        std::string GetTitle() const { return m_config.title; }
        void SetTitle(const std::string& title);

        bool IsResizable() const { return m_config.resizable; }
        void SetResizable(bool resizable);

        void SetParent(Window* parent);
        Window* GetParent() const { return m_config.parent; }

        void SetWindowStyle(WindowStyle style);
        WindowStyle GetWindowStyle() const { return m_config.style; }

        Size GetSize() const;

        WindowId GetId() const;
        Application* GetApp() { return m_application; }
        Graphics& GetGraphics() { return m_graphics; }

    protected:
        Application* m_application{nullptr};

        WindowHandle m_handle{nullptr};
        Graphics m_graphics;

        WindowConfig m_config{};

        std::vector<std::unique_ptr<Element>> m_elements;
        std::vector<Element*> m_popups;
        Element* m_focused{nullptr};
        Element* m_root{nullptr};

        bool m_shouldRedraw{false}, m_closeRequested{false};

        Backend& GetBackend();

        void ResizeScreenBuffer();
        void Redraw();
        void RequestRedrawAll();
        void Update();

        void HookEventsUp(Element* el);

        template <DerivedFromEvent E, typename... Args>
        void DispatchEvent(Args&&... args) {
            E event(std::forward<Args>(args)...);
            for (auto it = m_popups.rbegin(); it != m_popups.rend(); ++it) {
                if ((*it)->OnEvent(&event) == EventStatus::Consumed)
                    return;
            }

            bool consumed = false;
            for (auto&& el : m_elements) {
                if (!el->IsEnabled())
                    continue;
                if (el->GetParent() != nullptr)
                    continue;

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
} // namespace gui
