#include "Canvas.h"

#include <stack>

constexpr int GRID_SIZE = 32;

Canvas::Canvas()
    : gui::Element() {
    SetLocalBounds({0, 0, 64, 64});
    image = gui::Image(64, 64);
    preview = gui::Image(64, 64);

    tools[static_cast<size_t>(ToolType::Pencil)] = std::make_unique<PencilTool>();
    tools[static_cast<size_t>(ToolType::Curve)] = std::make_unique<CurveTool>();
    tools[static_cast<size_t>(ToolType::Fill)] = std::make_unique<FillTool>();
    tools[static_cast<size_t>(ToolType::Eraser)] =
        std::make_unique<PencilTool>(gui::Color{0, 0, 0, 0});
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

    if (toolActive) {
        g.DrawImage(&preview, 0, 0, b.w, b.h, gui::ImageFiltering::Nearest);
    }

    g.ClipPop();
}

static int ZoomCoord(int c, float z) {
    return static_cast<int>(static_cast<float>(c) / z);
}

void Canvas::OnMouseDown(gui::MouseEvent e) {
    secondaryColor = e.button == gui::MouseButton::Right;

    auto b = GetBounds();
    prevMouse.x = e.x + b.x;
    prevMouse.y = e.y + b.y;
    dragging = true;
    toolActive = true;
    if (e.button != gui::MouseButton::Middle) {
        auto& tool = tools[static_cast<size_t>(selectedTool)];
        tool->OnMouseDown(*this, ZoomCoord(e.x, zoom), ZoomCoord(e.y, zoom));
        Invalidate();
    }
}

