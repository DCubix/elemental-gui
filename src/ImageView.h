#pragma once

#include "Element.h"

namespace gui {
    enum class ImageScalingMode { Stretch, Contain, Cover };

    class ImageView : public Element {
    public:
        ImageView();

        void OnDraw(Graphics& g) override;
        Size GetPreferredSize() const override;

        Property<ImageScalingMode> scalingMode{ImageScalingMode::Stretch};
        Property<Image*> image{nullptr};
    };
} // namespace gui
