#pragma once
#include "Label.h"

namespace gui {
    class Tooltip : public Label {
    public:
        Tooltip();

        void OnDraw(Graphics& g) override;
        Size GetPreferredSize() const override;
        std::string StyleKey() const override { return "Tooltip"; }
    };
} // namespace gui
