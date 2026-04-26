#pragma once

#include "Element.h"

namespace gui {

    class Label : public Element {
    public:
        Label();

        void OnDraw(Graphics& g) override;

        Size GetPreferredSize() const override;

        std::string StyleKey() const override { return "DefaultText"; }

        std::string GetText() const { return m_text; }
        void SetText(const std::string& text) {
            m_text = text;
            Invalidate();
        }

        Alignment GetAlignment() const { return m_alignment; }
        void SetAlignment(Alignment align) {
            m_alignment = align;
            Invalidate();
        }

        Image* GetIcon() { return m_icon; }
        void SetIcon(Image* image) {
            m_icon = image;
            Invalidate();
        }

        uint GetIconSize() const { return m_iconSize; }
        void SetIconSize(uint size) {
            m_iconSize = size;
            Invalidate();
        }

    protected:
        std::string m_text;
        Alignment m_alignment;
        Image* m_icon;
        uint m_iconSize{16};
        Json m_textStyle;
    };
} // namespace gui
