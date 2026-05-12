# Elemental GUI

A C++20 desktop GUI framework (v0.5.2) combining declarative UI composition with
immediate-mode 2D rendering. Built on SDL3 (windowing/input) and Cairo
(graphics).

## Philosophy

Elemental GUI bridges the gap between retained-mode and immediate-mode UI
paradigms:

- **Retained**: Hierarchical element tree with event subscriptions, focus
  management, and dirty tracking
- **Immediate**: Direct Cairo drawing in `OnDraw()` with no retained draw lists
- **Declarative**: Flutter-inspired builder API with lazy widget instantiation
- **Flexbox**: CSS-inspired layout with gap, padding, align, justify, flexGrow

## Features

| Feature                 | Description                                                                    |
| ----------------------- | ------------------------------------------------------------------------------ |
| **Declarative API**     | Flutter-inspired builder syntax with lazy instantiation (`WidgetDesc`)         |
| **Flexbox Layout**      | Row/Column containers with gap, padding, align, justify, flexGrow              |
| **Event System**        | Type-safe pub-sub for mouse, keyboard, scroll, focus, text events              |
| **Multi-Window**        | Parent/child relationships, modal dialogs, utility windows                     |
| **Custom Widgets**      | Extend via `Element` or `Container` subclasses                                 |
| **JSON Theming**        | State-aware styles (normal/hover/click/checked) with variables and inheritance |
| **Immediate Graphics**  | Cairo wrapper with shapes, paths, text, SVG, gradients, clipping               |
| **Backend Abstraction** | Pluggable backends (SDL3 reference implementation included)                    |

## Quick Start

```bash
# Configure and build
cmake -B build -G Ninja
cmake --build build

# Run the widget showcase
./build/examples/elements/elements
```

**Dependencies:**

- **System**: Cairo (must be installed)
- **Auto-fetched**: SDL3, nlohmann_json v3.12.0, nanosvg (via CPM.cmake)

## Example

```cpp
#include "Application.h"
#include "Declarative.h"
#include "backends/sdl3/SDL3Backend.h"

using namespace gui;
namespace decl = gui::declarative;

class App : public Window {
public:
    App() : Window(WindowConfig{.title = "Hello", .width = 400, .height = 300}) {}

    WidgetDesc OnBuild() override {
        Show();
        return decl::Column({
            .gap = 16,
            .padding = EdgeInsets::All(32),
            .align = FlexAlign::Center
        }, {
            decl::Text("Hello, World!", {.align = Alignment::MiddleCenter}),
            decl::Button("Click Me", {
                .onClick = []() { printf("Clicked!\n"); }
            }),
            decl::Slider({
                .range = {0.0f, 100.0f},
                .value = 50.0f,
                .onValueChange = [](float v) { printf("%.1f\n", v); }
            })
        });
    }
};

int main() {
    Application app{new SDL3Backend()};
    app.CreateWindow<App>();
    return app.Start();
}
```

## Available Widgets

| Category    | Widgets                                                                                                                             |
| ----------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| **Layout**  | `Column`, `Row`, `ScrollView`, `SplitView`, `Panel`, `Spacer`                                                                       |
| **Input**   | `Button`, `ToolButton`, `ToolRadioButton`, `ToolToggleButton`, `CheckBox`, `RadioButton`, `Switch`, `Slider`, `Spinner`, `TextEdit` |
| **Display** | `Text`, `Image`, `ProgressBar`, `ColorPicker`, `List<T>`, `BasicList`                                                               |
| **Menus**   | `Menu`, `MenuItem`, `MenuSeparator`                                                                                                 |
| **Custom**  | `Custom<T, Props>()` for user-defined elements                                                                                      |

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│  Declarative API (WidgetDesc / builder functions)         │
├─────────────────────────────────────────────────────────┤
│  Widgets (Button, Slider, TextArea, Spinner, etc.)        │
├─────────────────────────────────────────────────────────┤
│  Container → Element (hierarchy, event propagation)       │
├─────────────────────────────────────────────────────────┤
│  FlexLayout (CSS-inspired flexbox)                        │
├─────────────────────────────────────────────────────────┤
│  Graphics (Cairo wrapper — shapes, SVG, text, gradients)  │
├─────────────────────────────────────────────────────────┤
│  EventSystem (type-safe pub-sub)                          │
├─────────────────────────────────────────────────────────┤
│  Window / Application (multi-window, theme, clipboard)    │
├─────────────────────────────────────────────────────────┤
│  Backend interface (SDL3 reference — swappable)           │
└─────────────────────────────────────────────────────────┘
```

## Custom Widgets

Inherit from `Element` (simple widgets) or `Container` (widgets with children):

```cpp
class MyWidget : public Element {
public:
    std::string StyleKey() const override { return "MyWidget"; }

