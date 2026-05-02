#include "ColorPicker.h"

namespace gui {
    constexpr int SLIDER_SIZE = 18;
    constexpr int SLIDER_GAP = 8;

    ColorPicker::ColorPicker()
        : Element() {
        SetLocalBounds({0, 0, 200, 200});

        selected.Bind([this](const Color& col) {
            float vMax = std::max({col.r, col.g, col.b});
            float vMin = std::min({col.r, col.g, col.b});
            float delta = vMax - vMin;

            value = vMax;
            saturation = (vMax < 1e-10f) ? 0.0f : delta / vMax;

            if (delta < 1e-10f) {
                hue = 0.0f;
            } else {
                if (vMax == col.r)
                    hue = 60.0f * std::fmod((col.g - col.b) / delta, 6.0f);
                else if (vMax == col.g)
                    hue = 60.0f * ((col.b - col.r) / delta + 2.0f);
                else
                    hue = 60.0f * ((col.r - col.g) / delta + 4.0f);
            }

            if (hue() < 0.0f)
                hue += 360.0f;

            if (m_onChange)
                m_onChange(selected());

            Invalidate();
        });

        hue.Bind([this](const float& v) {
            selected = Color::FromHSVA(v, saturation(), value(), selected().a);
        });
        saturation.Bind([this](const float& v) {
            selected = Color::FromHSVA(hue(), v, value(), selected().a);
        });
        value.Bind([this](const float& v) {
            selected = Color::FromHSVA(hue(), saturation(), v, selected().a);
        });

        selected = Color::FromHSVA(0.0f, 0.0f, 1.0f, 1.0f);
    }

    void ColorPicker::OnDraw(Graphics& g) {
        const Size sz = GetSatValSize();

        { /// Draw Hue slider
            int sliderX = sz.w + SLIDER_GAP;
            g.Rect(sliderX, 0, SLIDER_SIZE, sz.h);

            std::vector<ColorStop> stops;
            const int steps = 12;
            for (int i = 0; i <= steps; i++) {
                float f = (float(i) / steps);
                float hue = f * 360.0f;
                stops.emplace_back(f, Color::FromHSV(hue, 1.0f, 1.0f));
            }

            g.LinearGradient(stops, {0, 0}, {0, float(sz.h)});
            g.Fill();

            int huePos = (hue() / 360.0f) * sz.h;
            Pointer(g, {sliderX, huePos}, SLIDER_SIZE, 8, 0.0f);
            g.Color(1.0f, 1.0f, 1.0f);
            g.Fill(true);
            g.Color(0.0f, 0.0f, 0.0f);
            g.LineWidth(0.5f);
            g.Stroke();
        }

        { /// Draw alpha slider
            const gui::Color shade0{0.5f, 0.5f, 0.5f, 1.0f};
            const gui::Color shade1{0.75f, 0.75f, 0.75f, 1.0f};

            int sliderY = sz.h + SLIDER_GAP;

            g.ClipPushRect(0, sliderY, sz.w, SLIDER_SIZE);

            g.DrawCheckerboard(0, sliderY, sz.w, SLIDER_SIZE, 6);

            g.Rect(0, sliderY - 1, sz.w, SLIDER_SIZE + 2);
            g.LinearGradient(
                {
                    {0.0f, Color::FromHSVA(hue(), saturation(), value(), 0.0f)},
                    {1.0f, Color::FromHSVA(hue(), saturation(), value(), 1.0f)},
                },
                {0, 0},
                {float(sz.w), 0}
            );
            g.Fill();

            g.ClipPop();

            int alphaPos = selected().a * sz.w;
            Pointer(g, {alphaPos, sliderY}, SLIDER_SIZE, 8, M_PI / 2.0f);
            g.Color(1.0f, 1.0f, 1.0f);
            g.Fill(true);
            g.Color(0.0f, 0.0f, 0.0f);
            g.LineWidth(0.5f);
            g.Stroke();
        }

        { /// Draw Sat/Val area
            g.Rect(0, 0, sz.w, sz.h);

            // white bg
            g.Color(1.0f, 1.0f, 1.0f);
            g.Fill(true);

            // color gradient
            auto selHue = Color::FromHSV(hue(), 1.0f, 1.0f);
            g.LinearGradient(
                {
                    {0.0f, gui::Color::FromRGBA(selHue.r, selHue.g, selHue.b, 0.0f)},
                    {1.0f, selHue},
                },
                {0, 0},
                {float(sz.w), 0}
            );
            g.Fill(true);

            // dark gradient
            g.LinearGradient(
                {
                    {0.0f, gui::Color::FromRGBA(0.0f, 0.0f, 0.0f, 0.0f)},
                    {1.0f, gui::Color::FromRGBA(0.0f, 0.0f, 0.0f, 1.0f)},
                },
                {0, 0},
                {0, float(sz.h)}
            );
            g.Fill();

            int svX = saturation() * sz.w;
            int svY = sz.h - (value() * sz.h);
            g.Arc(svX, svY, 3.0f, 0.0f, 2.0f * M_PI);
            g.Color(1.0f, 1.0f, 1.0f);
            g.Fill(true);
            g.Color(0.0f, 0.0f, 0.0f);
            g.LineWidth(0.5f);
            g.Stroke();
        }
    }

