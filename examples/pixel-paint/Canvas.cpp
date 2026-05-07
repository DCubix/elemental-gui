#include "Canvas.h"

#include <fstream>
#include <set>
#include <stack>

constexpr int GRID_SIZE = 32;

Canvas::Canvas()
    : gui::Element() {
    SetLocalBounds({0, 0, 64, 64});
    imageSize = {64, 64};
    preview = gui::Image(imageSize.w, imageSize.h);
    layerOrders.PushBack(0);
    layers.PushBack(
        Layer{globalIdCounter++, gui::Image(imageSize.w, imageSize.h)}
    ); // Defaylt layer 0
    undoRedo = UndoRedo(this);

    tools[static_cast<size_t>(ToolType::Pencil)] = std::make_unique<PencilTool>();
    tools[static_cast<size_t>(ToolType::Curve)] = std::make_unique<CurveTool>();
    tools[static_cast<size_t>(ToolType::Fill)] = std::make_unique<FillTool>();
    tools[static_cast<size_t>(ToolType::Eraser)] =
        std::make_unique<PencilTool>(gui::Color{0, 0, 0, 0});
    tools[static_cast<size_t>(ToolType::Eyedrop)] = std::make_unique<PickerTool>();
    tools[static_cast<size_t>(ToolType::Square)] = std::make_unique<RectTool>();
    tools[static_cast<size_t>(ToolType::Circle)] = std::make_unique<EllipseTool>();
}

void Canvas::OnDraw(gui::Graphics& g) {
    auto b = GetLocalBounds();

    g.StyledRect(0, 0, b.w, b.h, m_style);

    const gui::Color shade0{0.5f, 0.5f, 0.5f, 1.0f};
    const gui::Color shade1{0.75f, 0.75f, 0.75f, 1.0f};

    g.ClipPushRect(0, 0, b.w, b.h);

    g.DrawCheckerboard(0, 0, b.w, b.h, GRID_SIZE);

    for (size_t i = layersOrdered.Size(); i-- > 0;) {
        auto* layer = layersOrdered[i];
        g.DrawImage(layer, 0, 0, b.w, b.h, gui::ImageFiltering::Nearest);
    }

    if (toolActive) {
        g.DrawImage(&preview, 0, 0, b.w, b.h, gui::ImageFiltering::Nearest);
    }

    if (zoom >= 3.0f) {
        auto sz = imageSize;
        g.Color(0.5f, 0.5f, 0.5f, 0.4f);
        g.LineWidth(1.0f);
        for (int x = 1; x < sz.w; x++)
            g.Line(x * zoom, 0, x * zoom, b.h);
        for (int y = 1; y < sz.h; y++)
            g.Line(0, y * zoom, b.w, y * zoom);
        g.Stroke();
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
    zoom += e.scrollY * 0.2f;
    zoom = std::clamp(zoom, 1.0f, 20.0f);

    auto sz = imageSize;
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
    } else if (e.key == gui::Key::Z && e.mod.control && undoRedo.CanUndo()) {
        Undo();
    } else if (e.key == gui::Key::Y && e.mod.control && undoRedo.CanRedo()) {
        Redo();
    }
}

void Canvas::OnKeyUp(gui::KeyEvent e) {
    if (e.key == gui::Key::LeftShift || e.key == gui::Key::RightShift) {
        shiftPressed = false;
    }
}

gui::Size Canvas::GetPreferredSize() const {
    auto sz = imageSize;
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
    CmdDrawPixels* cmd = new CmdDrawPixels();
    cmd->data = data;
    cmd->targetId = currentLayerId();
    undoRedo.Run(cmd);
    if (onImageChanged)
        onImageChanged();
    Invalidate();
}

void Canvas::Undo() {
    undoRedo.Undo();
    Invalidate();
}

void Canvas::Redo() {
    undoRedo.Redo();
    Invalidate();
}