    void OnDraw(Graphics& g) override {
        auto sz = GetSize();
        g.StyledRect(0, 0, sz.w, sz.h, GetStyle()["normal"]);
    }

    EventStatus OnEvent(Event* e) override {
        if (e->Type() == EventType::MouseButton) {
            auto* me = static_cast<MouseEvent*>(e);
            if (me->pressed && GetIntersectedBounds().HasPoint(me->x, me->y)) {
                Invalidate();
                return EventStatus::Consumed;
            }
        }
        return EventStatus::Active;
    }
};
```

Add to the declarative API:

```cpp
struct MyWidgetProps : Copiable<MyWidgetProps> {
    opt<ElementProps> base;
    opt<int> value;
    Fields(base, value)
};

WidgetDesc MyWidget(const MyWidgetProps& props) {
    return [props](Window& window) -> Element* {
        auto& w = window.Create<MyWidget>();
        if (props.base) ElementSetup(w, *props.base);
        w.SetValue(props.value.value_or(0));
        return &w;
    };
}
```

## Graphics API

```cpp
// Shapes & paths
g.Rect(x, y, w, h);
g.RoundRect(x, y, w, h, radius);
g.Arc(x, y, radius, startAngle, endAngle);
g.Line(x1, y1, x2, y2);
g.BeginPath(); g.AddPathPoint(x, y); g.EndPath(close);
g.Stroke();  // or g.Fill();

// Styling
g.Color(r, g, b, a);
g.LineWidth(w);
g.SetLineCap(LineCap::Round);
g.SetLineJoin(LineJoin::Round);

// Transforms & clipping
g.Save(); g.Restore();
g.Translate(x, y); g.Rotate(angle); g.Scale(sx, sy);
g.ClipPushRect(x, y, w, h); g.ClipPop();

// Text
g.Font(FontStyle::Bold, "Sans", 14);
g.MeasureText("Hello");  // TextExtents
g.GetFontExtents();      // FontExtents

// Images & SVG
g.DrawImage(img, x, y, w, h);
g.DrawSVG(svgStyleJson, x, y, w, h);
g.DrawShadow(elevation, x, y, w, h, radius);

// JSON-driven styled rendering (reads from theme)
g.StyledRect(x, y, w, h, styleJson);
g.StyledTextBegin(styleJson);
g.StyledTextEnd(text, x, y);
```

## Theming

Styles are JSON files with widget keys, state variants, and variable references:

```json
{
    "$primary": "#FF3859A6",
    "Button": {
        "padding": { "horizontal": 16, "vertical": 8 },
        "normal": { "background": { "color": "$primary" }, "elevation": 2 },
        "hover": { "background": { "color": "$primaryHover" }, "elevation": 4 },
        "click": { "background": { "color": "$primaryClick" }, "elevation": 1 }
    }
}
```

Load a custom theme at runtime:

```cpp
app.LoadTheme("path/to/LightStyle.json");  // load from file
app.ResetStyle();                           // revert to default dark theme
```

A `LightStyle.json` is included in `examples/assets/`.

## Project Structure

| Path                              | Description                                           |
| --------------------------------- | ----------------------------------------------------- |
| `src/*.h,*.cpp`                   | Core framework + all widget implementations           |
| `src/backends/sdl3/`              | SDL3 backend implementation                           |
| `src/generated/`                  | Embedded resources (auto-generated at configure time) |
| `resources/DefaultStyle.json`     | Default dark theme (edit this, not `generated/`)      |
| `examples/elements/`              | Comprehensive widget showcase                         |
| `examples/drawing-pad/`           | Custom element (infinite canvas)                      |
| `examples/pixel-paint/`           | Pixel art painting application                        |
| `examples/custom-backend/`        | Implementing a third-party backend                    |
| `examples/assets/LightStyle.json` | Light theme                                           |
| `cmake/`                          | Build helpers, CPM.cmake, resource embedding          |

## License

MIT
