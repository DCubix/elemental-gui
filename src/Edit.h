#pragma once
#include "LineEdit.h"

namespace gui {
    class Edit : public LineEdit {
    public:
        Edit();
        void OnCreate() override;
        void OnDraw(Graphics& g) override;
        std::string StyleKey() const override { return "Edit"; }
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnFocus(FocusEvent e) override;
        void OnBlur(BlurEvent e) override;
    };
}
