#include "Spinner.h"

#include "Application.h"
#include "Utils.h"
#include "Window.h"

#include <cmath>

namespace gui {

    static constexpr int ButtonWidth = 20;

    Spinner::Spinner()
        : Element() {
        SetLocalBounds(Rectangle(0, 0, 90, 24));
        TrackAll(value, step, formatString);

        m_editor.Init(this, &m_editText);
        m_editor.SetFilter(InputFilter::Numeric());
        m_editor.SetEditOps(
            {[this](char c) { m_editor.InsertChar(c); },
             [this](int i) { m_editor.RemoveChar(i); },
             [this]() { m_editor.DeleteSelected(); },
             []() {}}
        );

        m_editText.Bind([this](const auto&) {
            if (!m_editor.internalEdit) {
                m_editor.caretIndex = (int)m_editText().size();
                m_editor.selectionStart = -1;
                m_editor.selectionEnd = -1;
                m_editor.offsetX = 0;
            }
            if (m_window) {
                auto style = GetStyle();
                auto& g = GetWindow()->GetGraphics();
                m_editor.Rebuild(g, style);
            }
            Invalidate();
        });
    }

    std::string Spinner::FormatValue() const {
        try {
            return utils::Format(formatString(), value());
        } catch (...) {
            return std::to_string(value());
        }
    }

    Rectangle Spinner::GetLeftButtonRect() const {
        Size sz = GetSize();
        return Rectangle(0, 0, ButtonWidth, sz.h);
    }

    Rectangle Spinner::GetRightButtonRect() const {
        Size sz = GetSize();
        return Rectangle(sz.w - ButtonWidth, 0, ButtonWidth, sz.h);
    }

    Rectangle Spinner::GetCenterRect() const {
        Size sz = GetSize();
        return Rectangle(ButtonWidth, 0, sz.w - 2 * ButtonWidth, sz.h);
    }

    void Spinner::Step(float dir) {
        float snapped = std::round(value() / step()) * step();
        value = m_range.Constrain(snapped + dir * step());
        NotifyListeners();
    }

    void Spinner::EnterEditMode() {
        m_editing = true;
        m_editText = FormatValue();
        m_editor.caretIndex = (int)m_editText().size();
        m_editor.selectionStart = 0;
        m_editor.selectionEnd = m_editor.caretIndex;
        if (m_window) {
            auto style = GetStyle();
            auto& g = GetWindow()->GetGraphics();
            m_editor.Rebuild(g, style);
            m_window->StartInput();
        }
        Invalidate();
    }

    void Spinner::CommitEdit() {
        if (!m_editing)
            return;
        try {
            float v = std::stof(m_editText());
            value = m_range.Constrain(v);
            NotifyListeners();
        } catch (...) {
            // Invalid input — silently revert
        }
        CancelEdit();
    }

    void Spinner::CancelEdit() {
        m_editing = false;
        m_dragging = false;
        if (m_window)
            m_window->StopInput();
        Invalidate();
    }

    // ---- Drawing ----------------------------------------------------------------

    void Spinner::OnDraw(Graphics& g) {
        Size sz = GetSize();
        Json style = GetStyle();

        std::string stateName = "normal";
        if (m_editing || IsFocused())
            stateName = "focused";
        else if (m_hovered)
            stateName = "hover";

        g.StyledRect(0, 0, sz.w, sz.h, style[stateName]);

        // Center area: either the inline editor or the formatted value
        Rectangle center = GetCenterRect();
        if (m_editing) {
            m_editor.Draw(g, style, center, true);
        } else {
            EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
            int clipW = center.w - (int)pad.left - (int)pad.right;
            g.ClipPushRect(center.x + (int)pad.left, center.y, clipW, sz.h);
            g.StyledTextBegin(style);

            auto text = FormatValue();
            auto te = g.MeasureText(text);
            g.StyledTextEnd(
                text,
                center.x + (center.w / 2 - te.size.w / 2),
                (sz.h + (int)te.size.h) / 2
            );
            g.ClipPop();
        }

        auto buttonStyle = GetWindow()->GetApp()->GetStyle()["FlatButton"];
        // Left button highlight (decrement)
        Rectangle leftRect = GetLeftButtonRect();
        if (m_leftState != PartState::Normal) {
            std::string s = m_leftState == PartState::Click ? "click" : "hover";
            g.StyledRect(leftRect.x, leftRect.y, leftRect.w, leftRect.h, buttonStyle[s]);
        }

        // Right button highlight (increment)
        Rectangle rightRect = GetRightButtonRect();
        if (m_rightState != PartState::Normal) {
            std::string s = m_rightState == PartState::Click ? "click" : "hover";
            g.StyledRect(rightRect.x, rightRect.y, rightRect.w, rightRect.h, buttonStyle[s]);
        }

        g.Color(0.72f, 0.72f, 0.76f, 1.0f);

        // Left arrow
        {
            int cx = leftRect.x + leftRect.w / 2;
            int cy = leftRect.y + leftRect.h / 2;
            g.BeginSimplePath();
            g.AddPathPoint(cx + 3, cy - 4);
            g.AddPathPoint(cx - 3, cy);
            g.AddPathPoint(cx + 3, cy + 4);
            g.EndSimplePath(true);
            g.Fill();
        }

        // Right arrow
        {
            int cx = rightRect.x + rightRect.w / 2;
            int cy = rightRect.y + rightRect.h / 2;
            g.BeginSimplePath();
            g.AddPathPoint(cx - 3, cy - 4);
            g.AddPathPoint(cx + 3, cy);
            g.AddPathPoint(cx - 3, cy + 4);
            g.EndSimplePath(true);
            g.Fill();
        }
    }

