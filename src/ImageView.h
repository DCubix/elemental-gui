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

        ImageScalingMode GetScalingMode() const { return m_scalingMode; }
        void SetScalingMode(ImageScalingMode mode) {
            m_scalingMode = mode;
            Invalidate();
        }

        void OnDraw(Graphics& g) override;
        Size GetPreferredSize() const override;

    private:
        Image* m_image;
        ImageScalingMode m_scalingMode{ImageScalingMode::Stretch};
    };
} // namespace gui
