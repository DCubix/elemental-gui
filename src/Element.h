#pragma once

#include "DataBinding.h"
#include "EventSystem.h"
#include "Graphics.h"
#include "Utils.h"

#include <array>
#include <concepts>
#include <optional>
#include <regex>
#include <sstream>
using namespace gui::utils;

namespace gui {
    class Window;
    class Menu;
    template <typename>
    class List;
    class Element : public Subscriber {
        friend class Application;
        friend class Window;
        friend class Container;
        friend class ScrollView;
        friend class Menu;
        template <typename>
        friend class List;

    public:
        Element();
        ~Element() = default;

        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;
        Element(Element&&) = delete;
        Element& operator=(Element&&) = delete;

        virtual void OnDraw(Graphics& g);

        // Raw event handler - called by the application
        virtual EventStatus OnEvent(Event* event) override;

        // Event handlers - override these to handle specific events
        virtual void OnCreate() {}
        virtual void OnMouseDown(MouseEvent e) {}
        virtual void OnMouseMove(MotionEvent e) {}
        virtual void OnMouseUp(MouseEvent e) {}
        virtual void OnMouseEnter() {}
        virtual void OnMouseLeave() {}
        virtual void OnScroll(ScrollEvent e) {}
        virtual void OnKeyDown(KeyEvent e) {}
        virtual void OnKeyUp(KeyEvent e) {}
        virtual void OnTextInput(TextInputEvent e) {}
        virtual void OnFocus(FocusEvent e) {}
        virtual void OnBlur(BlurEvent e) {}
        virtual void OnResize() {}

        // Used for Auto-Size
        virtual Size GetPreferredSize() const { return {m_bounds.w, m_bounds.h}; }

        Rectangle GetBounds() const;
        Rectangle GetIntersectedBounds() const;
        Rectangle GetLocalIntersectedBounds() const;

        const Rectangle& GetLocalBounds() const { return m_bounds; }
        void SetLocalBounds(Rectangle b) {
            m_prevSize = GetSize();
            m_bounds = b;
            Invalidate();
        }

        PointI GetPosition() const { return {m_bounds.x, m_bounds.y}; }
        void SetPosition(PointI p) {
            m_bounds.x = p.x;
            m_bounds.y = p.y;
            Invalidate();
        }

        Size GetSize() const { return {m_bounds.w, m_bounds.h}; }
        void SetSize(Size s) {
            m_bounds.w = s.w;
            m_bounds.h = s.h;
            Invalidate();
        }

        Element* GetParent() const { return m_parent; }

        void Invalidate();
        void ShowPopup(Menu* el);

        bool IsVisible() const { return m_visible; }
        void SetVisible(bool v) {
            m_visible = v;
            Invalidate();
        }

        bool IsFocused() const { return m_focused; }
        void RequestFocus();

        bool IsAutoSize() const { return m_autoSize; }
        void SetAutoSize(bool as) {
            m_autoSize = as;
            Invalidate();
        }

        float GetFlexGrow() const { return m_flexGrow; }
        void SetFlexGrow(float fg) {
            m_flexGrow = fg;
            Invalidate();
        }

        Json GetStyle() const;
        void SetStyle(Json style) {
            m_style = style;
            Invalidate();
        }

        virtual std::string StyleKey() const { return ""; }
        virtual bool NeedsTextInput() const { return false; }

        Property<std::string>& GetTag() { return m_tag; }
        void SetTag(const std::string& tag) { m_tag = tag; }

        Window* GetWindow() const { return m_window; }

        void SetTooltip(const std::string& t) { m_tooltip = t; }
        const std::string& GetTooltip() const { return m_tooltip; }

    protected:
        Window* m_window{nullptr};
        Element* m_parent{nullptr};
        Rectangle m_bounds;
        Size m_prevSize{-1, -1};

        bool m_dirty{false}, m_visible{true}, m_focused{false}, m_autoSize{false}, m_hovered{false},
            m_mouseDown{false};
        float m_flexGrow{0.0f};

        Json m_style{};

        Property<std::string> m_tag{""};
        std::string m_tooltip{};

        virtual bool IsDirty() { return m_dirty; }
    };

    template <typename E>
    concept DerivedFromElement = std::derived_from<E, Element>;
} // namespace gui
