#pragma once

#include <array>
#include <memory>

#include <Declarative.h>
#include <Element.h>
#include <Graphics.h>

namespace dc = gui::declarative;

class Canvas;
struct Tool {
    virtual void OnMouseDown(Canvas& canvas, int x, int y) = 0;
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

    gui::Size GetPreferredSize() const override;

    void Reposition();

    gui::Image image;
    ToolType selectedTool{ToolType::Pencil};
    std::array<std::unique_ptr<Tool>, 7> tools;

    bool dragging{false};
    float zoom{1.0f};
    gui::PointI viewPosition{0, 0}, prevMouse{0, 0};
};

struct CanvasProps {
    dc::opt<dc::ElementProps> base;
};

struct PencilTool : public Tool {
    void OnMouseDown(Canvas& canvas, int x, int y);
    void OnMouseDrag(Canvas& canvas, int x, int y);

    void DrawLineDDA(Canvas& canvas, const gui::PointI& a, const gui::PointI& b, gui::Color col);

    gui::PointI prev{0, 0};
};