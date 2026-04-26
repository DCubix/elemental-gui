# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## Core Philosophy

**Elemental GUI** is a C++20 desktop GUI framework that bridges immediate-mode
and retained-mode UI paradigms. It provides:

- **Hierarchical element tree** with parent-child relationships
- **Event-driven architecture** with pub-sub event dispatch
- **Declarative API** inspired by Flutter (lazy widget instantiation via
  `WidgetDesc`)
- **Immediate-mode graphics** via Cairo (direct drawing in `OnDraw`)
- **Flexbox layout** with CSS-inspired alignment properties
- **JSON-based theming** with state-aware styles (normal/hover/click/checked)
- **Backend abstraction** for platform independence (SDL3 reference
  implementation)

## Build Commands

```bash
# Configure (first time or after CMakeLists changes)
cmake -B build -G Ninja

# Build library and test executable
cmake --build build

# Run the demo app
./build/tui_test
```

**Dependencies:** SDL3, Cairo (system), nlohmann_json, nanosvg. All except Cairo
are auto-fetched via CPM.cmake.

**Resource Embedding:** Files in `resources/` are embedded as C++ raw-string
headers into `src/generated/` at configure time. Edit the `.json` source, never
the generated headers.

## Architecture Overview

The framework follows a layered architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────┐
│  Declarative API (WidgetDesc)                             │
│  Builder functions → lazy instantiation                 │
├─────────────────────────────────────────────────────────┤
│  Widget Layer (Button, Slider, Edit, etc.)              │
│  OnDraw() → Graphics API | OnEvent() → Event handling   │
├─────────────────────────────────────────────────────────┤
│  Container (Element with children)                        │
│  Child management | Event propagation | Layout           │
├─────────────────────────────────────────────────────────┤
│  Element (Base class)                                     │
│  Bounds | Focus | Visibility | Dirty tracking | Style    │
├─────────────────────────────────────────────────────────┤
│  Layout Engine (FlexLayout)                             │
│  Flex direction | Justify | Align | Gap | Padding        │
├─────────────────────────────────────────────────────────┤
│  Graphics (Cairo wrapper)                                 │
│  Shapes | Paths | Text | Images | SVG | Gradients         │
├─────────────────────────────────────────────────────────┤
│  Event System (Pub-Sub)                                 │
│  Mouse | Keyboard | Scroll | Focus | Custom events       │
├─────────────────────────────────────────────────────────┤
│  Window / Application                                     │
│  Multi-window | Event loop | Dirty region redraw          │
├─────────────────────────────────────────────────────────┤
│  Backend (SDL3 implementation)                          │
│  Platform abstraction | Input | Rendering surface         │
└─────────────────────────────────────────────────────────┘
```

### Execution Flow

```
Application::Start(adapter)
  → ApplicationAdapter::OnCreate(app)      // user constructs UI
    → app.CreateWindow(config)             // creates platform window
      → window.SetRoot(widgetDesc)         // lazy instantiation via WidgetDesc
        → WidgetDesc resolves to Element*  // builder function called
          → element tree materialized
  → Main loop (while m_running):
    → Backend::PollEvents()                // SDL3 event pumping
      → EventSystem::Broadcast<event>()   // pub-sub dispatch
        → Subscriber::OnEvent()            // element tree receives events
          → Virtual OnMouseDown/OnKey/etc.  // widget-specific handling
    → For each dirty window:
      → Window::Redraw()                    // incremental redraw
        → Graphics::Clear()                 // clear background
        → Element::OnDraw(Graphics&)        // recursive draw calls
          → Cairo operations                // shapes, text, images
      → Backend::PresentFrame()             // upload to GPU/display
