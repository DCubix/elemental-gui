#pragma once

#include "Element.h"

namespace gui {

    class Label : public Element {
    public:
        Label();

        void OnDraw(Graphics& g) override;

        Size GetPreferredSize() const override;

        std::string StyleKey() const override { return "DefaultText"; }

        Image* GetIcon() { return m_icon; }
        void SetIcon(Image* image) {
            m_icon = image;
            Invalidate();
        }

        Property<std::string> text;
        Property<Alignment> alignment{Alignment::TopLeft};
        Property<uint> iconSize{16u};

    protected:
        Image* m_icon{nullptr};
        Json m_textStyle;
    };
} // namespace gui
