#include "Canvas.h"

constexpr int GRID_SIZE = 32;

Canvas::Canvas()
    : gui::Element() {
    SetLocalBounds({0, 0, 400, 400});
}

void Canvas::OnDraw(gui::Graphics& g) {
    auto b = GetLocalBounds();

    g.StyledRect(0, 0, b.w, b.h, m_style);

    const gui::Color shade0{0.5f, 0.5f, 0.5f, 1.0f};
    const gui::Color shade1{0.75f, 0.75f, 0.75f, 1.0f};

    g.ClipPushRect(0, 0, b.w, b.h);

    g.Rect(0, 0, b.w, b.h);
    g.Color(shade0.r, shade0.g, shade0.b);
    g.Fill();

    const int cellsX = b.w / GRID_SIZE;
    const int cellsY = b.h / GRID_SIZE;

    g.Color(shade1.r, shade1.g, shade1.b);
    for (int y = 0; y <= cellsY; y++) {
        for (int x = 0; x <= cellsX; x++) {
            if ((x & 1) != (y & 1)) {
                int cx = x * GRID_SIZE;
                int cy = y * GRID_SIZE;
                g.Rect(cx, cy, GRID_SIZE, GRID_SIZE);
                g.Fill();
            }
        }
    }

    g.ClipPop();
}
