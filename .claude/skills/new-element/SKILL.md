---
name: new-element
description: Bootstrap a new UI widget/element for the elemental-gui C++ framework. Creates the .h/.cpp class pair, adds a Props struct + factory to Declarative.h/.cpp, and inserts a style block into DefaultStyle.json. Use this skill whenever the user asks to create, add, scaffold, or bootstrap a new widget, element, or UI component — even if they phrase it casually like "I need a new ProgressBar" or "make a color picker widget". Trigger on: "new element", "new widget", "create a X widget", "add X element", "/new-element", "scaffold X", "bootstrap X".
---

# New Element Bootstrapper

Scaffold a complete new UI element for the elemental-gui C++ framework. This framework uses SDL3 + Cairo for rendering and follows a strict inheritance + declarative API pattern.

## Step 1: Determine name and archetype

If the name wasn't given, ask. Then classify into one of these archetypes — the classification drives what members, overrides, and style you generate:

| Archetype | Base class | When to use |
|---|---|---|
| **display** | `Element` | Shows data passively — no mouse interaction (ProgressBar, Badge, Separator, Avatar) |
| **interactive** | `Element` | Clickable with hover/press states, fires a callback (Toggle, RadioButton, IconButton) |
| **input** | `Element` | Accepts typed text or direct value editing; focus-aware (NumberInput, ColorInput, SearchBox) |
| **range** | `Element` | Draggable numeric value between min and max (Knob, RangeSlider, VolumeBar) |
| **container** | `Container` | Holds and lays out child elements (Card, Panel, GroupBox, TabView) |

If the type is genuinely ambiguous, ask the user. Otherwise just confirm: *"I'll scaffold \<Name\> as a \<archetype\> widget. Sound right?"*

---

## Step 2: Create `src/tui/<Name>.h`

Use this structure. Fill in the archetype-specific parts described below.

```cpp
#pragma once

#include "<BaseClass>.h"  // Element.h or Container.h

namespace gui {
    class <Name> : public <Base> {
    public:
        <Name>();

        void OnDraw(Graphics& g) override;
        <archetype-specific event overrides>

        Size GetPreferredSize() const override;

        <state accessors — getters/setters for the widget's public data>

    private:
        <archetype-specific members>
    };
}
```

### Archetype-specific event overrides and members

**display**
- No event overrides needed beyond OnDraw
- Members: whatever data the widget displays (e.g. `float m_value{0.0f};`, `Range m_range{0.0f, 1.0f};`)
- Callback if the value can be set externally: none needed for pure display

**interactive**
```cpp
// Overrides:
void OnMouseDown(MouseEvent e) override;
void OnMouseUp(MouseEvent e) override;
void OnMouseEnter() override;
void OnMouseLeave() override;

// Members:
private:
    enum class State { Normal, Hover, Click };
    State m_state{State::Normal};
    VoidCallback m_onClick;
```
The state machine: Normal ↔ Hover on enter/leave, Hover → Click on press, Click → Hover on release (+ fire callback).

**input**
```cpp
// Overrides:
void OnMouseDown(MouseEvent e) override;
void OnKeyDown(KeyEvent e) override;
void OnTextInput(TextInputEvent e) override;
void OnFocus(FocusEvent e) override;
void OnBlur(BlurEvent e) override;

// Members:
private:
    std::string m_value{""};
    int m_cursor{0};
    bool m_editing{false};
    ValueChanged<std::string> m_onChanged;
```

**range** (the Slider is the canonical reference — look at `src/tui/Slider.h/cpp`)
```cpp
// Overrides:
void OnMouseDown(MouseEvent e) override;
void OnMouseUp(MouseEvent e) override;
void OnMouseMove(MotionEvent e) override;
void OnMouseEnter() override;
void OnMouseLeave() override;
void OnScroll(ScrollEvent e) override;

// Members:
private:
    enum class State { Normal, Hover, Click };
    State m_state{State::Normal};
    Range m_range{0.0f, 1.0f};
    float m_value{0.0f};
    float m_step{0.01f};
    int m_dragOffset{0};
    ValueChanged<float> m_onValueChange;
```

**container**
- No extra event overrides — `Container::OnEvent` already dispatches to children
- Override `OnDraw` only if you need a custom background/border drawn behind children; if so, call `Container::OnDraw(g)` at the end
- Members: layout-related config (padding, gap, etc.) if needed

---

## Step 3: Create `src/tui/<Name>.cpp`

```cpp
#include "<Name>.h"
#include "Application.h"

namespace gui {

    <Name>::<Name>() : <Base>() {
        SetLocalBounds(Rectangle(0, 0, <w>, <h>));
        // Initialize members
    }

    void <Name>::OnDraw(Graphics& g) {
        Rectangle b = GetBounds();
        Json style = GetStyle()["<Name>"];

        // Draw the widget background/frame using style
        g.StyledRect(b.x, b.y, b.w, b.h, style["normal"]);

        // TODO: draw widget-specific content
    }

    // Implement event handlers here...

    Size <Name>::GetPreferredSize() const {
        if (IsAutoSize()) {
            // Compute natural size from content
            return { <natural_w>, <natural_h> };
        }
        return Element::GetPreferredSize();
    }

}
```

### OnDraw patterns

**interactive** — switch on state string:
```cpp
std::string state = "normal";
switch (m_state) {
    case State::Normal: state = "normal"; break;
    case State::Hover:  state = "hover";  break;
    case State::Click:  state = "click";  break;
}
g.StyledRect(b.x, b.y, b.w, b.h, style[state]);
```