void Canvas::LoadArtFile(const std::string& fileName) {
    std::ifstream fs(fileName, std::ios::binary);
    if (!fs.is_open())
        return;

    auto fnRead = [&]<typename T>(T& target) -> decltype(auto) {
        return fs.read(reinterpret_cast<char*>(&target), sizeof(T));
    };

#define fnReadV(T, v)                                                                              \
    T v;                                                                                           \
    fnRead(v)

    layers.Clear();
    layerOrders.Clear();
    undoRedo.Reset();

    // Image size
    fnRead(imageSize.w);
    fnRead(imageSize.h);

    // Color palette
    palette.Clear();
    fnReadV(uint, paletteSize);

    for (uint i = 0; i < paletteSize; i++) {
        gui::Color color{};
        fnRead(color.r);
        fnRead(color.g);
        fnRead(color.b);
        fnRead(color.a);
        palette.PushBack(color);
    }

    // Canvas specific
    fnRead(globalIdCounter);
    fnRead(zoom);
    fnRead(viewPosition.x);
    fnRead(viewPosition.y);

    fnRead(colors[0]().r);
    fnRead(colors[0]().g);
    fnRead(colors[0]().b);
    fnRead(colors[0]().a);
    fnRead(colors[1]().r);
    fnRead(colors[1]().g);
    fnRead(colors[1]().b);
    fnRead(colors[1]().a);

    // Layer ordering
    fnReadV(uint, layerCount);
    for (uint i = 0; i < layerCount; i++) {
        fnReadV(uint, order);
        layerOrders.PushBack(order);
    }

    // Layers
    for (uint i = 0; i < layerCount; i++) {
        fnReadV(uint, id);

        layers.PushBack(Layer{id, gui::Image(imageSize.w, imageSize.h)});

        gui::Image& img = layers[layers.Size() - 1].image;
        img.Lock();
        for (int y = 0; y < imageSize.h; y++) {
            for (int x = 0; x < imageSize.w; x++) {
                gui::Color color{};
                fnRead(color.r);
                fnRead(color.g);
                fnRead(color.b);
                fnRead(color.a);
                img.SetPixel(x, y, color);
            }
        }
        img.Unlock();
    }
#undef fnReadV

    fs.close();
}

void Canvas::LoadFromPNG(const std::string& fileName) {
    gui::Image img{fileName};
    imageSize = {img.GetWidth(), img.GetHeight()};

    globalIdCounter = 0;
    layerOrders.Clear();
    layers.Clear();
    layerOrders.PushBack(0);
    layers.PushBack(Layer{globalIdCounter++, gui::Image(imageSize.w, imageSize.h)});
    preview.Resize(img.GetWidth(), img.GetHeight());
    zoom = 1.0f;
    viewPosition = {0, 0};

    undoRedo.Reset();

    Reposition();

    auto& image = layers[0].image;
    img.Lock();
    image.Lock();
    for (int y = 0; y < img.GetHeight(); y++)
        for (int x = 0; x < img.GetWidth(); x++) {
            image.SetPixel(x, y, img.GetPixel(x, y));
        }
    image.Unlock();
    img.Unlock();

    palette.Clear();
    for (auto col : ExtractPalette()) {
        palette.PushBack(col);
    }

    Invalidate();
}

void Canvas::LoadEmpty() {
    imageSize = {64, 64};

    globalIdCounter = 0;
    layers.Clear();
    layerOrders.Clear();
    layers.PushBack(Layer{globalIdCounter++, gui::Image(imageSize.w, imageSize.h)});
    layerOrders.PushBack(0);
    preview.Resize(imageSize.w, imageSize.h);
    zoom = 1.0f;
    viewPosition = {0, 0};

    undoRedo.Reset();

    Reposition();

    auto& image = layers[0].image;
    image.Lock();
    for (int y = 0; y < image.GetHeight(); y++)
        for (int x = 0; x < image.GetWidth(); x++) {
            image.SetPixel(x, y, gui::Color{0, 0, 0, 0});
        }
    image.Unlock();

    Invalidate();
}

void Canvas::SaveToPNG(const std::string& fileName) {
    gui::Image img = gui::Image(imageSize.w, imageSize.h);
    gui::Graphics g = gui::Graphics::CreateGraphics(img);
    for (size_t i = layersOrdered.Size(); i-- > 0;) {
        auto* layer = layersOrdered[i];
        g.DrawImage(layer, 0, 0, imageSize.w, imageSize.h, gui::ImageFiltering::Nearest);
    }
    img.WriteToPNG(fileName);
}

