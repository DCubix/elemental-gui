#pragma once

#include "Element.h"

namespace gui {
    namespace utils {
        struct MarkdownToken {
            bool bold{false}, italic{false};
            std::string text;
        };

        std::vector<MarkdownToken> ParseBasicMarkdown(const std::string& text);
        std::string CleanMarkdown(const std::string& text);
    } // namespace utils

    class Label : public Element {
    public:
        Label();

        void OnDraw(Graphics& g) override;

        Size GetPreferredSize() const override;

        std::string StyleKey() const override { return "DefaultText"; }

        Property<std::string> text;
        Property<Alignment> alignment{Alignment::TopLeft};
        Property<uint> iconSize{16u};
        Property<Image*> icon{nullptr};

    protected:
        Json m_textStyle;
    };
} // namespace gui
