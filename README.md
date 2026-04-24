# Elemental GUI

A C++20 desktop GUI framework combining declarative UI composition with immediate-mode 2D rendering. Built on SDL3 (windowing/input) and Cairo (graphics).

## Philosophy

Elemental GUI bridges the gap between retained-mode and immediate-mode UI paradigms:
- **Retained**: Hierarchical element tree with event subscriptions, focus management, and dirty tracking
- **Immediate**: Direct Cairo drawing in `OnDraw()` with no retained draw lists
- **Declarative**: Flutter-inspired builder API with lazy widget instantiation
- **Flexbox**: CSS-inspired layout with gap, padding, align, justify, flexGrow

## Features

| Feature | Description |
|---------|-------------|
| **Declarative API** | Flutter-inspired builder syntax with lazy instantiation (`WidgetDesc`) |
| **Flexbox Layout** | Row/Column containers with gap, padding, align, justify, flexGrow |
| **Event System** | Type-safe pub-sub for mouse, keyboard, scroll, focus, text events |
| **Multi-Window** | Parent/child relationships, modal dialogs, utility windows |
| **Custom Widgets** | Extend via `Element` or `Container` subclasses |
| **JSON Theming** | State-aware styles (normal/hover/click/checked) with gradients, shadows |
| **Immediate Graphics** | Cairo wrapper with shapes, paths, text, SVG, gradients, clipping |
| **Backend Abstraction** | Pluggable backends (SDL3 reference implementation included) |

## Quick Start

```bash
# Configure and build
cmake -B build -G Ninja
cmake --build build

# Run the demo
./build/tui_test
```

**Dependencies:**
- **System**: Cairo (must be installed)
- **Auto-fetched**: SDL3, nlohmann_json, nanosvg (via CPM.cmake)

## Example

```cpp
#include "tui/Application.h"
#include "tui/SDL3Backend.h"
#include "tui/Declarative.h"

using namespace tui;
namespace decl = tui::declarative;

class App : public ApplicationAdapter {
public:
    void OnCreate(Application& app) {
        auto* window = app.CreateWindow({
            .title = "Hello",
            .width = 400,
            .height = 300
        });

        auto ui = decl::Column({
            .gap = 16,
            .padding = EdgeInsets::All(32),
            .align = FlexAlign::Center
        }, {
            decl::Text("Hello, World!", { .align = Alignment::Center }),
            decl::Button("Click Me", {
                .onClick = []() { std::cout << "Clicked!\n"; }
            })
        });

        window->SetRoot(ui(*window));
        window->Show();
    }
    void OnDestroy() {}
};

int main() {
    tui::Application app;
    app.SetBackend(std::make_unique<tui::SDL3Backend>());
    return app.Start(new App());
}
```

## Available Widgets

| Category | Widgets |
|----------|---------|
| **Layout** | `Column`, `Row`, `ScrollView`, `SplitView`, `Panel` |
| **Input** | `Button`, `ToolButton`, `ToolRadioButton`, `ToolToggleButton`, `CheckBox`, `RadioButton`, `Switch`, `Slider`, `TextEdit` |
| **Display** | `Text`, `Image`, `ProgressBar`, `List`, `BasicList` |
| **Menus** | `Menu`, `MenuItem`, `MenuSeparator` |

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│  Declarative API (WidgetDesc)                             │
├─────────────────────────────────────────────────────────┤
│  Widgets (Button, Slider, Edit, etc.)                     │
├─────────────────────────────────────────────────────────┤
│  Container → Element (hierarchy)                          │
├─────────────────────────────────────────────────────────┤
│  FlexLayout (CSS-inspired flexbox)                        │
├─────────────────────────────────────────────────────────┤
│  Graphics (Cairo wrapper)                                 │
├─────────────────────────────────────────────────────────┤
│  EventSystem (pub-sub events)                             │
├─────────────────────────────────────────────────────────┤
│  Window / Application                                     │
├─────────────────────────────────────────────────────────┤
│  Backend (SDL3 - swappable)                               │
└─────────────────────────────────────────────────────────┘
```

## Custom Widgets

Inherit from `Element` (simple widgets) or `Container` (widgets with children):

```cpp
class MyWidget : public Element {
public:
    void OnDraw(Graphics& g) override {
        auto b = GetBounds();
        g.Rect(b.x, b.y, b.w, b.h);
        g.Color(0.2f, 0.4f, 0.8f);
        g.Fill();
    }

    EventStatus OnEvent(Event* e) override {
        if (e->Type() == EventType::MouseButton) {
            auto* me = static_cast<MouseEvent*>(e);
            if (me->pressed && GetIntersectedBounds().HasPoint(me->x, me->y)) {
                Invalidate();  // Mark for redraw
                return EventStatus::Consumed;
            }
        }
        return EventStatus::Active;
    }
};
```

Add to declarative API:
```cpp
struct MyWidgetProps {
    ElementProps base{};
    int value{0};
};

WidgetDesc MyWidget(const MyWidgetProps& props) {
    return [props](Window& window) -> Element* {
        auto& w = window.Create<MyWidget>();
        ElementSetup(w, props.base);
        return &w;
    };
}
```

## Graphics API

```cpp
// Shapes
g.Rect(x, y, w, h);
g.RoundRect(x, y, w, h, radius);
g.Arc(x, y, radius, startAngle, endAngle);
g.Line(x1, y1, x2, y2);

// Paths
g.BeginPath();
g.AddPathPoint(x, y);
g.EndPath(close);
g.Stroke();  // or g.Fill();

// Styling
g.Color(r, g, b, a);
g.Color(Color::FromHex("#FF5733"));
g.LineWidth(w);
g.SetLineCap(LineCap::Round);
g.SetLineJoin(LineJoin::Round);

// Transforms & Clipping
g.Save(); g.Restore();
g.Translate(x, y); g.Rotate(angle); g.Scale(sx, sy);
g.ClipPushRect(x, y, w, h); g.ClipPop();

// Text
g.Font("Sans", 14);
g.Text("Hello", x, y);
auto extents = g.MeasureText("Hello");

// Images
g.DrawImage(&img, x, y, w, h);

// Styled rendering (JSON from theme)
g.StyledRect(x, y, w, h, styleJson);
g.StyledPaint(paintJson);
```

## Project Structure

| Path | Description |
|------|-------------|
| `src/tui/*.h,*.cpp` | Core framework source |
| `src/tui/generated/` | Embedded resources (auto-generated) |
| `resources/*.json` | Theme definitions (edit these) |
| `cmake/` | Build configuration, CPM.cmake, resource embedding |
| `src/main.cpp` | Demo application |

## License

MIT
