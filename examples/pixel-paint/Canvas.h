#pragma once

#include <array>
#include <memory>
#include <stack>

#include <Declarative.h>
#include <Element.h>
#include <Graphics.h>

namespace dc = gui::declarative;

class Canvas;
struct ICommand {
    virtual void Execute(Canvas& canvas) = 0;
    virtual void Undo(Canvas& canvas) = 0;
};

class UndoRedo {
public:
    UndoRedo() = default;
    UndoRedo(Canvas* canvas)
        : m_canvas(canvas) {}

    void Run(ICommand* command);

    void Undo();
    void Redo();

    void Reset();

    bool CanUndo() const;
    bool CanRedo() const;

private:
    Canvas* m_canvas{nullptr};
    std::stack<std::unique_ptr<ICommand>> m_undoStack, m_redoStack;
};

namespace algos {
    struct Pixel {
        gui::PointI position{0, 0};
        gui::Color color{0, 0, 0, 0};
    };

    std::vector<Pixel> GetLine(gui::PointI a, gui::PointI b, gui::Color color);
    std::vector<Pixel> GetQuadBezier(gui::PointI a, gui::PointI b, gui::PointI c, gui::Color color);
    std::vector<Pixel> GetFloodFill(gui::Image& image, gui::PointI p, gui::Color color);
    std::vector<Pixel> GetEllipse(gui::PointI center, int rx, int ry, gui::Color color);
} // namespace algos

class Canvas;
struct Tool {
    virtual void OnMouseDown(Canvas& canvas, int x, int y) = 0;
    virtual void OnMouseMove(Canvas& canvas, int x, int y) {}
    virtual void OnMouseDrag(Canvas& canvas, int x, int y) {}
    virtual void OnMouseUp(Canvas& canvas, int x, int y) {}
};

struct Layer {
    uint id;
    gui::Image image;
    bool operator==(const Layer& o) const { return id == o.id && image == o.image; }
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

    void Undo();
    void Redo();

    void LoadArtFile(const std::string& fileName);
    void LoadFromPNG(const std::string& fileName);
    void LoadEmpty();

    void SaveToPNG(const std::string& fileName);
    void SaveArtFile(const std::string& fileName);

    void NewLayer();
    void DeleteLayer(size_t i);
    void MoveLayerUp(size_t i);
    void MoveLayerDown(size_t i);

    std::vector<gui::Color> ExtractPalette(uint paletteSize = 32, uint iterations = 10);
    gui::Color GetCurrentColor() const { return secondaryColor ? colors[1]() : colors[0](); }

    gui::Image* GetLayerImage(uint id);
    void RemoveLayerById(uint id);

    gui::Size imageSize;
    gui::Image preview;
    gui::Property<int> currentLayer{0};
    gui::Property<std::vector<Layer>> layers;
    gui::Property<std::vector<uint>> layerOrders;
    gui::Property<std::vector<gui::Color>> palette;

    gui::Computed<std::vector<gui::Image*>> layersOrdered = gui::Computed<std::vector<gui::Image*>>{
        [this]() {
            std::vector<gui::Image*> ret;
            ret.reserve(layers.Size());
            for (auto index : layerOrders()) {
                ret.push_back(&layers[index].image);
            }
            return ret;
        },
        layers,
        layerOrders
    };
    gui::Computed<gui::Image*> currentImage = gui::Computed<gui::Image*>{
        [this]() -> gui::Image* {
            int idx = currentLayer();
            if (idx < 0 || static_cast<size_t>(idx) >= layersOrdered.Size())
                return nullptr;
            return layersOrdered[idx];
        },
        currentLayer,
        layers,
        layerOrders
    };
    gui::Computed<uint> currentLayerId = gui::Computed<uint>{
        [this]() -> uint {
            int idx = currentLayer();
            if (layers.Size() == 0 || idx < 0 || static_cast<size_t>(idx) >= layerOrders.Size())
                return 0;
            return layers[layerOrders[idx]].id;
        },
        currentLayer,
        layers,
        layerOrders
    };

    ToolType selectedTool{ToolType::Pencil};
    std::array<std::unique_ptr<Tool>, 7> tools;

    bool dragging{false}, toolActive{false}, shiftPressed{false}, secondaryColor{false};
    float zoom{1.0f};
    gui::PointI viewPosition{0, 0}, prevMouse{0, 0};
    gui::Property<gui::Color> colors[2] = {
        gui::Color::FromHex("#000"),
        gui::Color::FromHex("#FFF")
    };

    UndoRedo undoRedo;
    VoidCallback onImageChanged;

    uint globalIdCounter{0};
};

struct CanvasProps {
    dc::opt<dc::ElementProps> base;
    dc::opt<VoidCallback> onImageChanged;
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

struct PickerTool : public Tool {
    void OnMouseDown(Canvas& canvas, int x, int y) override;
};

struct RectTool : public Tool {
    void OnMouseDown(Canvas& canvas, int x, int y) override;
    void OnMouseDrag(Canvas& canvas, int x, int y) override;
    void OnMouseUp(Canvas& canvas, int x, int y) override;

    std::vector<algos::Pixel> BuildRect(gui::PointI p1, gui::Color col, bool fromCenter = false);

    gui::PointI p0{0, 0}, p1{0, 0};
};

struct EllipseTool : public Tool {
    void OnMouseDown(Canvas& canvas, int x, int y) override;
    void OnMouseDrag(Canvas& canvas, int x, int y) override;
    void OnMouseUp(Canvas& canvas, int x, int y) override;

    std::vector<algos::Pixel> BuildEllipse(gui::PointI p1, gui::Color col, bool fromCenter);

    gui::PointI p0{0, 0};
};

// --- Undo/Redo Commands ------------------------------------------
struct CmdDrawPixels : public ICommand {
    void Execute(Canvas& canvas) override;
    void Undo(Canvas& canvas) override;

    uint targetId;
    std::vector<algos::Pixel> data, previousData;
};

struct CmdMoveLayer : public ICommand {
    void Execute(Canvas& canvas) override;
    void Undo(Canvas& canvas) override;

    uint toLayer, fromLayer;
};

struct CmdDeleteLayer : public ICommand {
    void Execute(Canvas& canvas) override;
    void Undo(Canvas& canvas) override;

    uint id, index;
    uint savedCurrentLayer;
    std::vector<gui::Color> data;
};

struct CmdNewLayer : public ICommand {
    void Execute(Canvas& canvas) override;
    void Undo(Canvas& canvas) override;

    uint id;
    uint index;
};