```

### Key Design Patterns

**1. Lazy Widget Instantiation (Declarative API)**

```cpp
// WidgetDesc is std::function<Element*(Window&)>
// The tree is NOT built until SetRoot() calls the function
auto desc = decl::Column({...}, { decl::Button(...) });
window.SetRoot(desc);  // ← instantiation happens here
```

**2. Event Propagation**

- Events flow: `Backend` → `Application` → `Window` → `Element::OnEvent()`
- Two-phase handling: raw `OnEvent()` → typed virtuals (`OnMouseDown`,
  `OnKeyDown`)
- Return `EventStatus::Consumed` to stop propagation, `Active` to continue
- Motion events always reach all subscribers (for hover state clearing)

**3. Dirty Region Tracking**

- `Invalidate()` marks element + ancestors as dirty
- Only dirty windows redraw each frame (not per-element clipping)
- Call `Invalidate()` whenever visual state changes (position, size, color,
  etc.)

**4. Resource Management**

- Elements owned by `Window` (unique_ptr storage)
- Parent-child relationships are non-owning (raw pointers)
- `Window::Create<T>()` factory ensures proper event hookup

**5. Layout System**

- `FlexLayout` computes bounds during `Apply()` based on container size
- Children report preferred sizes via `GetPreferredSize()`
- `flexGrow` distributes extra space among siblings
- Layout happens before draw, after any size change

### Core Classes Reference

| Class         | Responsibility                                  | Key Methods                                      |
| ------------- | ----------------------------------------------- | ------------------------------------------------ |
| `Application` | App lifecycle, window management, event loop    | `Start()`, `CreateWindow()`, event dispatch      |
| `Window`      | Element ownership, rendering, focus management  | `SetRoot()`, `Create<T>()`, `FindByTag<T>()`     |
| `Element`     | Base UI node — bounds, visibility, focus, style | `OnDraw()`, `OnEvent()`, `Invalidate()`          |
| `Container`   | Element with children — event propagation       | `Add()`, `Remove()`, `GetPreferredSize()`        |
| `EventSystem` | Pub-sub event bus — type-safe broadcast         | `Subscribe()`, `Broadcast<Event>()`              |
| `Subscriber`  | Event receiver interface                        | `OnEvent()`, enabled/priority flags              |
| `Graphics`    | Cairo wrapper — 2D rendering API                | `Rect()`, `Text()`, `Color()`, `ClipPush/Pop()`  |
| `FlexLayout`  | Flexbox layout engine                           | `Apply()`, direction/justify/align properties    |
| `Backend`     | Platform abstraction interface                  | Window/create/destroy, event pump, present       |
| `Image`       | Raster/SVG image loading                        | `Image(file)`, used by `ImageView`, `ToolButton` |

### Adding a New Widget

**Step 1: Create the Widget Class**

```cpp
// MyWidget.h
#pragma once
#include "Element.h"
#include "Graphics.h"

namespace gui {
    class MyWidget : public Element {  // or Container for widgets with children
    public:
        void OnDraw(Graphics& g) override;
        EventStatus OnEvent(Event* e) override;

        // Custom properties
        void SetValue(int v) { m_value = v; Invalidate(); }

    private:
        int m_value{0};
    };
}
```

**Step 2: Implement Drawing & Event Handling**

```cpp
// MyWidget.cpp
#include "MyWidget.h"

void MyWidget::OnDraw(Graphics& g) {
    auto b = GetBounds();
    g.Rect(b.x, b.y, b.w, b.h);
    g.Color(0.2f, 0.4f, 0.8f);
    g.Fill();
}

EventStatus MyWidget::OnEvent(Event* e) {
    if (e->Type() == EventType::MouseButton) {
        auto* me = static_cast<MouseEvent*>(e);
        if (me->pressed && GetIntersectedBounds().HasPoint(me->x, me->y)) {
            SetValue(m_value + 1);
            return EventStatus::Consumed;
        }
    }
    return EventStatus::Active;
}
```

**Step 3: Add Declarative Builder**

```cpp
// In Declarative.h - add props struct and function declaration
struct MyWidgetProps {
    ElementProps base{};
    int initialValue{0};
    ValueChanged<int> onValueChanged;
};
WidgetDesc MyWidget(const MyWidgetProps& props);

// In Declarative.cpp - implement builder
WidgetDesc MyWidget(const MyWidgetProps& props) {
    return [props](Window& window) -> Element* {
        auto& w = window.Create<gui::MyWidget>();
        ElementSetup(w, props.base);
        w.SetValue(props.initialValue);
        // ... wire up callbacks
        return &w;
    };
}
```

**Step 4: Add Styling (optional)** Add entries to `resources/DefaultStyle.json`
under a key matching your widget class name.

**Step 5: Override `StyleKey()`** so the framework auto-assigns your widget's
JSON block when it is created:

```cpp
// MyWidget.h
std::string StyleKey() const override { return "MyWidget"; }
```

### Styling System

Styles are JSON objects keyed by widget class name. The framework supports:

- **State-aware styles**: `normal`, `hover`, `click`, `checked`, `selected`
- **Visual properties**: background (solid/gradient), border
  (radius/color/width), elevation (shadow), padding
- **Typography**: font, fontSize, fontWeight, fontSlant, color
- **SVG icons**: embedded vector graphics for checkmarks, arrows

Style application flow:

1. `Application` loads `DefaultStyle.json` at startup
2. Widgets query `GetStyle()` which returns their default style fetched from the
   root style using `StyleKey()` merged with `m_style` at the top level
3. `Graphics::StyledRect()` and `StyledTextBegin()` consume style JSON directly

**StyleKey conventions:**

- `Label`, `LineEdit` → `"DefaultText"` (text-only styling)
- `Edit`, `TextArea` → `"Edit"` (styled text field)
- All other widgets → their own class name key in `DefaultStyle.json`
- Elements with no style → leave `StyleKey()` returning `""` (default)

**Padding from style:** Widgets must **not** cache padding as a member. Read it
on demand:

```cpp
EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
```

`EdgeInsets::FromStyle` returns `EdgeInsets::Zero` when the key is absent, so
`LineEdit` (no padding in `DefaultText`) naturally gets `{0,0,0,0}`.

### Event System

The event system uses a **pub-sub pattern** with type-safe broadcast:

```cpp
// Event types: MouseButton, MouseMotion, Key, TextInput, Focus, Blur, Scroll
// Define custom events by inheriting from Event:
struct MyEvent : public Event {
    EventType Type() const override { return EventType::Custom; }
    int data;
};

