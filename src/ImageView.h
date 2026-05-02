#pragma once

#include "Element.h"

namespace gui {
    enum class ImageScalingMode { Stretch, Contain, Cover };

    class ImageView : public Element {
    public:
        ImageView();

        Image* GetImage() { return m_image; }
        void SetImage(const Image* img) {
            m_image = const_cast<Image*>(img);
            Invalidate();
        }

        void OnDraw(Graphics& g) override;
        Size GetPreferredSize() const override;

        Property<ImageScalingMode> scalingMode{ImageScalingMode::Stretch};

    private:
        Image* m_image;
    };
} // namespace gui
