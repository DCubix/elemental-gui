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

        Color GetSelected() const { return m_selected; }
        void SetSelected(const Color& color);

        float GetHue() const { return m_hue; }
        void SetHue(float hue) {
            m_hue = hue;
            m_selected = Color::FromHSVA(m_hue, m_saturation, m_value, m_selected.a);
            Invalidate();
        }

        float GetSaturation() const { return m_saturation; }
        void SetSaturation(float sat) {
            m_saturation = sat;
            m_selected = Color::FromHSVA(m_hue, m_saturation, m_value, m_selected.a);
            Invalidate();
        }

        float GetValue() const { return m_value; }
        void SetValue(float val) {
            m_value = val;
            m_selected = Color::FromHSVA(m_hue, m_saturation, m_value, m_selected.a);
            Invalidate();
        }

        void SetOnChange(ValueChanged<Color> onChange) { m_onChange = onChange; }

    private:
        enum Region { Unknown = 0, SatVal, Hue, Alpha };
        Region m_clicked{Unknown};

        bool m_dragging{false};

        float m_hue{0.0f}, m_saturation{0.0f}, m_value{0.0f};
        Color m_selected{Color::FromHex("#000")};

        ValueChanged<Color> m_onChange;
        // Image m_hueCircle;
        // void RecomputeHueCircle();

        Size GetSatValSize() const;
        void Pointer(Graphics& g, const PointI& p, int width, int height, float rotation);
        void Update(int x, int y);
    };
} // namespace gui