    // ---- Events -----------------------------------------------------------------

    void Spinner::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;

        if (m_editing) {
            m_editor.HandleMouseDown(e, EdgeInsets::FromStyle(GetStyle()["padding"]), ButtonWidth);
            return;
        }

        if (GetLeftButtonRect().HasPoint(e.x, e.y)) {
            m_leftState = PartState::Click;
            Step(-1.0f);
        } else if (GetRightButtonRect().HasPoint(e.x, e.y)) {
            m_rightState = PartState::Click;
            Step(+1.0f);
        } else if (GetCenterRect().HasPoint(e.x, e.y)) {
            m_dragStartX = e.x;
            m_dragStartValue = value();
            m_dragging = false;
        }
        Invalidate();
    }

    void Spinner::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;

        if (m_editing) {
            m_editor.HandleMouseUp(e);
            return;
        }

        if (m_leftState == PartState::Click)
            m_leftState =
                GetLeftButtonRect().HasPoint(e.x, e.y) ? PartState::Hover : PartState::Normal;
        if (m_rightState == PartState::Click)
            m_rightState =
                GetRightButtonRect().HasPoint(e.x, e.y) ? PartState::Hover : PartState::Normal;

        m_dragging = false;
        Invalidate();
    }

    void Spinner::OnMouseDoubleClick(MouseDoubleClickEvent e) {
        if (e.button != MouseButton::Left)
            return;
        if (GetCenterRect().HasPoint(e.x, e.y))
            EnterEditMode();
    }

    void Spinner::OnMouseMove(MotionEvent e) {
        if (m_editing) {
            m_editor.HandleMouseMove(e, EdgeInsets::FromStyle(GetStyle()["padding"]), ButtonWidth);
            return;
        }

        // Drag to scrub
        if (e.button == MouseButton::Left && GetCenterRect().HasPoint(m_dragStartX, 0)) {
            int dx = e.x - m_dragStartX;
            if (!m_dragging && std::abs(dx) >= DragThreshold)
                m_dragging = true;
            if (m_dragging) {
                float steps = static_cast<float>(dx) / DragSensitivity;
                value = m_range.Constrain(m_dragStartValue + steps * step());
                NotifyListeners();
                Invalidate();
                return;
            }
        }

        // Button hover states (only when not dragging)
        if (!m_dragging) {
            PartState newLeft =
                GetLeftButtonRect().HasPoint(e.x, e.y) ? PartState::Hover : PartState::Normal;
            PartState newRight =
                GetRightButtonRect().HasPoint(e.x, e.y) ? PartState::Hover : PartState::Normal;
            if (m_leftState != PartState::Click && newLeft != m_leftState) {
                m_leftState = newLeft;
                Invalidate();
            }
            if (m_rightState != PartState::Click && newRight != m_rightState) {
                m_rightState = newRight;
                Invalidate();
            }
        }
    }

    void Spinner::OnMouseEnter() {
        Invalidate();
    }

    void Spinner::OnMouseLeave() {
        m_leftState = PartState::Normal;
        m_rightState = PartState::Normal;
        Invalidate();
    }

    void Spinner::OnScroll(ScrollEvent e) {
        if (!m_editing)
            Step(e.scrollY > 0 ? +1.0f : -1.0f);
    }

    void Spinner::OnKeyDown(KeyEvent e) {
        if (!m_editing)
            return;
        if (e.key == Key::Enter || e.key == Key::NumpadEnter) {
            CommitEdit();
        } else if (e.key == Key::Escape) {
            CancelEdit();
        } else {
            m_editor.HandleKeyDown(e);
        }
    }

    void Spinner::OnTextInput(TextInputEvent e) {
        if (m_editing)
            m_editor.HandleTextInput(e);
    }

    void Spinner::OnFocus(FocusEvent e) {
        Invalidate();
    }

    void Spinner::OnBlur(BlurEvent e) {
        if (m_editing)
            CommitEdit();
        Invalidate();
    }

    void Spinner::SetRange(float min, float max) {
        m_range.minimum = min;
        m_range.maximum = max;
        value = m_range.Constrain(value());
        Invalidate();
    }

} // namespace gui
