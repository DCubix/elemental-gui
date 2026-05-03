# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## Core Philosophy

**Elemental GUI** (v0.5.1) is a C++20 desktop GUI framework that bridges
immediate-mode and retained-mode UI paradigms. It provides:

- **Hierarchical element tree** with parent-child relationships
- **Event-driven architecture** with pub-sub event dispatch
- **Declarative API** inspired by Flutter (lazy widget instantiation via
  `WidgetDesc`)
- **Immediate-mode graphics** via Cairo (direct drawing in `OnDraw`)
- **Flexbox layout** with CSS-inspired alignment properties
- **JSON-based theming** with state-aware styles (normal/hover/click/checked)
- **Backend abstraction** for platform independence (SDL3 reference
  implementation)
- **Property system** with reactive data binding via `Property<T>::Bind()`

## Build Commands

```bash
# Configure (first time or after CMakeLists changes)
cmake -B build -G Ninja

# Build library and examples
cmake --build build

# Run an example
./build/examples/elements/elements
```

**Dependencies:** SDL3, Cairo (system), nlohmann_json v3.12.0, nanosvg. All
except Cairo are auto-fetched via CPM.cmake.

**Build options:**
- `ELEMENTAL_BUILD_EXAMPLES` (default: ON) — build example apps
- `ELEMENTAL_BACKEND_SDL3` (default: ON) — enable SDL3 backend

**Resource Embedding:** Files in `resources/` are embedded as C++ raw-string
headers into `src/generated/` at configure time via `embed_resources.cmake`.
Edit the `.json` source, never the generated headers.

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

### Widget Class Hierarchy

```
Subscriber (event interface)
  └─ Element (base UI node — bounds, style, dirty tracking)
      ├─ Label (text + optional icon display)
      │   ├─ Button (clickable, state machine)
      │   └─ ToolButton (toggle / radio / normal modes)
      ├─ LineEdit (single-line text editor, no background)
      │   └─ Edit (LineEdit + styled background)
      │       └─ TextArea (multiline, per-char formatting)
      ├─ CheckBox (checkbox + label)
      ├─ RadioButton (radio + label, group mutual exclusion)
      ├─ Switch (toggle switch, 3 states: off/on/indeterminate)
      ├─ Slider (draggable range, horizontal or vertical)
      ├─ Spinner (numeric +/- buttons with decimal support)
      ├─ ProgressBar (progress indicator, indeterminate mode)
      ├─ Scrollbar (internal — used by ScrollView and List)
      ├─ ImageView (image display with scaling modes)
      ├─ Menu (popup context menu, hierarchical submenus)
      ├─ MenuItem (menu item with icon, checkbox, submenu)
      ├─ ScrollView (single-child scrollable container)
      └─ Container (element with managed children)
          ├─ Panel (flex layout container with optional background)
          ├─ SplitView (two-pane resizable divider)
          └─ List<T> (typed item list with selection, lazy scrollbar)
```

### Execution Flow

```
main()
  → Application app{new SDL3Backend()}
  → app.CreateWindow<MyWindow>()
    → MyWindow::OnBuild()           // user builds declarative UI tree
      → returns WidgetDesc          // lazy builder function
  → app.Start()
    → Application loads DefaultStyle.json (resolves $vars, inheritance)
    → Backend::Init()
    → For each window: OnBuild() → SetRoot(widgetDesc)
    → Main loop (while m_running):
      → Backend::PollEvents()               // SDL3 event pumping
        → EventSystem::Broadcast<event>()  // pub-sub dispatch
          → Subscriber::OnEvent()          // element tree receives events
            → Virtual OnMouseDown/OnKey/etc.
      → Remove closed windows
      → For each dirty window:
        → Window::Update()                   // incremental redraw
          → Graphics::Clear()
          → Element::OnDraw(Graphics&)       // recursive draw calls
            → Cairo operations
          → Backend::PresentFrame()          // upload to display
```

### Key Design Patterns

**1. Lazy Widget Instantiation (Declarative API)**

