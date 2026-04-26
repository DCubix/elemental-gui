#include "Element.h"

#include "Application.h"
#include "EventSystem.h"
#include "Window.h"

#include <cmath>

namespace gui {

    Element::Element()
        : m_parent(nullptr),
          m_bounds(0, 0, 50, 50),
          m_dirty(true),
          m_visible(true),
          m_focused(false),
          m_autoSize(false) {}

    void Element::OnDraw(Graphics& g) {}

    EventStatus Element::OnEvent(Event* event) {
        Rectangle intersectedBounds = GetIntersectedBounds();
        Rectangle localBounds = GetBounds();

        // Handle focus on mouse click
        if (event->Type() == EventType::MouseButton) {
            MouseEvent* e = dynamic_cast<MouseEvent*>(event);
            if (intersectedBounds.HasPoint(e->x, e->y) && e->pressed &&
                e->button == MouseButton::Left) {
                RequestFocus();
            }
        }

        switch (event->Type()) {
            case EventType::MouseButton: {
                MouseEvent eCopy = *dynamic_cast<MouseEvent*>(event);

                // Only accept clicks within bounds, but always accept releases if we're tracking a drag
                if (!intersectedBounds.HasPoint(eCopy.x, eCopy.y)) {
                    if (eCopy.pressed || !m_mouseDown) {
                        return EventStatus::Active;
                    }
                    // Release event outside bounds while dragging - still handle it
                }

                eCopy.x -= localBounds.x;
                eCopy.y -= localBounds.y;

                if (eCopy.pressed) {
                    m_mouseDown = true;
                    OnMouseDown(eCopy);
                } else {
                    m_mouseDown = false;
                    OnMouseUp(eCopy);
                }
                return EventStatus::Consumed;
            };
            case EventType::MouseMotion: {
                MotionEvent eCopy = *dynamic_cast<MotionEvent*>(event);

                bool isInBounds = intersectedBounds.HasPoint(eCopy.x, eCopy.y);

                // Handle hover state
                if (!isInBounds) {
                    if (m_hovered) {
                        m_hovered = false;
                        OnMouseLeave();
                    }
                    // If not dragging from this element, stop handling the event
                    if (!m_mouseDown) {
                        return EventStatus::Active;
                    }
                } else {
                    if (!m_hovered) {
                        m_hovered = true;
                        OnMouseEnter();
                    }
                }

                // If dragging from this element, always report motion events (even outside bounds)
                if (m_mouseDown) {
                    eCopy.x -= localBounds.x;
                    eCopy.y -= localBounds.y;
                    OnMouseMove(eCopy);
                    return EventStatus::Consumed;
                }

                // Normal hover motion (within bounds)
                if (isInBounds) {
                    eCopy.x -= localBounds.x;
                    eCopy.y -= localBounds.y;
                    OnMouseMove(eCopy);
                }
                return EventStatus::Consumed;
            };
            case EventType::Scroll: {
                ScrollEvent eCopy = *dynamic_cast<ScrollEvent*>(event);
                if (!intersectedBounds.HasPoint(eCopy.mouseX, eCopy.mouseY)) {
                    return EventStatus::Active;
                }
                eCopy.mouseX -= localBounds.x;
                eCopy.mouseY -= localBounds.y;
                OnScroll(eCopy);
                return EventStatus::Consumed;
            };
            case EventType::Key: {
                if (!m_focused)
                    return EventStatus::Active;
                KeyEvent eCopy = *dynamic_cast<KeyEvent*>(event);
                if (eCopy.pressed) {
                    OnKeyDown(eCopy);
                } else {
                    OnKeyUp(eCopy);
                }
                return EventStatus::Consumed;
            }
            case EventType::TextInput: {
                if (!m_focused)
                    return EventStatus::Active;
                TextInputEvent eCopy = *dynamic_cast<TextInputEvent*>(event);
                OnTextInput(eCopy);
                return EventStatus::Consumed;
            }
            case EventType::Focus: {
                FocusEvent eCopy = *dynamic_cast<FocusEvent*>(event);
                OnFocus(eCopy);
                return eCopy.element == this ? EventStatus::Consumed : EventStatus::Active;
            }
            case EventType::Blur: {
                BlurEvent eCopy = *dynamic_cast<BlurEvent*>(event);
                OnBlur(eCopy);
                return eCopy.element == this ? EventStatus::Consumed : EventStatus::Active;
            }
        }

        return EventStatus::Active;
    }

    void Element::Invalidate() {
        m_dirty = true;
        if (m_window)
            m_window->RequestRedraw();
    }

    void Element::RequestFocus() {
        if (m_window) {
            m_window->Focus(this);
            Invalidate();
        }
    }

    Json Element::GetStyle() const {
        const auto styleKey = StyleKey();
        Json appStyle = GetWindow()->GetApp()->GetStyle();
        Json mergedStyle = appStyle[styleKey];
        if (appStyle["DefaultText"].is_object()) {
            mergedStyle.update(appStyle["DefaultText"]);
        }
        if (mergedStyle.is_object() && m_style.is_object()) {
            mergedStyle.update(m_style);
        }
        return mergedStyle;
    }

    Rectangle Element::GetBounds() const {
        Rectangle b = m_bounds;
        if (m_parent != nullptr) {
            b.x += m_parent->GetBounds().x;
            b.y += m_parent->GetBounds().y;
        }
        return b;
    }

    Rectangle Element::GetLocalIntersectedBounds() const {
        Rectangle b = GetBounds();
        Rectangle c = GetIntersectedBounds();
        return {c.x - b.x, c.y - b.y, c.w, c.h};
    }

    Rectangle Element::GetIntersectedBounds() const {
        if (m_parent != nullptr) {
            // Returns only the visible part of the element
            // by intersecting it with the parent's intersected bounds
            return m_parent->GetIntersectedBounds()
                .Intersect(GetBounds())
                .value_or(Rectangle(0, 0, 0, 0));
        }
        return GetBounds();
    }

} // namespace gui
