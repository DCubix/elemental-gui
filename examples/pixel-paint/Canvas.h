#pragma once

#include <array>
#include <memory>
#include <stack>

#include <elemental/Declarative.h>
#include <elemental/Element.h>
#include <elemental/Graphics.h>

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

class Canvas;
struct Tool {
    virtual void OnMouseDown(Canvas& canvas, int x, int y) = 0;
    virtual void OnMouseMove(Canvas& canvas, int x, int y) {}
    virtual void OnMouseDrag(Canvas& canvas, int x, int y) {}
    virtual void OnMouseUp(Canvas& canvas, int x, int y) {}
};

struct Layer {
    uint id;

    Layer(uint id, int width, int height);

    void Present(const std::vector<gui::Color>& palette);
    uint8_t GetPixel(int x, int y);
    void SetPixel(int x, int y, uint8_t value);

    gui::Image& GetImage() { return image; }
    std::vector<uint8_t>& GetIndexed() { return imageIndexed; }

    int GetWidth() const { return image.GetWidth(); }
    int GetHeight() const { return image.GetHeight(); }

    bool operator==(const Layer& o) const { return id == o.id && image == o.image; }

private:
    gui::Image image;
    std::vector<uint8_t> imageIndexed;
};

namespace algos {
    struct Pixel {
        gui::PointI position{0, 0};
        uint8_t color{0};
    };

    std::vector<Pixel> GetLine(gui::PointI a, gui::PointI b, uint8_t color);
    std::vector<Pixel> GetQuadBezier(gui::PointI a, gui::PointI b, gui::PointI c, uint8_t color);
    std::vector<Pixel> GetFloodFill(Layer& layer, gui::PointI p, uint8_t color);
    std::vector<Pixel> GetEllipse(gui::PointI center, int rx, int ry, uint8_t color);
} // namespace algos

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

    std::vector<gui::Color> ExtractPalette(
        gui::Image& image,
        std::vector<uint8_t>& outIndexed,
        uint paletteSize = 32,
        uint iterations = 10
    );
    uint8_t GetCurrentColor() const { return secondaryColor ? colors[1]() : colors[0](); }
    gui::Color GetCurrentActualColor() const { return palette()[GetCurrentColor()]; }

    Layer* GetLayer(uint id);
    void RemoveLayerById(uint id);

    gui::Size imageSize;
    gui::Image preview;
    gui::Property<int> currentLayerIndex{0};
    gui::Property<std::vector<Layer>> layers;
    gui::Property<std::vector<uint>> layerOrders;
    gui::Property<std::vector<gui::Color>> palette;

    gui::Computed<std::vector<Layer*>> layersOrdered = gui::Computed<std::vector<Layer*>>{
        [this]() {
            std::vector<Layer*> ret;
            ret.reserve(layers.Size());
            for (auto index : layerOrders()) {
                ret.push_back(&layers[index]);
            }
            return ret;
        },
        layers,
        layerOrders
    };
    gui::Computed<Layer*> currentLayer = gui::Computed<Layer*>{
        [this]() -> Layer* {
            int idx = currentLayerIndex();
            if (idx < 0 || static_cast<size_t>(idx) >= layersOrdered.Size())
                return nullptr;
            return layersOrdered[idx];
        },
        currentLayerIndex,
        layers,
        layerOrders
    };
    gui::Computed<uint> currentLayerId = gui::Computed<uint>{
        [this]() -> uint {
            int idx = currentLayerIndex();
            if (layers.Size() == 0 || idx < 0 || static_cast<size_t>(idx) >= layerOrders.Size())
                return 0;
            return layers[layerOrders[idx]].id;
        },
        currentLayerIndex,
        layers,
        layerOrders
    };

    ToolType selectedTool{ToolType::Pencil};
    std::array<std::unique_ptr<Tool>, 7> tools;

    bool dragging{false}, toolActive{false}, shiftPressed{false}, secondaryColor{false};
    gui::Property<float> zoom{1.0f};
    gui::PointI viewPosition{0, 0}, prevMouse{0, 0};
    gui::Property<uint8_t> colors[2] = {0, 1};

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
    PencilTool(uint8_t colorOverride)
        : colorOverride(colorOverride) {}

    void OnMouseDown(Canvas& canvas, int x, int y) override;
    void OnMouseDrag(Canvas& canvas, int x, int y) override;
    void OnMouseUp(Canvas& canvas, int x, int y) override;

    gui::PointI prev{0, 0};
    std::vector<algos::Pixel> buffer;
    std::optional<uint8_t> colorOverride{std::nullopt};
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

    std::vector<algos::Pixel> BuildRect(gui::PointI p1, uint8_t col, bool fromCenter = false);

    gui::PointI p0{0, 0}, p1{0, 0};
};

struct EllipseTool : public Tool {
    void OnMouseDown(Canvas& canvas, int x, int y) override;
    void OnMouseDrag(Canvas& canvas, int x, int y) override;
    void OnMouseUp(Canvas& canvas, int x, int y) override;

    std::vector<algos::Pixel> BuildEllipse(gui::PointI p1, uint8_t col, bool fromCenter);

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
    std::vector<uint8_t> data;
};

struct CmdNewLayer : public ICommand {
    void Execute(Canvas& canvas) override;
    void Undo(Canvas& canvas) override;

    uint id;
    uint index;
};