```cpp
// WidgetDesc is std::function<Element*(Window&)>
// The tree is NOT built until SetRoot() calls the function
auto desc = decl::Column({...}, { decl::Button(...) });
window.SetRoot(desc);  // ← instantiation happens here
```

**2. Window as UI Entry Point**

The preferred pattern is to subclass `Window` and override `OnBuild()`:

```cpp
class MyWindow : public Window {
public:
    MyWindow() : Window(WindowConfig{.title="App", .width=800, .height=600}) {}

    WidgetDesc OnBuild() override {
        Show();
        return decl::Column({.gap=16, .padding=EdgeInsets::All(32)}, {
            decl::Button("OK", {.onClick = []() { /* ... */ }})
        });
    }
};

int main() {
    Application app{new SDL3Backend()};
    app.CreateWindow<MyWindow>();
    return app.Start();
}
```

**3. Event Propagation**

- Events flow: `Backend` → `Application` → `Window` → `Element::OnEvent()`
- Two-phase handling: raw `OnEvent()` → typed virtuals (`OnMouseDown`,
  `OnKeyDown`)
- Return `EventStatus::Consumed` to stop propagation, `Active` to continue
- Motion events always reach all subscribers (for hover state clearing)

**4. Dirty Region Tracking**

- `Invalidate()` marks element + ancestors as dirty
- Only dirty windows redraw each frame (not per-element clipping)
- Call `Invalidate()` whenever visual state changes (position, size, color, etc.)

**5. Resource Management**

- Elements owned by `Window` (unique_ptr storage)
- Parent-child relationships are non-owning (raw pointers)
- `Window::Create<T>()` factory ensures proper event hookup

**6. Layout System**

- `FlexLayout` computes bounds during `Apply()` based on container size
- Children report preferred sizes via `GetPreferredSize()`
- `flexGrow` distributes extra space among siblings
- Layout happens before draw, after any size change

### Core Classes Reference

| Class         | Responsibility                                  | Key Methods                                      |
| ------------- | ----------------------------------------------- | ------------------------------------------------ |
| `Application` | App lifecycle, window management, event loop    | `Start()`, `CreateWindow<W>()`, `LoadTheme()`    |
| `Window`      | Element ownership, rendering, focus management  | `OnBuild()`, `Create<T>()`, `FindByTag<T>()`, `ShowPopup()` |
| `Element`     | Base UI node — bounds, visibility, focus, style | `OnDraw()`, `OnEvent()`, `Invalidate()`, `GetIntersectedBounds()` |
| `Container`   | Element with children — event propagation       | `Add()`, `Remove()`, `GetChildren()`             |
| `EventSystem` | Pub-sub event bus — type-safe broadcast         | `Subscribe()`, `Broadcast<Event>()`              |
| `Subscriber`  | Event receiver interface                        | `OnEvent()`, enabled/priority flags              |
| `Graphics`    | Cairo wrapper — 2D rendering API                | `StyledRect()`, `DrawSVG()`, `ClipPush/Pop()`    |
| `FlexLayout`  | Flexbox layout engine                           | `Apply()`, direction/justify/align properties    |
| `Backend`     | Platform abstraction interface                  | window create/destroy, event pump, present frame |
| `Image`       | Raster/SVG image loading                        | `IsValid()`, used by `ImageView`, `ToolButton`   |
| `Label`       | Text + optional icon display                    | `SetText()`, `SetIcon()`, base for Button        |
| `Panel`       | Flex container with pluggable layout            | `SetLayout()`, `GetLayout<L>()`                  |
| `Scrollbar`   | Interactive scrollbar (internal)                | lazy-created by `List<T>` and `ScrollView`       |

## Application API

```cpp
class Application {
    Application(Backend* backend);

    // Window management
    Window* CreateWindow(const WindowConfig& config);
    template<typename W> W* CreateWindow();

    // Lifecycle
    int Start();

    // Theming
    void LoadTheme(const std::string& path);           // load .json file
    void LoadThemeFromString(const std::string& json); // load from string
    void ResetStyle();                                 // revert to DefaultStyle.json

    // Clipboard
    std::string GetClipboard();
    void SetClipboard(const std::string& text);

    // Access
    Backend& GetBackend();
    EventSystem& GetEventSystem();
    Json& GetStyle();
};
```

