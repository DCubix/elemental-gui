#pragma once

#include <array>
#include <memory>

#include <Declarative.h>
#include <Element.h>
#include <Graphics.h>

namespace dc = gui::declarative;

namespace algos {
    struct Pixel {
        gui::PointI position{0, 0};
        gui::Color color{0, 0, 0, 0};
    };

    std::vector<Pixel> GetLine(gui::PointI a, gui::PointI b, gui::Color color);
    std::vector<Pixel> GetQuadBezier(gui::PointI a, gui::PointI b, gui::PointI c, gui::Color color);
    std::vector<Pixel> GetFloodFill(gui::Image& image, gui::PointI p, gui::Color color);
} // namespace algos

class Canvas;
struct Tool {
    virtual void OnMouseDown(Canvas& canvas, int x, int y) = 0;
    virtual void OnMouseMove(Canvas& canvas, int x, int y) {}
    virtual void OnMouseDrag(Canvas& canvas, int x, int y) {}
    virtual void OnMouseUp(Canvas& canvas, int x, int y) {}
};

class Canvas : public gui::Element {
public:
    enum class ToolType { Pencil = 0, Eraser, Eyedrop, Curve, Square, Circle, Fill };

    Canvas();

    void OnDraw(gui::Graphics& g) override;
    void OnMouseDown(gui::MouseEvent e) override;
    void OnMouseMove(gui::MotionEvent e) override;
    void OnMouseUp(gui::MouseEvent e) override;
    void OnScroll(gui::ScrollEvent e) override;
    void OnKeyDown(gui::KeyEvent e) override;
    void OnKeyUp(gui::KeyEvent e) override;

    gui::Size GetPreferredSize() const override;

    void Reposition();
    void Preview(const std::vector<algos::Pixel>& data);
    void Draw(const std::vector<algos::Pixel>& data);

    gui::Image image, preview;
    ToolType selectedTool{ToolType::Pencil};
    std::array<std::unique_ptr<Tool>, 7> tools;

    bool dragging{false}, toolActive{false}, shiftPressed{false}, secondaryColor{false};
    float zoom{1.0f};
    gui::PointI viewPosition{0, 0}, prevMouse{0, 0};
    gui::Color colors[2] = {gui::Color::FromHex("#000"), gui::Color::FromHex("#FFF")};

    gui::Color GetCurrentColor() const { return secondaryColor ? colors[1] : colors[0]; }
};

struct CanvasProps {
    dc::opt<dc::ElementProps> base;
};

struct PencilTool : public Tool {
    PencilTool() = default;
    PencilTool(const gui::Color& colorOverride)
        : colorOverride(colorOverride) {}

    void OnMouseDown(Canvas& canvas, int x, int y) override;
    void OnMouseDrag(Canvas& canvas, int x, int y) override;
    void OnMouseUp(Canvas& canvas, int x, int y) override;

    gui::PointI prev{0, 0};
    std::vector<algos::Pixel> buffer;
    std::optional<gui::Color> colorOverride{std::nullopt};
};

struct CurveTool : public Tool {
    void OnMouseDown(Canvas& canvas, int x, int y) override;
    void OnMouseMove(Canvas& canvas, int x, int y) override;
    void OnMouseDrag(Canvas& canvas, int x, int y) override;
    void OnMouseUp(Canvas& canvas, int x, int y) override;

    gui::PointI pts[3];
    int clickCount{0};
};

struct FillTool : public Tool {
    void OnMouseDown(Canvas& canvas, int x, int y) override;
};