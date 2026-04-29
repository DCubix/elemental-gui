#include "Canvas.h"

constexpr int GRID_SIZE = 32;

Canvas::Canvas()
    : gui::Element() {
    SetLocalBounds({0, 0, 64, 64});
    image = gui::Image(64, 64);

    tools[0] = std::make_unique<PencilTool>();
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

    g.DrawImage(&image, 0, 0, b.w, b.h, gui::ImageFiltering::Nearest);

    g.ClipPop();
}

static int ZoomCoord(int c, float z) {
    return static_cast<int>(static_cast<float>(c) / z);
}

void Canvas::OnMouseDown(gui::MouseEvent e) {
    auto b = GetBounds();
    prevMouse.x = e.x + b.x;
    prevMouse.y = e.y + b.y;
    dragging = true;
    if (e.button != gui::MouseButton::Middle) {
        auto& tool = tools[static_cast<size_t>(selectedTool)];
        tool->OnMouseDown(*this, ZoomCoord(e.x, zoom), ZoomCoord(e.y, zoom));
        Invalidate();
    }
}

void Canvas::OnMouseMove(gui::MotionEvent e) {
    if (dragging) {
        if (e.button != gui::MouseButton::Middle) {
            auto& tool = tools[static_cast<size_t>(selectedTool)];
            tool->OnMouseDrag(*this, ZoomCoord(e.x, zoom), ZoomCoord(e.y, zoom));
            Invalidate();
        } else {
            auto b = GetBounds();
            int screenX = e.x + b.x;
            int screenY = e.y + b.y;
            viewPosition.x += screenX - prevMouse.x;
            viewPosition.y += screenY - prevMouse.y;
            prevMouse.x = screenX;
            prevMouse.y = screenY;
            Reposition();
        }
    }
}

void Canvas::OnMouseUp(gui::MouseEvent e) {
    auto& tool = tools[static_cast<size_t>(selectedTool)];
    tool->OnMouseUp(*this, ZoomCoord(e.x, zoom), ZoomCoord(e.y, zoom));
    dragging = false;
    Invalidate();
}

void Canvas::OnScroll(gui::ScrollEvent e) {
    float oldZoom = zoom;
    zoom += e.scrollY * 0.1f;
    zoom = std::clamp(zoom, 1.0f, 10.0f);

    auto sz = image.GetSize();
    float imgX = static_cast<float>(e.mouseX) / oldZoom;
    float imgY = static_cast<float>(e.mouseY) / oldZoom;
    float dz = zoom - oldZoom;
    viewPosition.x += static_cast<int>(dz * (static_cast<float>(sz.w) / 2.0f - imgX));
    viewPosition.y += static_cast<int>(dz * (static_cast<float>(sz.h) / 2.0f - imgY));

    Reposition();
}

gui::Size Canvas::GetPreferredSize() const {
    auto sz = image.GetSize();
    return {
        static_cast<int>(static_cast<float>(sz.w) * zoom),
        static_cast<int>(static_cast<float>(sz.h) * zoom),
    };
}

void Canvas::Reposition() {
    auto* parent = GetParent();

    auto cs = GetPreferredSize();
    auto ps = parent->GetSize();
    int cx = (ps.w / 2 - cs.w / 2) + viewPosition.x;
    int cy = (ps.h / 2 - cs.h / 2) + viewPosition.y;

    SetPosition({cx, cy});
    SetSize(cs);
    Invalidate();
}

void PencilTool::OnMouseDown(Canvas& canvas, int x, int y) {
    prev.x = x;
    prev.y = y;
    canvas.image.Lock();
    canvas.image.SetPixel(x, y, gui::Color::FromHex("#000"));
    canvas.image.Unlock();
}

void PencilTool::OnMouseDrag(Canvas& canvas, int x, int y) {
    DrawLineDDA(canvas, prev, {x, y}, gui::Color::FromHex("#000"));
    prev.x = x;
    prev.y = y;
}

void PencilTool::DrawLineDDA(
    Canvas& canvas,
    const gui::PointI& a,
    const gui::PointI& b,
    gui::Color col
) {
    int dx = b.x - a.x;
    int dy = b.y - a.y;

    int steps = std::max(std::abs(dx), std::abs(dy));

    float xInc = static_cast<float>(dx) / steps;
    float yInc = static_cast<float>(dy) / steps;

    float x = a.x, y = a.y;

    canvas.image.Lock();
    canvas.image.SetPixel(std::round(x), std::round(y), col);

    for (int i = 0; i < steps; i++) {
        x += xInc;
        y += yInc;
        canvas.image.SetPixel(std::round(x), std::round(y), col);
    }

    canvas.image.Unlock();
}