void Canvas::SaveArtFile(const std::string& fileName) {
    std::ofstream fs(fileName, std::ios::binary);
    if (!fs.is_open())
        return;

    auto fnWrite = [&]<typename T>(const T& value) -> decltype(auto) {
        return fs.write(reinterpret_cast<const char*>(&value), sizeof(T));
    };

    // Image size
    fnWrite(imageSize.w);
    fnWrite(imageSize.h);

    // Color palette
    fnWrite(static_cast<uint>(palette.Size()));
    for (const auto& color : palette()) {
        fnWrite(color.r);
        fnWrite(color.g);
        fnWrite(color.b);
        fnWrite(color.a);
    }

    // Canvas specific
    fnWrite(globalIdCounter);
    fnWrite(zoom);
    fnWrite(viewPosition.x);
    fnWrite(viewPosition.y);

    fnWrite(colors[0]().r);
    fnWrite(colors[0]().g);
    fnWrite(colors[0]().b);
    fnWrite(colors[0]().a);
    fnWrite(colors[1]().r);
    fnWrite(colors[1]().g);
    fnWrite(colors[1]().b);
    fnWrite(colors[1]().a);

    // Layer ordering
    fnWrite(static_cast<uint>(layerOrders.Size()));
    for (const auto& order : layerOrders()) {
        fnWrite(order);
    }

    // Layers
    for (auto& layer : layers()) {
        // layer ID
        fnWrite(layer.id);

        // Pixels
        layer.image.Lock();
        for (int y = 0; y < imageSize.h; y++) {
            for (int x = 0; x < imageSize.w; x++) {
                auto color = layer.image.GetPixel(x, y);
                fnWrite(color.r);
                fnWrite(color.g);
                fnWrite(color.b);
                fnWrite(color.a);
            }
        }
        layer.image.Unlock();
    }

    fs.close();
}

void Canvas::NewLayer() {
    CmdNewLayer* cmd = new CmdNewLayer();
    cmd->id = globalIdCounter++;
    undoRedo.Run(cmd);
}

void Canvas::DeleteLayer(size_t i) {
    if (layers.Size() == 1)
        return;
    if (i >= layers.Size())
        return;

    CmdDeleteLayer* cmd = new CmdDeleteLayer();
    cmd->index = i;
    cmd->id = layers[layerOrders[i]].id;
    undoRedo.Run(cmd);
    Invalidate();
}

void Canvas::MoveLayerUp(size_t i) {
    if (i < 1)
        return;
    CmdMoveLayer* cmd = new CmdMoveLayer();
    cmd->fromLayer = i;
    cmd->toLayer = i - 1;
    undoRedo.Run(cmd);
}

void Canvas::MoveLayerDown(size_t i) {
    if (i >= layers.Size() - 1)
        return;
    CmdMoveLayer* cmd = new CmdMoveLayer();
    cmd->fromLayer = i;
    cmd->toLayer = i + 1;
    undoRedo.Run(cmd);
}

std::vector<gui::Color> Canvas::ExtractPalette(uint paletteSize, uint iterations) {
    auto& image = layers[0].image;
    image.Lock();
    std::vector<gui::Color> centers(paletteSize);
    for (int i = 0; i < paletteSize; ++i)
        centers[i] = image.GetPixel(rand() % image.GetWidth(), rand() % image.GetHeight());

    const uint pixelCount = image.GetWidth() * image.GetHeight();
    std::vector<int> assignment(pixelCount);

    for (uint iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < int(pixelCount); i++) {
            int x = i % image.GetWidth();
            int y = i / image.GetWidth();
            float best = 1e30f;
            for (uint k = 0; k < paletteSize; k++) {
                auto col = image.GetPixel(x, y);
                float dr = col.r - centers[k].r;
                float dg = col.g - centers[k].g;
                float db = col.b - centers[k].b;
                float da = col.a - centers[k].a;
                float d = dr * dr + dg * dg + db * db + da * da;
                if (d < best) {
                    best = d;
                    assignment[size_t(i)] = k;
                }
            }
        }

        std::vector<double> sumR(paletteSize, 0.0), sumG(paletteSize, 0.0), sumB(paletteSize, 0.0),
            sumA(paletteSize, 0.0);
        std::vector<int> count(paletteSize, 0);

        for (std::size_t i = 0; i < pixelCount; ++i) {
            int k = assignment[i];
            int x = i % image.GetWidth();
            int y = i / image.GetWidth();
            auto col = image.GetPixel(x, y);
            sumR[k] += col.r;
            sumG[k] += col.g;
            sumB[k] += col.b;
            sumA[k] += col.a;
            count[k]++;
        }

        for (int k = 0; k < paletteSize; ++k) {
            if (count[k] == 0)
                continue;
            centers[k] = {
                float(sumR[k] / count[k]),
                float(sumG[k] / count[k]),
                float(sumB[k] / count[k]),
                float(sumA[k] / count[k])
            };
        }
    }

    std::set<gui::Color> uniqueColors;
    for (int i = 0; i < paletteSize; ++i)
        uniqueColors.insert(centers[i]);

    auto fnClosestColor = [&uniqueColors](const gui::Color& col) -> gui::Color {
        float dist = 9999.0f;
        gui::Color selected{0, 0, 0, 0};
        for (const auto& palCol : uniqueColors) {
            float dr = col.r - palCol.r;
            float dg = col.g - palCol.g;
            float db = col.b - palCol.b;
            float da = col.a - palCol.a;
            float d = dr * dr + dg * dg + db * db + da * da;
            if (d < dist) {
                dist = d;
                selected = palCol;
            }
        }
        return selected;
    };

    for (int i = 0; i < int(pixelCount); i++) {
        int x = i % image.GetWidth();
        int y = i / image.GetWidth();
        auto col = fnClosestColor(image.GetPixel(x, y));
        image.SetPixel(x, y, col);
    }
    image.Unlock();

    return std::vector<gui::Color>(uniqueColors.begin(), uniqueColors.end());
}