void Canvas::OnMouseMove(gui::MotionEvent e) {
    auto& tool = tools[static_cast<size_t>(selectedTool)];

    if (dragging) {
        if (e.button != gui::MouseButton::Middle) {
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
    } else {
        tool->OnMouseMove(*this, ZoomCoord(e.x, zoom), ZoomCoord(e.y, zoom));
        Invalidate();
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
    zoom += e.scrollY * 0.25f;
    zoom = std::clamp(zoom, 1.0f, 10.0f);

    auto sz = image.GetSize();
    float imgX = static_cast<float>(e.mouseX) / oldZoom;
    float imgY = static_cast<float>(e.mouseY) / oldZoom;
    float dz = zoom - oldZoom;
    viewPosition.x += static_cast<int>(dz * (static_cast<float>(sz.w) / 2.0f - imgX));
    viewPosition.y += static_cast<int>(dz * (static_cast<float>(sz.h) / 2.0f - imgY));

    Reposition();
}

void Canvas::OnKeyDown(gui::KeyEvent e) {
    if (e.key == gui::Key::LeftShift || e.key == gui::Key::RightShift) {
        shiftPressed = true;
    }
}

void Canvas::OnKeyUp(gui::KeyEvent e) {
    if (e.key == gui::Key::LeftShift || e.key == gui::Key::RightShift) {
        shiftPressed = false;
    }
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

void Canvas::Preview(const std::vector<algos::Pixel>& data) {
    preview.Lock();
    for (int y = 0; y < preview.GetHeight(); y++) {
        for (int x = 0; x < preview.GetWidth(); x++) {
            preview.SetPixel(x, y, gui::Color{0.0f, 0.0f, 0.0f, 0.0f});
        }
    }

    for (auto px : data) {
        preview.SetPixel(px.position.x, px.position.y, px.color);
    }
    preview.Unlock();
    Invalidate();
}

void Canvas::Draw(const std::vector<algos::Pixel>& data) {
    // TODO: undo/redo
    image.Lock();
    for (auto px : data) {
        image.SetPixel(px.position.x, px.position.y, px.color);
    }
    image.Unlock();
    Invalidate();
}

void PencilTool::OnMouseDown(Canvas& canvas, int x, int y) {
    if (buffer.capacity() < 1000) {
        buffer.reserve(1000);
    }

    prev.x = x;
    prev.y = y;
    buffer.push_back({{x, y}, colorOverride.value_or(canvas.GetCurrentColor())});
}

void PencilTool::OnMouseDrag(Canvas& canvas, int x, int y) {
    if (canvas.shiftPressed) {
        gui::Color color{};
        if (colorOverride.has_value()) {
            if (colorOverride.value().a > 0.001f) {
                color = colorOverride.value();
            } else {
                color = gui::Color::FromHex("#F00");
            }
        } else {
            color = canvas.GetCurrentColor();
        }
        canvas.Preview(algos::GetLine(prev, {x, y}, color));
    } else {
        auto pixels =
            algos::GetLine(prev, {x, y}, colorOverride.value_or(canvas.GetCurrentColor()));
        buffer.insert(buffer.end(), pixels.begin(), pixels.end());
        prev.x = x;
        prev.y = y;

        canvas.Preview(buffer);
    }
}

void PencilTool::OnMouseUp(Canvas& canvas, int x, int y) {
    if (buffer.empty())
        return;
    if (canvas.shiftPressed) {
        auto pixels =
            algos::GetLine(prev, {x, y}, colorOverride.value_or(canvas.GetCurrentColor()));
        buffer.insert(buffer.end(), pixels.begin(), pixels.end());
    }
    canvas.Draw(buffer);
    canvas.toolActive = false;
    buffer.clear();
}

void CurveTool::OnMouseDown(Canvas& canvas, int x, int y) {
    if (clickCount == 0) {
        pts[clickCount++] = {x, y};
    } else {
        clickCount = -1;
        canvas.Draw(algos::GetQuadBezier(pts[0], pts[2], pts[1], canvas.GetCurrentColor()));
        canvas.toolActive = false;
    }
}

void CurveTool::OnMouseMove(Canvas& canvas, int x, int y) {
    if (clickCount == 2) {
        pts[clickCount] = {x, y};
        canvas.Preview(algos::GetQuadBezier(pts[0], pts[2], pts[1], canvas.GetCurrentColor()));
    }
}

void CurveTool::OnMouseDrag(Canvas& canvas, int x, int y) {
    pts[clickCount] = {x, y};
    canvas.Preview(algos::GetLine(pts[0], pts[1], canvas.GetCurrentColor()));
}

void CurveTool::OnMouseUp(Canvas& canvas, int x, int y) {
    pts[++clickCount] = {x, y};
    if (pts[0].x == pts[1].x && pts[0].y == pts[1].y) {
        clickCount = 0;
    }
}

namespace algos {
    std::vector<Pixel> GetLine(gui::PointI a, gui::PointI b, gui::Color color) {
        std::vector<Pixel> ret;

        gui::PointI diff = (b - a).Abs();
        diff.y = -diff.y;

        gui::PointI slope{a.x < b.x ? 1 : -1, a.y < b.y ? 1 : -1};
        int err = diff.x + diff.y, e2;
        while (true) {
            ret.push_back({a, color});
            if (a.x == b.x && a.y == b.y)
                break;
            e2 = 2 * err;
            if (e2 >= diff.y) {
                err += diff.y;
                a.x += slope.x;
            }
            if (e2 <= diff.x) {
                err += diff.x;
                a.y += slope.y;
            }
        }

        return ret;
    }

    std::vector<Pixel>
    GetQuadBezier(gui::PointI a, gui::PointI b, gui::PointI c, gui::Color color) {
        std::vector<Pixel> ret;
        int steps = std::max(5, (gui::PointI(c - a).GetLength() / 2));

        auto lerp = [](float p0, float p1, float t) {
            return p0 + (p1 - p0) * t;
        };

        auto eval = [&](float t) -> gui::PointI {
            // De Casteljau: P = (1-t)^2 * A + 2(1-t)t * B + t^2 * C
            float x = lerp(lerp((float)a.x, (float)b.x, t), lerp((float)b.x, (float)c.x, t), t);
            float y = lerp(lerp((float)a.y, (float)b.y, t), lerp((float)b.y, (float)c.y, t), t);
            return {(int)std::round(x), (int)std::round(y)};
        };

        gui::PointI prev = eval(0.0f);
        for (int i = 1; i <= steps; ++i) {
            float t = (float)i / (float)steps;
            gui::PointI curr = eval(t);
            auto seg = GetLine(prev, curr, color);
            ret.insert(ret.end(), seg.begin(), seg.end());
            prev = curr;
        }

        return ret;
    }

    std::vector<Pixel> GetFloodFill(gui::Image& image, gui::PointI p, gui::Color color) {
        int w = image.GetWidth(), h = image.GetHeight();

        std::unordered_map<int, gui::Color> grid;
        grid.reserve(w * h);

        std::vector<algos::Pixel> ret;
        ret.reserve(w * h);

        std::stack<gui::PointI> stk;
        stk.push(p);

        const gui::PointI dirs[] = {
            {0, 1},
            {0, -1},
            {1, 0},
            {-1, 0},
        };

        image.Lock();
        const auto target = image.GetPixel(p.x, p.y);

        // copy image pixels
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                grid[x + y * w] = image.GetPixel(x, y);
            }
        }

        while (!stk.empty()) {
            auto cp = stk.top();
            stk.pop();

            if (cp.x < 0 || cp.x >= w || cp.y < 0 || cp.y >= h)
                continue;

            int index = cp.x + cp.y * w;
            if (grid[index] != target)
                continue;

            ret.push_back({cp, color});
            grid[index] = color;

            for (size_t i = 0; i < 4; i++)
                stk.push(cp + dirs[i]);
        }
        image.Unlock();

        return ret;
    }
} // namespace algos

void FillTool::OnMouseDown(Canvas& canvas, int x, int y) {
    canvas.Draw(algos::GetFloodFill(canvas.image, {x, y}, canvas.GetCurrentColor()));
}
