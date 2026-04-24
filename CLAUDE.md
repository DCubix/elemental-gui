# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (first time or after CMakeLists changes)
cmake -B build -G Ninja

# Build library and test executable
cmake --build build

# Run the demo app
./build/tui_test
```

Dependencies (SDL3, Cairo, nlohmann_json, nanosvg) are fetched automatically via CPM.cmake on first build. Cairo must be installed on the system; the others are downloaded.

Resource files in `resources/` are embedded as C++ raw-string headers into `src/tui/generated/` by `cmake/embed_resources.cmake` at configure time — edit the `.json` source, not the generated header.

## Architecture Overview

**elemental-gui** is a C++20 desktop GUI framework built on SDL3 (windowing/input) and Cairo (2D rendering). It follows a hierarchical, event-driven model with an optional declarative API.

### Execution Flow

```
Application::Start(adapter)
  → ApplicationAdapter::OnCreate(app)      // user sets up windows
    → app.CreateWindow(...)
      → window.SetRoot(declarative element)
  → Main loop:
    → SDL3 event polling
    → EventSystem dispatch through element tree
    → Incremental redraw (dirty tracking via Invalidate())
    → Cairo renders to SDL3 texture → presented to screen
```

### Core Layers

| Layer | Key Classes | Role |
|---|---|---|
| App/Window | `Application`, `Window` | Lifecycle, multi-window management, SDL3 integration |
| Events | `EventSystem`, `Subscriber` | Pub-sub for mouse, keyboard, text, focus, scroll events |
| Graphics | `Graphics` | Cairo wrapper — shapes, paths, text, SVG, gradients, clipping, shadows |
| Element tree | `Element`, `Container` | Base UI node; parent-child hierarchy; virtual `OnDraw`/`OnEvent` |
| Layout | `Layout`, `FlexLayout` | Flexbox-style layout (direction, justify, align, gap, padding) |
| Styling | JSON themes | `DefaultStyle.json` / `NeumorphismDark` loaded at startup via `Application` |
| Declarative API | `declarative` namespace | Builder functions returning `WidgetDesc` (lazy factories) |

### Adding a New Widget

1. Inherit from `Element` (or `Container` if it holds children).
2. Override `OnDraw(Graphics&)` and optionally `OnEvent(Event&)`.
3. Call `Invalidate()` whenever visual state changes.
4. Add a builder function in `Declarative.h/cpp` following the existing `WidgetDesc` pattern.
5. Expose props via a `struct Props` and wire them in the builder.

### Styling

Styles are JSON objects keyed by widget class name. `Application` loads the default style at startup and merges per-element overrides. To add style keys for a new widget, add entries to `resources/DefaultStyle.json`; the header is regenerated at build time.

### Event System

Events flow from `Window` → `EventSystem::Dispatch` → registered `Subscriber`s. Elements subscribe in their constructor via `EventSystem::Subscribe<EventType>(this)` and unsubscribe in the destructor. Custom events must inherit from `Event` and carry their data as fields.

### Declarative API

```cpp
using namespace elemental::declarative;
window.SetRoot(
    VBox({
        Label({ .text = "Hello" }),
        Button({ .text = "Click me", .onClick = []{ ... } })
    })
);
```

`WidgetDesc` is a `std::function<Element*()>` — the tree is materialized lazily when `SetRoot` resolves it.