gui::Image* Canvas::GetLayerImage(uint id) {
    for (auto& layer : layers.Get()) {
        if (layer.id == id)
            return &layer.image;
    }
    return nullptr;
}

void Canvas::RemoveLayerById(uint id) {
    auto& tmpLayers = layers.Get();
    auto it = std::find_if(tmpLayers.begin(), tmpLayers.end(), [&id](const Layer& layer) {
        return layer.id == id;
    });
    if (it == tmpLayers.end())
        return;

    auto layerIndex = static_cast<uint>(std::distance(tmpLayers.begin(), it));

    // Find the render-order slot that references this layer and remove it
    auto orders = layerOrders();
    auto orderIt = std::find(orders.begin(), orders.end(), layerIndex);
    if (orderIt != orders.end())
        orders.erase(orderIt);

    // Shift down all remaining indices that were above layerIndex
    for (auto& ord : orders)
        if (ord > layerIndex)
            ord--;

    layerOrders = orders;
    layers.EraseAt(layerIndex);
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

    std::vector<Pixel> GetEllipse(gui::PointI center, int rx, int ry, gui::Color color) {
        std::vector<Pixel> ret;

        auto plot4 = [&](int x, int y) {
            ret.push_back({{center.x + x, center.y + y}, color});
            if (x != 0)
                ret.push_back({{center.x - x, center.y + y}, color});
            if (y != 0)
                ret.push_back({{center.x + x, center.y - y}, color});
            if (x != 0 && y != 0)
                ret.push_back({{center.x - x, center.y - y}, color});
        };

        if (rx <= 0 && ry <= 0) {
            plot4(0, 0);
            return ret;
        }
        if (rx <= 0) {
            for (int y = -ry; y <= ry; y++)
                ret.push_back({{center.x, center.y + y}, color});
            return ret;
        }
        if (ry <= 0) {
            for (int x = -rx; x <= rx; x++)
                ret.push_back({{center.x + x, center.y}, color});
            return ret;
        }

        long rx2 = (long)rx * rx, ry2 = (long)ry * ry;
        long x = 0, y = ry;
        long dx = 0, dy = 2 * rx2 * y;
        long d = ry2 - rx2 * ry + (rx2 + 2) / 4;

        plot4((int)x, (int)y);
        while (dx < dy) {
            x++;
            dx += 2 * ry2;
            if (d < 0)
                d += ry2 + dx;
            else {
                y--;
                dy -= 2 * rx2;
                d += ry2 + dx - dy;
            }
            plot4((int)x, (int)y);
        }

        d = (long)std::round(
            (double)ry2 * (x + 0.5) * (x + 0.5) + (double)rx2 * (y - 1) * (y - 1) -
            (double)rx2 * ry2
        );
        y--;
        dy -= 2 * rx2;
        if (d > 0)
            d += rx2 - dy;
        else {
            x++;
            dx += 2 * ry2;
            d += rx2 - dy + dx;
        }
        while (y >= 0) {
            plot4((int)x, (int)y);
            y--;
            dy -= 2 * rx2;
            if (d > 0)
                d += rx2 - dy;
            else {
                x++;
                dx += 2 * ry2;
                d += rx2 - dy + dx;
            }
        }

        return ret;
    }
} // namespace algos