**range** — draw a track + thumb (mirror `Slider::OnDraw`):
```cpp
g.StyledRect(/* track rect */, style["track"]);
g.StyledRect(/* thumb rect */, style["thumb"][state]);
```

**container**:
```cpp
g.StyledRect(b.x, b.y, b.w, b.h, style["background"]);
Container::OnDraw(g);  // draws children on top
```

### Standard event handler patterns

**interactive OnMouseDown/Up:**
```cpp
void <Name>::OnMouseDown(MouseEvent e) {
    if (e.button != MouseButton::Left) return;
    if (m_state == State::Hover) { m_state = State::Click; Invalidate(); }
}
void <Name>::OnMouseUp(MouseEvent e) {
    if (e.button != MouseButton::Left) return;
    if (m_state == State::Click) {
        if (m_onClick) m_onClick();
        m_state = State::Hover;
        Invalidate();
    }
}
void <Name>::OnMouseEnter() { if (m_state == State::Normal)  { m_state = State::Hover;  Invalidate(); } }
void <Name>::OnMouseLeave() { if (m_state != State::Normal)  { m_state = State::Normal; Invalidate(); } }
```

---

## Step 4: Edit `src/tui/Declarative.h`

Insert just before the closing `}` of `namespace gui::declarative`, after the last existing `WidgetDesc` declaration. Pattern:

```cpp
    // --- <Name> widget -------------------------------------
    struct <Name>Props {
        ElementProps base{};
        <widget-specific props — initial value, callback, etc.>
    };
    WidgetDesc <Name>(const <Name>Props& props = {});
```

For widgets that take a label string (like Button, CheckBox), add it as a first parameter:
```cpp
    WidgetDesc <Name>(const std::string& text, const <Name>Props& props = {});
```

Common prop fields by archetype:
- **interactive**: `VoidCallback onClick;`
- **input**: `std::string value{""};` + `ValueChanged<std::string> onChanged;`
- **range**: `Range range{0.0f, 1.0f};` + `float value{0.0f};` + `float step{0.01f};` + `ValueChanged<float> onValueChange;`
- **display**: value fields matching the widget's data (e.g. `float value{0.0f};`)

---

## Step 5: Edit `src/tui/Declarative.cpp`

Insert before the closing `}` of `namespace gui::declarative`. Standard factory pattern:

```cpp
    WidgetDesc <Name>(const <Name>Props& props) {
        return [props](Window& window) -> Element* {
            auto& w = window.Create<gui::<Name>>();
            ElementSetup(w, props.base);
            // Apply widget-specific props:
            // if (props.onClick) w.SetOnClick(props.onClick);
            // w.SetValue(props.value);
            return &w;
        };
    }
```

---

## Step 6: Edit `resources/DefaultStyle.json`

Insert a new top-level key `"<Name>"` before the final `}` of the JSON object. Remember to add a comma after the previous entry.

Use archetype-appropriate style values (colors should follow the existing palette: dark backgrounds ~`#FF1F1F2A`, accent blue ~`#FF3D6BC7`, borders ~`#FF404047`):

**display**:
```json
"<Name>": {
    "background": { "color": "#FF2A2A35" },
    "foreground": { "color": "#FF3D6BC7" },
    "border": { "radius": 4.0, "color": "#FF404047", "width": 1.0 }
}
```

**interactive** (3 states):
```json
"<Name>": {
    "padding": { "horizontal": 8.0, "vertical": 4.0 },
    "normal": {
        "border": { "radius": 6.0, "color": "#FF404047", "width": 1.0 },
        "background": { "color": "#FF2A2A35" }
    },
    "hover": {
        "border": { "radius": 6.0, "color": "#FF4D73CC", "width": 1.0 },
        "background": { "color": "#FF363645" }
    },
    "click": {
        "border": { "radius": 6.0, "color": "#FF2E4780", "width": 1.0 },
        "background": { "color": "#FF1A1A28" }
    }
}
```

**range** (track + thumb states):
```json
"<Name>": {
    "track": {
        "background": { "color": "#FF3A3A45" },
        "border": { "radius": 2.0, "color": "#FF404047", "width": 0.0 }
    },
    "thumb": {
        "normal": { "background": { "color": "#FFDDDDEE" }, "border": { "radius": 6.0, "color": "#FF4D73CC", "width": 1.0 } },
        "hover":  { "background": { "color": "#FFFFFFFF" }, "border": { "radius": 6.0, "color": "#FF5B84E0", "width": 1.0 } },
        "click":  { "background": { "color": "#FFBBBBCC" }, "border": { "radius": 6.0, "color": "#FF2E4780", "width": 1.0 } }
    }
}
```

**container**:
```json
"<Name>": {
    "background": { "color": "#FF252530" },
    "border": { "radius": 8.0, "color": "#FF404047", "width": 1.0 },
    "padding": { "horizontal": 12.0, "vertical": 12.0 }
}
```

---

## Step 7: Confirm and summarise

After writing all files, tell the user:
- Which files were **created**: `src/tui/<Name>.h`, `src/tui/<Name>.cpp`
- Which files were **modified**: `Declarative.h`, `Declarative.cpp`, `resources/DefaultStyle.json`
- CMake uses `GLOB_RECURSE` so the new `.cpp` is picked up automatically — just run `cmake --build build`
- What the user should implement next: the `OnDraw` visual content and any non-trivial event logic