// Subscribe in element constructor
EventSystem::Subscribe(EventType::MouseButton, this);

// Broadcast from anywhere with type-safe arguments
EventSystem::Broadcast<MouseEvent>(x, y, button, pressed);
```

**Event Handling Strategy:**

- Elements inherit `Subscriber` and implement `OnEvent()`
- Two-phase dispatch: `OnEvent()` → typed virtual (`OnMouseDown`, `OnKeyDown`)
- Coordinates in screen space; use `GetBounds()` / `GetIntersectedBounds()` for
  hit testing
- Always check `GetIntersectedBounds().HasPoint(x, y)` before consuming mouse
  events

### Declarative API

The declarative API uses **builder functions** that return `WidgetDesc` (a
`std::function<Element*(Window&)>`). Widgets are instantiated lazily when
`SetRoot()` calls the function.

```cpp
using namespace gui::declarative;

auto ui = Column({
    .gap = 16,
    .padding = EdgeInsets::All(32),
    .align = FlexAlign::Center
}, {
    Text("Hello, World!", { .align = Alignment::Center }),
    Row({ .gap = 8 }, {
        Button("OK", { .onClick = []() { /* ... */ } }),
        Button("Cancel", { .onClick = []() { /* ... */ } })
    }),
    Slider({ .range = {0.0f, 100.0f}, .value = 50.0f })
});

window.SetRoot(ui(window));  // ← instantiation happens here
```

**Available Widgets:**

- **Layout**: `Column` (vertical flex), `Row` (horizontal flex), `ScrollView`,
  `SplitView`
- **Input**: `Button`, `ToolButton`, `ToolRadioButton`, `ToolToggleButton`,
  `CheckBox`, `RadioButton`, `Switch`, `Slider`, `TextEdit`
- **Display**: `Text`, `Image`, `ProgressBar`, `List`, `BasicList`
- **Containers**: `Panel`, `Menu`, `MenuItem`, `MenuSeparator`
- **Custom**: `Custom<T, Props>()` for user-defined elements

### Text Editing Widgets

The text editing hierarchy is built in three layers:

| Class      | Base       | Description                                                                                                                                                                                                                                                                                                                                                                                                                |
| ---------- | ---------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `LineEdit` | `Element`  | Single-line text editor with no background. Handles caret, selection, keyboard shortcuts (Home/End/Left/Right/Backspace/Delete/Ctrl+C/X/V/A), horizontal scroll offset, optional masking. `StyleKey()` = `"DefaultText"`. Padding is read on demand via `EdgeInsets::FromStyle(GetStyle()["padding"])` (resolves to zero for `DefaultText`).                                                                               |
| `Edit`     | `LineEdit` | Adds a styled background drawn from `GetStyle()[state]` where state is `"normal"`, `"hover"`, or `"focused"`. Handles `OnMouseEnter`/`OnMouseLeave` for hover highlight and `OnFocus`/`OnBlur` to trigger a style refresh. `StyleKey()` = `"Edit"`. Padding comes from `GetStyle()["padding"]` (auto-resolved from the `Edit` JSON block).                                                                                 |
| `TextArea` | `Edit`     | Multiline editor with per-character formatting. Adds `m_lines` (`vector<text::Line>`) as the single source of truth for both geometry and per-character style. Overrides `InsertChar` (allows `'\n'`), `RemoveChar`, `DeleteSelected`, `Rebuild` (calls `text::ComputeLines`, then resets all chars to the default style color). Adds Up/Down/Enter line-aware key handling and `Format(from, len, style, r, g, b)` / `Format(style, r, g, b)` (directly sets `color`/`style` on `text::Char` objects in `m_lines`; no rebuild needed). |

**Key rules:**

- Always call `Rebuild()` after mutating `m_textRaw` to keep `m_lines` in sync. `Rebuild()` resets all char styles to the default color, so re-apply `Format()` calls after each rebuild if per-character styling must be preserved.
- `Format()` modifies `text::Char::color` and `text::Char::style` directly in `m_lines` and calls `Invalidate()` — no rebuild required.
- In `TextArea::Rebuild` and `Format`, `globalIdx = lineOffset + ch.index` maps a visible char to its position in `m_textRaw`. `lineOffset` advances by `line.chars.size()` (visible chars + sentinel), which accounts for the `'\n'` separator.

**Props Pattern:** Every widget has a `Props` struct with `ElementProps base` as
the first member:

```cpp
struct ButtonProps {
    ElementProps base{};   // tag, bounds, flexGrow, enabled, style
    gui::Image* icon{nullptr};
    VoidCallback onClick;
};
```

## Approach

- Read existing files before writing. Don't re-read unless changed.
- Thorough in reasoning, concise in output.
- Skip files over 100KB unless required.
- No sycophantic openers or closing fluff.
- No emojis or em-dashes.
- Do not guess APIs, versions, flags, commit SHAs, or package names. Verify by
  reading code or docs before asserting.