void EllipseTool::OnMouseDown(Canvas& canvas, int x, int y) {
    p0 = {x, y};
    canvas.Preview({{{x, y}, canvas.GetCurrentColor()}});
}

void EllipseTool::OnMouseDrag(Canvas& canvas, int x, int y) {
    canvas.Preview(BuildEllipse({x, y}, canvas.GetCurrentColor(), canvas.shiftPressed));
}

void EllipseTool::OnMouseUp(Canvas& canvas, int x, int y) {
    canvas.Draw(BuildEllipse({x, y}, canvas.GetCurrentColor(), canvas.shiftPressed));
    canvas.toolActive = false;
}

std::vector<algos::Pixel>
EllipseTool::BuildEllipse(gui::PointI p1, gui::Color col, bool fromCenter) {
    gui::PointI center;
    int rx, ry;
    if (fromCenter) {
        center = p0;
        int r = std::min(std::abs(p1.x - p0.x), std::abs(p1.y - p0.y));
        rx = ry = r;
    } else {
        center = {(p0.x + p1.x) / 2, (p0.y + p1.y) / 2};
        rx = std::abs(p1.x - p0.x) / 2;
        ry = std::abs(p1.y - p0.y) / 2;
    }
    return algos::GetEllipse(center, rx, ry, col);
}

void FillTool::OnMouseDown(Canvas& canvas, int x, int y) {
    canvas.Draw(algos::GetFloodFill(*canvas.currentImage(), {x, y}, canvas.GetCurrentColor()));
    canvas.toolActive = false;
}

void UndoRedo::Run(ICommand* command) {
    command->Execute(*m_canvas);
    m_undoStack.push(std::unique_ptr<ICommand>(command));
    while (!m_redoStack.empty())
        m_redoStack.pop();
}

void UndoRedo::Undo() {
    if (!CanUndo())
        return;
    auto cmd = std::move(m_undoStack.top());
    m_undoStack.pop();
    cmd->Undo(*m_canvas);
    m_redoStack.push(std::move(cmd));
}

void UndoRedo::Redo() {
    if (!CanRedo())
        return;
    auto cmd = std::move(m_redoStack.top());
    m_redoStack.pop();
    cmd->Execute(*m_canvas);
    m_undoStack.push(std::move(cmd));
}

void UndoRedo::Reset() {
    while (!m_undoStack.empty())
        m_undoStack.pop();
    while (!m_redoStack.empty())
        m_redoStack.pop();
}

bool UndoRedo::CanUndo() const {
    return !m_undoStack.empty();
}

bool UndoRedo::CanRedo() const {
    return !m_redoStack.empty();
}

void CmdDrawPixels::Execute(Canvas& canvas) {
    previousData.reserve(data.size());

    auto* target = canvas.GetLayerImage(targetId);
    target->Lock();
    for (auto px : data) {
        auto prev = target->GetPixel(px.position.x, px.position.y);
        previousData.push_back({px.position, prev});
    }
    for (auto px : data) {
        target->SetPixel(px.position.x, px.position.y, px.color);
    }
    target->Unlock();
}

void CmdDrawPixels::Undo(Canvas& canvas) {
    auto* target = canvas.GetLayerImage(targetId);
    target->Lock();
    for (auto px : previousData) {
        target->SetPixel(px.position.x, px.position.y, px.color);
    }
    target->Unlock();
    previousData.clear();
}

void PickerTool::OnMouseDown(Canvas& canvas, int x, int y) {
    auto* img = canvas.currentImage();
    img->Lock();
    if (!canvas.secondaryColor)
        canvas.colors[0] = img->GetPixel(x, y);
    else
        canvas.colors[1] = img->GetPixel(x, y);
    img->Unlock();
    canvas.toolActive = false;
}

void RectTool::OnMouseDown(Canvas& canvas, int x, int y) {
    p0 = {x, y};
    p1 = {x, y};
    canvas.Preview({{{x, y}, canvas.GetCurrentColor()}});
}

void RectTool::OnMouseDrag(Canvas& canvas, int x, int y) {
    p1 = {x, y};
    canvas.Preview(BuildRect(p1, canvas.GetCurrentColor(), canvas.shiftPressed));
}