    void ColorPicker::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        const Size sz = GetSatValSize();
        if (Rectangle{0, 0, sz.w, sz.h}.HasPoint(e.x, e.y))
            m_clicked = SatVal;
        else if (Rectangle{sz.w + SLIDER_GAP, 0, SLIDER_SIZE, sz.h}.HasPoint(e.x, e.y))
            m_clicked = Hue;
        else if (Rectangle{0, sz.h + SLIDER_GAP, sz.w, SLIDER_SIZE}.HasPoint(e.x, e.y))
            m_clicked = Alpha;
        else
            m_clicked = Unknown;

        Update(e.x, e.y);
    }

    void ColorPicker::OnMouseMove(MotionEvent e) {
        Update(e.x, e.y);
    }

    void ColorPicker::OnMouseUp(MouseEvent e) {
        m_clicked = Unknown;
    }

    Size ColorPicker::GetSatValSize() const {
        const Size sz = GetSize();
        return {sz.w - (SLIDER_SIZE + SLIDER_GAP), sz.h - (SLIDER_SIZE + SLIDER_GAP)};
    }

    void ColorPicker::Pointer(Graphics& g, const PointI& p, int width, int height, float rotation) {
        PointF pts[] = {
            {static_cast<float>(p.x + width), static_cast<float>(p.y - height / 2)},
            {static_cast<float>(p.x + width), static_cast<float>(p.y + height / 2)},
        };

        auto fnRot = [](const PointF& p, float rot) -> PointF {
            return {
                p.x * std::cos(rot) - p.y * std::sin(rot),
                p.x * std::sin(rot) + p.y * std::cos(rot),
            };
        };

        auto pf = PointF{
            static_cast<float>(p.x),
            static_cast<float>(p.y),
        };
        for (PointF& pt : pts) {
            auto tp = pt - pf;
            auto rotated = fnRot(tp, rotation);
            pt = rotated + pf;
        }

        g.BeginPath();
        g.MoveTo(p.x, p.y);
        g.LineTo(pts[0].x, pts[0].y);
        g.LineTo(pts[1].x, pts[1].y);
        g.ClosePath();
    }

    void ColorPicker::Update(int x, int y) {
        const Size sz = GetSatValSize();

        float fx = static_cast<float>(x) / sz.w;
        float fy = static_cast<float>(y) / sz.h;

        if (m_clicked != Unknown) {
            switch (m_clicked) {
                case SatVal:
                    saturation = std::clamp(fx, 0.0f, 1.0f);
                    value = 1.0f - std::clamp(fy, 0.0f, 1.0f);
                    break;
                case Hue:
                    hue = std::clamp(fy, 0.0f, 1.0f) * 360.0f;
                    break;
                case Alpha:
                    selected.Set(
                        gui::Color{
                            selected().r,
                            selected().g,
                            selected().b,
                            std::clamp(fx, 0.0f, 1.0f)
                        }
                    );
                    break;
            }
            return;
        }
    }
} // namespace gui