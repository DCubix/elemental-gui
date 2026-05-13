#pragma once

#include "Element.h"
#include "TextInputController.h"

namespace gui {

    class Spinner : public Element {
    public:
        Spinner();

        void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "Spinner"; }
        bool NeedsTextInput() const override { return m_editing; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseDoubleClick(MouseDoubleClickEvent e) override;
        void OnMouseMove(MotionEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnScroll(ScrollEvent e) override;
        void OnKeyDown(KeyEvent e) override;
        void OnTextInput(TextInputEvent e) override;
        void OnFocus(FocusEvent e) override;
        void OnBlur(BlurEvent e) override;

        const Range& GetRange() const { return m_range; }
        void SetRange(float min, float max);

        Property<float> value{0.0f};
        Property<float> step{1.0f};
        Property<std::string> formatString{"{:.2f}"};

    private:
        enum class PartState { Normal, Hover, Click };

        Range m_range{0.0f, 100.0f};
        PartState m_leftState{PartState::Normal};
        PartState m_rightState{PartState::Normal};

        // Drag-to-scrub
        bool m_dragging{false};
        int m_dragStartX{0};
        float m_dragStartValue{0.0f};
        static constexpr int DragThreshold = 3;
        static constexpr float DragSensitivity = 2.0f;

        // Inline text editing
        bool m_editing{false};
        TextInputController m_editor;
        Property<std::string> m_editText;

        Rectangle GetLeftButtonRect() const;
        Rectangle GetRightButtonRect() const;
        Rectangle GetCenterRect() const;

        void Step(float direction);
        void EnterEditMode();
        void CommitEdit();
        void CancelEdit();
        std::string FormatValue() const;
    };

} // namespace gui