void RectTool::OnMouseUp(Canvas& canvas, int x, int y) {
    p1 = {x, y};
    canvas.Draw(BuildRect(p1, canvas.GetCurrentColor(), canvas.shiftPressed));
    canvas.toolActive = false;
}

std::vector<algos::Pixel> RectTool::BuildRect(gui::PointI p1, gui::Color col, bool fromCenter) {
    std::vector<algos::Pixel> ret;

    gui::PointI tl, br;
    if (fromCenter) {
        int s = std::min(std::abs(p1.x - p0.x), std::abs(p1.y - p0.y));
        tl = {p0.x - s, p0.y - s};
        br = {p0.x + s, p0.y + s};
    } else {
        tl = {std::min(p0.x, p1.x), std::min(p0.y, p1.y)};
        br = {std::max(p0.x, p1.x), std::max(p0.y, p1.y)};
    }

    auto l1 = algos::GetLine({tl.x, tl.y}, {br.x, tl.y}, col);
    auto l2 = algos::GetLine({br.x, tl.y + 1}, {br.x, br.y}, col);
    auto l3 = algos::GetLine({br.x - 1, br.y}, {tl.x, br.y}, col);
    auto l4 = algos::GetLine({tl.x, br.y - 1}, {tl.x, tl.y + 1}, col);
    ret.insert(ret.end(), l1.begin(), l1.end());
    ret.insert(ret.end(), l2.begin(), l2.end());
    ret.insert(ret.end(), l3.begin(), l3.end());
    ret.insert(ret.end(), l4.begin(), l4.end());

    return ret;
}

void CmdMoveLayer::Execute(Canvas& canvas) {
    auto orders = canvas.layerOrders();
    auto tmp = orders[toLayer];
    orders[toLayer] = orders[fromLayer];
    orders[fromLayer] = tmp;
    canvas.layerOrders = orders;
}

void CmdMoveLayer::Undo(Canvas& canvas) {
    auto orders = canvas.layerOrders();
    auto tmp = orders[fromLayer];
    orders[fromLayer] = orders[toLayer];
    orders[toLayer] = tmp;
    canvas.layerOrders = orders;
}

void CmdDeleteLayer::Execute(Canvas& canvas) {
    auto* layer = canvas.GetLayerImage(id);

    data.clear();
    data.reserve(layer->GetWidth() * layer->GetHeight());

    // Save old image
    layer->Lock();
    for (uint y = 0; y < layer->GetHeight(); y++) {
        for (uint x = 0; x < layer->GetWidth(); x++) {
            data.push_back(layer->GetPixel(x, y));
        }
    }
    layer->Unlock();

    savedCurrentLayer = canvas.currentLayer();

    canvas.RemoveLayerById(id);
    canvas.currentLayer = canvas.currentLayer() >= canvas.layers.Size() ? canvas.layers.Size() - 1
                          : canvas.currentLayer() > 0                   ? canvas.currentLayer() - 1
                                                                        : 0;
}

void CmdDeleteLayer::Undo(Canvas& canvas) {
    canvas.layers.PushBack(Layer{id, gui::Image(canvas.imageSize.w, canvas.imageSize.h)});
    canvas.layerOrders.Insert(index, canvas.layers.Size() - 1);

    auto* layer = canvas.GetLayerImage(id);
    layer->Lock();
    for (uint y = 0; y < layer->GetHeight(); y++) {
        for (uint x = 0; x < layer->GetWidth(); x++) {
            auto pixel = data[x + y * layer->GetWidth()];
            layer->SetPixel(x, y, pixel);
        }
    }
    layer->Unlock();

    canvas.currentLayer = savedCurrentLayer;
}

void CmdNewLayer::Execute(Canvas& canvas) {
    Layer layer{};
    layer.id = id;
    layer.image = gui::Image(canvas.imageSize.w, canvas.imageSize.h);

    canvas.layers.PushBack(layer);
    canvas.layerOrders.PushBack(canvas.layers.Size() - 1);
    index = canvas.layers.Size() - 1;
}

void CmdNewLayer::Undo(Canvas& canvas) {
    canvas.RemoveLayerById(id);
    canvas.currentLayer = canvas.currentLayer() >= canvas.layers.Size() ? canvas.layers.Size() - 1
                          : canvas.currentLayer() > 0                   ? canvas.currentLayer() - 1
                                                                        : 0;
}
