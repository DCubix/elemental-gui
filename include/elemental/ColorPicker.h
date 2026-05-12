#pragma once

#include "Element.h"

namespace gui {
    class ColorPicker : public Element {
    public:
        ColorPicker();

        void OnDraw(Graphics& g) override;
        void OnMouseDown(MouseEvent e) override;
        void OnMouseMove(MotionEvent e) override;
        void OnMouseUp(MouseEvent e) override;

        Property<Color> selected{Color::FromHex("#000")};
        Property<float> hue{0.0f}, saturation{0.0f}, value{0.0f};

    private:
        enum Region { Unknown = 0, SatVal, Hue, Alpha };
        Region m_clicked{Unknown};

        bool m_dragging{false};

        Size GetSatValSize() const;
        void Pointer(Graphics& g, const PointI& p, int width, int height, float rotation);
        void Update(int x, int y);
    };
} // namespace gui