## Window API

```cpp
struct WindowConfig {
    std::string title;
    uint32_t width{800};
    uint32_t height{600};
    bool resizable{false};
    WindowStyle style{WindowStyle::TopLevel};
    Window* parent{nullptr};
};

enum class WindowStyle { TopLevel, Modal, Utility, Borderless, Fullscreen };

class Window {
    // UI construction
    virtual WidgetDesc OnBuild();           // override to define UI
    void SetRoot(Element* root);
    Element& GetRoot();

    // Element factory & lookup
    template<typename E> E& Create();
    template<typename E> E* FindByTag(const std::string& tag);
    template<typename E> E* Find(std::function<bool(E*)> predicate = nullptr);

    // Visibility & lifecycle
    void Show();
    void Hide();
    void Close();

    // Focus & text input
    void Focus(Element* e);
    void StartInput();
    void StopInput();

    // Overlays / popups
    void ShowPopup(Element* popup);
    void DismissPopup(Element* popup);

    // Properties
    std::string GetTitle(); void SetTitle(const std::string&);
    Size GetSize();
    Application* GetApp();
    WindowId GetId();
};
```

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
        std::string StyleKey() const override { return "MyWidget"; }
        void OnDraw(Graphics& g) override;
        EventStatus OnEvent(Event* e) override;
        void SetValue(int v) { m_value = v; Invalidate(); }

    private:
        int m_value{0};
    };
}
```

**Step 2: Implement Drawing & Event Handling**

```cpp
void MyWidget::OnDraw(Graphics& g) {
    auto sz = GetSize();
    g.StyledRect(0, 0, sz.w, sz.h, GetStyle()["normal"]);
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
// In Declarative.h
struct MyWidgetProps {
    ElementProps base{};
    int initialValue{0};
    ValueChanged<int> onValueChanged;
};
WidgetDesc MyWidget(const MyWidgetProps& props);

// In Declarative.cpp
WidgetDesc MyWidget(const MyWidgetProps& props) {
    return [props](Window& window) -> Element* {
        auto& w = window.Create<gui::MyWidget>();
        ElementSetup(w, props.base);
        w.SetValue(props.initialValue);
        return &w;
    };
}
```

**Step 4: Add Styling (optional)** Add entries to `resources/DefaultStyle.json`
under a key matching `StyleKey()`.

## Styling System

Styles are JSON objects keyed by widget class name. The framework supports:

- **State-aware styles**: `normal`, `hover`, `click`, `checked`, `selected`,
  `focused`, `indeterminate`, plus ON-state variants (`normalOn`, `hoverOn`,
  `clickOn`)
- **Visual properties**: background (solid/gradient), border
  (radius/color/width), elevation (shadow), padding
- **Typography**: font, fontSize, fontWeight, fontSlant, color
- **SVG icons**: embedded vector graphics for checkmarks, arrows, dots
- **Variable references**: `"$primaryColor"` resolves to a named color variable
- **Style inheritance**: `"inherits": "BaseStyleKey"` merges parent style

Style application flow:

1. `Application` loads `DefaultStyle.json` at startup; resolves `$variables` and
   `inherits` chains
2. Widgets query `GetStyle()` which returns their style block (keyed by
   `StyleKey()`) merged with any per-element `m_style`
3. `Graphics::StyledRect()` and `StyledTextBegin()` consume style JSON directly

**StyleKey conventions:**

- `Label`, `LineEdit` → `"DefaultText"` (text-only styling, no padding)
- `Edit`, `TextArea` → `"Edit"` (styled text field)
- `Panel` → `"Panel"` (elevation, border, background)
- All other widgets → their own class name key in `DefaultStyle.json`
- Elements with no style → leave `StyleKey()` returning `""` (default)

**Padding from style:** Widgets must **not** cache padding as a member. Read it
on demand:

```cpp
EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
```

`EdgeInsets::FromStyle` accepts:
- Single number: `10` → `All(10)`
- Array [2]: `[h, v]` → `Symmetric(h, v)`
- Array [4]: `[l, t, r, b]` → `Only(l, t, r, b)`
- Object: `{"horizontal": 10, "vertical": 20}` or `{"left": 10, "top": 20, ...}`

**Alternate theme:** `examples/assets/LightStyle.json` is a full light-mode
theme. Load at runtime with `app.LoadTheme("path/to/LightStyle.json")` or
revert with `app.ResetStyle()`.

### Style Property Types

```
color        — hex string "#AARRGGBB" or array [r,g,b,a]
elevation    — float shadow depth
border       — {radius: float, color: ..., width: float}
background   — {color: ...} or {linearGradient: {start, end, stops, colors}}
padding      — number | [h,v] | [l,t,r,b] | {horizontal, vertical}
font/fontSize/fontWeight/fontSlant — typography
svg/stroke/lineCap/lineJoin — inline SVG paths
```

## Event System

The event system uses a **pub-sub pattern** with type-safe broadcast:

```cpp
// All event types
enum class EventType { MouseButton, MouseMotion, Key, TextInput, Focus, Blur, Scroll };

// Event structures
struct MouseEvent  { int x, y; MouseButton button; bool pressed; };
struct MotionEvent { int x, y; MouseButton button; };
struct KeyEvent    { Key key; ModifierState mod; bool pressed; };
struct TextInputEvent { char inputChar; };
struct FocusEvent  { Subscriber* element; };
struct BlurEvent   { Subscriber* element; };
struct ScrollEvent { float scrollX, scrollY; int mouseX, mouseY; };

struct ModifierState { bool shift, control, alt, meta; };

enum class MouseButton { Unknown, Left, Right, Middle, X1, X2 };
```

The `Key` enum has 200+ values covering letters, digits, numpad, function keys
(F1–F24), arrows, navigation (Home/End/PageUp/PageDown), modifiers
(Shift/Ctrl/Alt/Meta), and media/browser keys.

```cpp
// Subscribe in element constructor
EventSystem::Subscribe(EventType::MouseButton, this);

// Broadcast from anywhere with type-safe arguments
EventSystem::Broadcast<MouseEvent>(x, y, button, pressed);
```

**Subscriber priority:** `SetHighPriority(true)` puts a subscriber at the front
of the queue. Motion events are never consumed — all subscribers always receive
them (required for hover state clearing).

**Event Handling Strategy:**

- Elements inherit `Subscriber` and implement `OnEvent()`
- Two-phase dispatch: `OnEvent()` → typed virtual (`OnMouseDown`, `OnKeyDown`)
- Coordinates in screen space; use `GetIntersectedBounds()` for hit testing
  (clips by ancestor bounds); `GetBounds()` for unclipped global coords
- Always check `GetIntersectedBounds().HasPoint(x, y)` before consuming mouse
  events

## Graphics API

```cpp
// Basic shapes
g.Clear(r, g, b, a);
g.Rect(x, y, w, h);
g.RoundRect(x, y, w, h, radius);
g.Arc(x, y, radius, startAngle, endAngle);
g.Line(x1, y1, x2, y2);

// Rendering
g.Color(r, g, b, a);
g.Fill(preserve=false);
g.Stroke(preserve=false);
g.LineWidth(w);
g.SetLineCap(LineCap::Butt|Round|Square);
g.SetLineJoin(LineJoin::Miter|Round|Bevel);

// Paths
g.BeginPath();
g.AddPathPoint(x, y);
g.EndPath(close=false);

// Images & SVG
g.DrawImage(img, x, y, w, h);
g.DrawSVG(svgStyleJson, x, y, w, h);
g.DrawShadow(elevation, x, y, w, h, radius);

// Styled rendering (JSON-driven, reads theme style blocks)
g.StyledRect(x, y, w, h, styleJson);
g.StyledPaint(paintJson);
g.StyledTextBegin(styleJson);
g.StyledTextEnd(text, x, y, rotation=0.0f);

// Text
g.Font(FontStyle::Normal|Bold|Italic|BoldItalic, family, size);
g.MeasureText(text);   // → TextExtents {size, xBearing, yBearing, xAdvance}
g.GetFontExtents();    // → FontExtents {ascent, descent, height}
g.DrawChar(c, x, y);  // returns x advance

// Clipping (stack-based)
g.ClipPushRect(x, y, w, h);
g.ClipPushRoundRect(x, y, w, h, radius);
g.ClipPushPath(pathFunc);
g.ClipPop();

// Transforms
g.Save(); g.Restore();
g.Translate(tx, ty); g.Rotate(angle); g.Scale(sx, sy);

// Escape hatch
cairo_t* ctx = g.GetCairoContext();
```

**Color struct** (used internally; most APIs accept float r/g/b/a directly):

```cpp
Color c = Color::FromHex("#FF3859A6");  // #RGB, #RRGGBB, #AARRGGBB
Color c = Color::FromRGBA(r, g, b, a);
Color c = Color::FromHSLA(h, s, l, a);
Color c = c.Darken(amount); // or Lighten(amount)
Color::FromStyle(jsonValue); // accepts array or hex string
```

**JSON paint formats:**

```json
{"color": "#RRGGBB"}
{"linearGradient": {"start": [x,y], "end": [x,y], "colors": [...], "stops": [...]}}
{"radialGradient": {"center": [x,y], "radius": r, "colors": [...], "stops": [...]}}
```

## Layout

```cpp
enum class FlexDirection { Row, Column };
enum class FlexJustify  { Start, Center, End, SpaceBetween, SpaceEvenly };
enum class FlexAlign    { Start, Center, End, Stretch };

// FlexLayout applies inside Panel/Column/Row containers
layout.SetDirection(FlexDirection::Column);
layout.SetJustifyContent(FlexJustify::SpaceBetween);
layout.SetAlignItems(FlexAlign::Center);
layout.SetPadding(EdgeInsets::All(16));
layout.SetGap(8);

// EdgeInsets factory methods
EdgeInsets::All(v)
EdgeInsets::Symmetric(horizontal, vertical)
EdgeInsets::Only(left, top, right, bottom)
EdgeInsets::FromStyle(json)     // parses number / [h,v] / [l,t,r,b] / object
EdgeInsets::Zero
```

Elements with `flexGrow > 0` share free space proportionally. Invisible
elements are skipped in layout calculations.

## Declarative API

The declarative API uses **builder functions** that return `WidgetDesc` (a
`std::function<Element*(Window&)>`). Widgets are instantiated lazily when
`SetRoot()` calls the function.

**Type aliases:**

```cpp
using WidgetDesc = std::function<Element*(Window&)>;
using MenuDesc   = std::function<gui::Menu*(Window&)>;
using VoidCallback        = std::function<void()>;
template<typename T>
using ValueChanged        = std::function<void(const T&)>;
```

**ElementProps** (first member of every Props struct):

```cpp
struct ElementProps {
    std::string tag{""};      // for FindByTag<T>()
    bool enabled{true};
    bool autoSize{false};
    float flexGrow{0.0f};
    Rectangle bounds{0,0,0,0};
    Json style;               // per-element style override
};
```

**Available Widgets and Props:**

```
Layout:
  Column(ColumnProps, children)   — gap, padding, align, justify, showBackground
  Row(RowProps, children)         — gap, padding, align, justify, showBackground
  ScrollView(child, ScrollViewProps) — scrollDirection: Direction
  SplitView(SplitViewProps)       — direction, splitPosition (px), first, second
  Spacer()                        — flexible empty space

Input:
  Button(text, ButtonProps)       — icon, onClick
  ToolButton(text, ToolButtonProps)      — icon, iconSize, toggled, onClick, group
  ToolRadioButton(text, ToolButtonProps) — radio behavior within group
  ToolToggleButton(text, ToolButtonProps)— persistent toggle behavior
  CheckBox(text, CheckBoxProps)   — checked, onChanged(bool)
  RadioButton(text, RadioButtonProps) — group (string), checked, onChanged(bool)
  Switch(SwitchProps)             — checked, onChanged(bool)
  Slider(SliderProps)             — direction, range, value, step, onValueChange(float)
  Spinner(SpinnerProps)           — range, value, step, decimals, onValueChange(float)
  TextEdit(TextEditProps)         — text, onChanged, multiLine, masked

Display:
  Text(text, TextProps)           — align (Alignment enum), icon
  Image(ImageProps)               — image (Image*), scaling (Stretch/Contain/Cover)
  ProgressBar(ProgressBarProps)   — range, value, indeterminate, direction
  BasicList(BasicListProps)       — items(vector<string>), selectedIndex, onSelectionChanged(int)
  List<T>(ListProps<T>)           — items(vector<ListItem<T>>), selectedIndex, onSelectionChanged(int)

Menus:
  Menu(MenuProps, items)          — onDismiss
  MenuItem(MenuItemProps)         — text, icon, checked, onClick, subMenu(MenuDesc)
  MenuSeparator()

Custom:
  Custom<Elem, Props>(props, setup)  — template for arbitrary elements
```

**Supporting types:**

```cpp
struct Range { float minimum, maximum;
    float Normalized(float v); float Remap(Range other, float v); float Constrain(float v); };

template<typename T>
struct ListItem { T value; std::string label; };

enum class Direction       { Horizontal, Vertical };
enum class Alignment       { TopLeft, TopCenter, TopRight, MiddleLeft, MiddleCenter,
                             MiddleRight, BottomLeft, BottomCenter, BottomRight };
enum class ImageScalingMode { Stretch, Contain, Cover };
```

### Text Editing Widgets

The text editing hierarchy is built in three layers:

| Class      | Base       | Description                                                                                                                                                                                                                                                                                                                                                                                                                |
| ---------- | ---------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `LineEdit` | `Element`  | Single-line text editor with no background. Handles caret, selection, keyboard shortcuts (Home/End/Left/Right/Backspace/Delete/Ctrl+C/X/V/A), horizontal scroll offset, optional masking. `StyleKey()` = `"DefaultText"`. Padding is read on demand via `EdgeInsets::FromStyle(GetStyle()["padding"])` (resolves to zero for `DefaultText`).                                                                               |
| `Edit`     | `LineEdit` | Adds a styled background drawn from `GetStyle()[state]` where state is `"normal"`, `"hover"`, or `"focused"`. Handles `OnMouseEnter`/`OnMouseLeave` for hover highlight and `OnFocus`/`OnBlur` to trigger a style refresh. `StyleKey()` = `"Edit"`. Padding comes from `GetStyle()["padding"]` (auto-resolved from the `Edit` JSON block).                                                                                 |
| `TextArea` | `Edit`     | Multiline editor with per-character formatting. Adds `m_lines` (`vector<text::Line>`) as the single source of truth for both geometry and per-character style. Overrides `InsertChar` (allows `'\n'`), `RemoveChar`, `DeleteSelected`, `Rebuild` (calls `text::ComputeLines`, then resets all chars to the default style color). Adds Up/Down/Enter line-aware key handling and `Format(from, len, style, r, g, b)` / `Format(style, r, g, b)` (directly sets `color`/`style` on `text::Char` objects in `m_lines`; no rebuild needed). |

**Key rules:**

- Always call `Rebuild()` after mutating `m_textRaw` to keep `m_lines` in sync.
  `Rebuild()` resets all char styles to the default color, so re-apply
  `Format()` calls after each rebuild if per-character styling must be
  preserved.
- `Format()` modifies `text::Char::color` and `text::Char::style` directly in
  `m_lines` and calls `Invalidate()` — no rebuild required.
- In `TextArea::Rebuild` and `Format`, `globalIdx = lineOffset + ch.index` maps
  a visible char to its position in `m_textRaw`. `lineOffset` advances by
  `line.chars.size()` (visible chars + sentinel), which accounts for the `'\n'`
  separator.

## Backend Interface

Implement `Backend` to add a new windowing system:

```cpp
class Backend {
    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual void PollEvents(BackendEventSink& sink) = 0;
    // Window lifecycle
    virtual WindowHandle CreateWindow(const WindowConfig&, WindowHandle parent) = 0;
    virtual void DestroyWindow(WindowHandle) = 0;
    virtual void ShowWindow(WindowHandle) = 0;
    virtual void HideWindow(WindowHandle) = 0;
    virtual void ResizeWindow(WindowHandle, uint32_t w, uint32_t h) = 0;
    virtual void SetWindowTitle(WindowHandle, const std::string&) = 0;
    // Rendering: Cairo ARGB32 pixel blit
    virtual void CreateRenderBuffer(WindowHandle, uint32_t w, uint32_t h) = 0;
    virtual void PresentFrame(WindowHandle, unsigned char* data, int stride,
                              uint32_t w, uint32_t h) = 0;
    // Text input (IME)
    virtual void StartTextInput(WindowHandle) = 0;
    virtual void StopTextInput(WindowHandle) = 0;
    // Clipboard
    virtual void SetClipboardText(const std::string&) = 0;
    virtual std::string GetClipboardText() const = 0;
};
```

The SDL3 reference implementation is in `src/backends/sdl3/SDL3Backend.h/.cpp`.
See `examples/custom-backend/main.cpp` for a third-party backend example.

## Project Structure

```
elemental-gui/
├── src/
│   ├── Application.h/cpp     — app lifecycle, theme loading, clipboard
│   ├── Window.h/cpp          — element ownership, focus, popups, OnBuild
│   ├── Element.h             — base UI node
│   ├── Container.h           — children management
│   ├── Declarative.h/cpp     — builder functions and Props structs
│   ├── EventSystem.h/cpp     — pub-sub event bus
│   ├── Backend.h             — backend interface
│   ├── Graphics.h/cpp        — Cairo 2D rendering wrapper
│   ├── FlexLayout.h/cpp      — flexbox layout engine
│   ├── Layout.h/cpp          — EdgeInsets, Layout base class
│   ├── Utils.h               — VoidCallback, ValueChanged, Range, enums
│   ├── Button.h/cpp          — clickable button
│   ├── Label.h               — text + icon display (base for Button)
│   ├── LineEdit.h/cpp        — single-line text editor
│   ├── Edit.h/cpp            — styled LineEdit
│   ├── TextArea.h/cpp        — multiline text editor
│   ├── CheckBox.h/cpp        — checkbox with label
│   ├── RadioButton.h/cpp     — radio button with group
│   ├── Switch.h/cpp          — toggle switch
│   ├── Slider.h/cpp          — draggable range
│   ├── Spinner.h/cpp         — numeric +/- input
│   ├── ProgressBar.h/cpp     — progress indicator
│   ├── Scrollbar.h/cpp       — scrollbar (internal)
│   ├── ScrollView.h/cpp      — scrollable single-child container
│   ├── SplitView.h/cpp       — two-pane resizable container
│   ├── Panel.h/cpp           — flex layout container
│   ├── List.h                — typed item list (template)
│   ├── Menu.h/cpp            — context menu
│   ├── MenuItem.h/cpp        — menu item
│   ├── ImageView.h/cpp       — image display
│   ├── Image.h/cpp           — image loading (raster + SVG)
│   ├── ToolButton.h/cpp      — icon button (normal/toggle/radio)
│   ├── WindowConfig.h        — WindowConfig struct and WindowStyle enum
│   ├── backends/sdl3/        — SDL3 backend implementation
│   └── generated/            — embedded resource headers (auto-generated)
├── resources/
│   └── DefaultStyle.json     — dark theme (edit this, not generated/)
├── examples/
│   ├── elements/main.cpp     — comprehensive widget showcase
│   ├── drawing-pad/main.cpp  — custom element (infinite canvas) example
│   ├── custom-backend/main.cpp — third-party backend implementation
│   └── assets/LightStyle.json — light theme alternative
├── cmake/                    — build helpers, CPM.cmake, embed_resources.cmake
└── CMakeLists.txt
```

## Approach

- Read existing files before writing. Don't re-read unless changed.
- Thorough in reasoning, concise in output.
- Skip files over 100KB unless required.
- No sycophantic openers or closing fluff.
- No emojis or em-dashes.
- Do not guess APIs, versions, flags, commit SHAs, or package names. Verify by
  reading code or docs before asserting.
