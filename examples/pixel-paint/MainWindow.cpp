#include "MainWindow.h"

#include "Canvas.h"
#include "portable-file-dialogs.h"

#include <ColorPicker.h>
#include <Panel.h>
#include <ToolButton.h>

struct DefaultProps {
    dc::opt<dc::ElementProps> base{};
};

class ColorView : public gui::Element {
public:
    ColorView()
        : gui::Element() {
        color.SetOnUpdate([this]() { Invalidate(); });
    }

    void OnDraw(gui::Graphics& g) override {
        const gui::Size sz = GetSize();

        g.ClipPushRoundRect(0, 0, sz.w, sz.h, 8.0f);

        g.DrawCheckerboard(0, 0, sz.w, sz.h, 8);

        g.Rect(sz.w / 2 - 2, 0, sz.w / 2 + 4, sz.h);
        g.Color(color().r, color().g, color().b, color().a);
        g.Fill();
        g.Rect(0, 0, sz.w / 2, sz.h);
        g.Color(color().r, color().g, color().b, 1.0f);
        g.Fill();

        g.ClipPop();
    }

    gui::Property<gui::Color> color{gui::Color::FromHex("#000")};
};

class PaletteSelector : public gui::Element {
public:
    PaletteSelector()
        : gui::Element() {}

    void OnDraw(gui::Graphics& g) override {
        auto fnDrawBlock = [&g](int x, int y, int size, bool selected, const gui::Color& col) {
            gui::Rectangle b{x + 2, y + 2, size - 4, size - 4};

            g.ClipPushRoundRect(b.x, b.y, b.w, b.h, 6.0f);
            g.DrawCheckerboard(b.x, b.y, b.w, b.h, 6);

            g.Rect(b.x, b.y, b.w, b.h);
            g.Color(col.r, col.g, col.b, col.a);
            g.Fill();

            g.ClipPushPath([&]() {
                g.BeginPath();
                g.MoveTo(x, y);
                g.LineTo(x + size, y);
                g.LineTo(x, y + size);
                g.ClosePath();
            });
            g.Rect(b.x, b.y, b.w, b.h);
            g.Color(col.r, col.g, col.b, 1.0f);
            g.Fill();
            g.ClipPop();

            g.ClipPop();

            if (selected) {
                g.RoundRect(b.x, b.y, b.w, b.h, 6.0f);
                g.LineWidth(4.0f);
                g.Color(0.0f, 0.0f, 0.0f);
                g.Stroke(true);

                g.LineWidth(2.0f);
                g.Color(1.0f, 1.0f, 1.0f);
                g.Stroke();
            }
        };

        const gui::Size sz = GetSize();
        const int cellsPerRow = sz.w / cellSize;

        int x = 0, y = 0;
        int index = 0;
        for (const auto& col : palette()) {
            if (x + cellSize > sz.w) {
                x = 0;
                y += cellSize;
            }
            fnDrawBlock(x, y, cellSize, index == selected(), col);
            x += cellSize;
            index++;
        }
    }

    void OnMouseUp(gui::MouseEvent e) override {
        if (e.button != gui::MouseButton::Left)
            return;
        const gui::Size sz = GetSize();
        const int cellsPerRow = sz.w / cellSize;
        int x = 0, y = 0;
        int index = 0;
        for (const auto& col : palette()) {
            if (x + cellSize > sz.w) {
                x = 0;
                y += cellSize;
            }
            if (gui::Rectangle{x, y, cellSize, cellSize}.HasPoint(e.x, e.y)) {
                selected = index;
                break;
            }
            x += cellSize;
            index++;
        }
    }

    void Add(const gui::Color& color) {
        palette.PushBack(color);
        Invalidate();
    }

    void Remove(size_t index) {
        if (index == (size_t)-1)
            return;
        palette.EraseAt(index);
        selected = index == selected() ? -1 : selected();
        Invalidate();
    }

    void Clear() {
        palette.Clear();
        selected = -1;
        Invalidate();
    }

    gui::Size GetPreferredSize() const override {
        const gui::Size sz = GetSize();
        const int cellsPerRow = sz.w / cellSize;
        int rowCount = palette.Size() / cellsPerRow + 1;
        return {cellSize * cellsPerRow, cellSize * rowCount};
    }

    void SetOnSelect(ValueChanged<gui::Color> onSelect) { m_onSelect = onSelect; }

    gui::Property<int> selected{-1};
    gui::Computed<gui::Color> selectedColor{gui::Computed<gui::Color>(
        [this]() {
            if (selected() < 0)
                return gui::Color{0, 0, 0, 1};
            return palette[selected()];
        },
        selected,
        palette
    )};

    gui::Property<std::vector<gui::Color>> palette;

private:
    const int cellSize = 24;
    ValueChanged<gui::Color> m_onSelect;
};

struct PaletteSelectorProps {
    dc::opt<dc::ElementProps> base{std::nullopt};
    dc::opt<ValueChanged<gui::Color>> onSelect{std::nullopt};
};

MainWindow::MainWindow()
    : gui::Window(
          gui::WindowConfig{
              .title = "Pixel Paint",
              .width = 1280,
              .height = 800,
              .resizable = true,
          }
      ) {}

dc::WidgetDesc MainWindow::OnBuild() {
    icons[icPencil] = gui::Image("pencil.svg");
    icons[icEraser] = gui::Image("eraser.svg");
    icons[icEyedrop] = gui::Image("eyedrop.svg");
    icons[icCurve] = gui::Image("curve.svg");
    icons[icSquare] = gui::Image("square.svg");
    icons[icCircle] = gui::Image("ellipse.svg");
    icons[icFill] = gui::Image("color-fill.svg");
    icons[icPalette] = gui::Image("color-palette.svg");
    icons[icSwap] = gui::Image("swap-horizontal.svg");
    icons[icAdd] = gui::Image("add.svg");
    icons[icRemove] = gui::Image("remove.svg");
    icons[icTrash] = gui::Image("trash.svg");
    icons[icNewLayer] = gui::Image("duplicate.svg");
    icons[icUp] = gui::Image("arrow-up.svg");
    icons[icDown] = gui::Image("arrow-down.svg");

    Show();

    // clang-format off
    const auto toolProps = dc::ToolButtonProps{
        .base = dc::ElementProps{
            .autoSize = true,
        },
        .iconSize = 20,
    };

    const auto textProps = dc::TextProps{
        .base = dc::ElementProps{
            .autoSize = true,
            .style = Json::parse(R"({
                "fontSize": 17.0,
                "fontWeight": "bold",
                "color": "#80FFFFFF"
            })"),
        },
        .align = gui::utils::Alignment::MiddleLeft,
    };

    // Painting toolbar
    auto toolBar = dc::Column({
        .base = dc::ElementProps{
            .autoSize = true,
        },
        .gap = 4,
        .padding = gui::EdgeInsets::All(4),
        .align = gui::FlexAlign::Stretch,
        .justify = gui::FlexJustify::Start,
        .showBackground = true,
    }, {
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_pencil", .tooltip = "Pencil" }),
            .icon = &icons[icPencil],
            .toggled = true,
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Pencil;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_curve", .tooltip = "Curve" }),
            .icon = &icons[icCurve],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Curve;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_fill", .tooltip = "Fill" }),
            .icon = &icons[icFill],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Fill;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_eraser", .tooltip = "Eraser" }),
            .icon = &icons[icEraser],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Eraser;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_eyedrop", .tooltip = "Eyedropper" }),
            .icon = &icons[icEyedrop],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Eyedrop;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_square", .tooltip = "Rectangle. (*Shift* = Square)" }),
            .icon = &icons[icSquare],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Square;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_circle", .tooltip = "Ellipse. (*Shift* = Circle)" }),
            .icon = &icons[icCircle],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Circle;
            },
            .group = "tools",
        })),
        dc::Spacer(),
    });

    const auto colorSelectionArea = dc::Row({
        .base = dc::ElementProps{
            .bounds = gui::Rectangle::FromHeight(32),
        },
        .gap = 8,
        .align = gui::FlexAlign::Stretch,
    }, {
        dc::Custom<ColorView, DefaultProps>({
            .base = dc::ElementProps{
                .tag = "color0",
                .flexGrow = 1.0f,
            },
        }, [this](ColorView& el, const DefaultProps& props) {}),
        dc::Button("", {
            .base = dc::ElementProps{
                .bounds = gui::Rectangle::FromSize(32, 32),
                .tooltip = "Swap Colors",
            },
            .icon = &icons[icSwap],
            .iconSize = 20,
            .onClick = [this]() {
                auto* canvas = FindByTag<Canvas>("canvas");
                auto tmp = canvas->colors[0]();
                canvas->colors[0] = canvas->colors[1]();
                canvas->colors[1] = tmp;
            },
        }),
        dc::Custom<ColorView, DefaultProps>({
            .base = dc::ElementProps{
                .tag = "color1",
                .flexGrow = 1.0f,
            },
        }, [this](ColorView& el, const DefaultProps& props) {}),
    });

    const auto colorPaletteArea = dc::Column({
        .base = dc::ElementProps{
            .autoSize = true,
        },
        .gap = 4,
        .align = gui::FlexAlign::Stretch,
    }, {
        dc::Row({
            .base = dc::ElementProps{
                .bounds = gui::Rectangle::FromHeight(32),
            },
            .gap = 6,
            .justify = gui::FlexJustify::End,
        }, {
            dc::Text("Palette", textProps.CopyWith({
                .base = BaseOf(textProps).CopyWith({ .flexGrow = 1.0f }),
            })),
            dc::Button("", {
                .base = dc::ElementProps{
                    .bounds = gui::Rectangle::FromSize(24, 24),
                    .tooltip = "Add to palette",
                },
                .icon = &icons[icAdd],
                .onClick = [this]() {
                    auto* picker = FindByTag<gui::ColorPicker>("picker");
                    auto* palette = FindByTag<PaletteSelector>("palette");
                    palette->Add(picker->selected());
                },
            }),
            dc::Button("", {
                .base = dc::ElementProps{
                    .bounds = gui::Rectangle::FromSize(24, 24),
                    .tooltip = "Remove selected",
                },
                .icon = &icons[icRemove],
                .onClick = [this]() {
                    auto* palette = FindByTag<PaletteSelector>("palette");
                    palette->Remove(palette->selected());
                },
            }),
        }),
        dc::ScrollView(dc::Custom<PaletteSelector, PaletteSelectorProps>({
            .base = dc::ElementProps{
                .tag = "palette",
                .autoSize = true,
            },
            .onSelect = [this](gui::Color color) {
                if (FindByTag<PaletteSelector>("palette")->selected() < 0) return;
                auto* canvas = FindByTag<Canvas>("canvas");
                canvas->colors[0] = color;
            },
        }, [](PaletteSelector& el, const PaletteSelectorProps& props) {
            el.SetOnSelect(props.onSelect.value_or(nullptr));
        }), {
            .base = dc::ElementProps{
                .bounds = gui::Rectangle::FromHeight(150),
            },
            .scrollDirection = gui::utils::Direction::Vertical,
        }),
    });

    const auto layersArea = dc::Column({
        .base = dc::ElementProps{
            .flexGrow = 1.0f,
        },
        .gap = 4,
        .align = gui::FlexAlign::Stretch,
    }, {
        dc::Row({
            .base = dc::ElementProps{
                .bounds = gui::Rectangle::FromHeight(32),
            },
            .gap = 6,
            .justify = gui::FlexJustify::End,
        }, {
            dc::Text("Layers", textProps.CopyWith({
                .base = BaseOf(textProps).CopyWith({ .flexGrow = 1.0f }),
            })),
            dc::Button("", {
                .base = dc::ElementProps{
                    .bounds = gui::Rectangle::FromSize(24, 24),
                    .tooltip = "New Layer",
                },
                .icon = &icons[icNewLayer],
                .onClick = [this]() {
                    auto* canvas = FindByTag<Canvas>("canvas");
                    canvas->NewLayer();
                },
            }),
            dc::Button("", {
                .base = dc::ElementProps{
                    .bounds = gui::Rectangle::FromSize(24, 24),
                    .tooltip = "Delete selected layer",
                },
                .icon = &icons[icTrash],
                .onClick = [this]() {
                    auto* layers = FindByTag<gui::List<gui::Image*>>("layers");
                    if (layers->selectedIndex() < 0) return;

                    auto* canvas = FindByTag<Canvas>("canvas");
                    canvas->DeleteLayer(layers->selectedIndex());
                },
            }),
            dc::Button("", {
                .base = dc::ElementProps{
                    .bounds = gui::Rectangle::FromSize(24, 24),
                    .tooltip = "Move selected up",
                },
                .icon = &icons[icUp],
                .onClick = [this]() {
                    auto* layers = FindByTag<gui::List<gui::Image*>>("layers");
                    if (layers->selectedIndex() < 0) return;

                    auto* canvas = FindByTag<Canvas>("canvas");
                    canvas->MoveLayerUp(layers->selectedIndex());
                },
            }),
            dc::Button("", {
                .base = dc::ElementProps{
                    .bounds = gui::Rectangle::FromSize(24, 24),
                    .tooltip = "Move selected down",
                },
                .icon = &icons[icDown],
                .onClick = [this]() {
                    auto* layers = FindByTag<gui::List<gui::Image*>>("layers");
                    if (layers->selectedIndex() < 0) return;

                    auto* canvas = FindByTag<Canvas>("canvas");
                    canvas->MoveLayerDown(layers->selectedIndex());
                },
            }),
        }),
        dc::List<gui::Image*>({
            .base = dc::ElementProps{
                .tag = "layers",
                .flexGrow = 1.0f,
                .style = Json::parse(R"({
                    "item": {
                        "height": 48.0,
                        "iconSize": 999.0
                    }
                })"),
            },
            .selectedIndex = 0,
            .labelBuilder = [](uint index, gui::Image* image) {
                return "Layer " + std::to_string(index + 1);
            },
            .iconBuilder = [](uint index, gui::Image* image) {
                return image;
            },
        }),
    });

    const auto mnuFile = dc::Menu({}, {
        dc::MenuItem({ .text = "New", .onClick = [this]() {
            if (!saved) {
                auto res = pfd::message(
                    "New Image",
                    "Your current image is going to be lost. Continue?",
                    pfd::choice::yes_no,
                    pfd::icon::warning
                ).result();
                if (res == pfd::button::yes) {
                    FindByTag<Canvas>("canvas")->LoadEmpty();
                    fileName = "";
                    saved = true;
                }
            } else {
                FindByTag<Canvas>("canvas")->LoadEmpty();
                fileName = "";
                saved = true;
            }
        } }),
        dc::MenuSeparator(),
        dc::MenuItem({ .text = "Open", .onClick = [this]() {
            auto fnOpen = [this]() {
                auto res = pfd::open_file(
                    "Open Art File", ".",
                    { "Pixel Art File", "*.art" },
                    pfd::opt::none
                ).result();
                if (!res.empty()) {
                    auto* canvas = FindByTag<Canvas>("canvas");
                    canvas->LoadArtFile(res.front());
                    saved = true;
                    fileName = res.front();
                }
            };
            if (!saved) {
                auto res = pfd::message(
                    "Open Art File",
                    "Your current art is going to be lost. Continue?",
                    pfd::choice::yes_no,
                    pfd::icon::warning
                ).result();
                if (res == pfd::button::yes) {
                    fnOpen();
                }
            } else {
                fnOpen();
            }
        } }),
        dc::MenuItem({ .text = "Save", .onClick = [this]() {
            auto fnSave = [this]() {
                auto* canvas = FindByTag<Canvas>("canvas");
                canvas->SaveArtFile(fileName);
                saved = true;
            };
            if (!saved) {
                auto res = pfd::save_file(
                    "Save Art File", ".",
                    { "Pixel Art File", "*.art" }
                ).result();
                if (!res.empty()) {
                    fileName = res;
                    fnSave();
                }
            } else {
                fnSave();
            }
        } }),
        dc::MenuItem({ .text = "Save As...", .onClick = [this]() {
            auto res = pfd::save_file(
                "Save Art File As...", ".",
                { "Pixel Art File", "*.art" }
            ).result();
            if (!res.empty()) {
                fileName = res;
                auto* canvas = FindByTag<Canvas>("canvas");
                canvas->SaveArtFile(fileName);
                saved = true;
            }
        } }),
        dc::MenuSeparator(),
        dc::MenuItem({ .text = "Import PNG...", .onClick = [this]() {
            auto fnOpen = [this]() {
                auto res = pfd::open_file(
                    "Import PNG Image", ".",
                    { "Portable Network Graphics", "*.png" },
                    pfd::opt::none
                ).result();
                if (!res.empty()) {
                    auto* canvas = FindByTag<Canvas>("canvas");
                    canvas->LoadFromPNG(res.front());
                    saved = false;
                    fileName = "";
                }
            };
            if (!saved) {
                auto res = pfd::message(
                    "Import PNG Image",
                    "Your current art is going to be lost. Continue?",
                    pfd::choice::yes_no,
                    pfd::icon::warning
                ).result();
                if (res == pfd::button::yes) {
                    fnOpen();
                }
            } else {
                fnOpen();
            }
        } }),
        dc::MenuItem({ .text = "Export PNG...", .onClick = [this]() {
            auto res = pfd::save_file(
                "Export PNG Image", ".",
                { "Portable Network Graphics", "*.png" }
            ).result();
            if (!res.empty()) {
                auto* canvas = FindByTag<Canvas>("canvas");
                canvas->SaveToPNG(res);
            }
        } }),
        dc::MenuSeparator(),
        dc::MenuItem({ .text = "Exit", .onClick = [this]() {
            if (!saved) {
                auto res = pfd::message(
                    "Open Image",
                    "Your current image is going to be lost. Continue?",
                    pfd::choice::yes_no,
                    pfd::icon::warning
                ).result();
                if (res == pfd::button::yes) {
                    Close();
                }
            } else {
                Close();
            }
        } }),
    });

    const auto mnuEdit = dc::Menu({}, {
        dc::MenuItem({ .base = dc::ElementProps{ .tag = "undo" }, .text = "Undo", .onClick = [this]() {
            FindByTag<Canvas>("canvas")->Undo();
        } }),
        dc::MenuItem({ .base = dc::ElementProps{ .tag = "redo" }, .text = "Redo", .onClick = [this]() {
            FindByTag<Canvas>("canvas")->Redo();
        } }),
    });

    return dc::Column({
        .align = gui::FlexAlign::Stretch,
        .justify = gui::FlexJustify::Start,
    }, {
        dc::Row({
            .base = dc::ElementProps{
                .autoSize = true,
            },
            .gap = 4,
            .padding = gui::EdgeInsets::All(4),
        }, {
            dc::ToolButton("File", toolProps.CopyWith({
                .base = BaseOf(toolProps).CopyWith({ .tag = "file" }),
                .onClick = [mnuFile, this]() {
                    auto* menu = mnuFile(*this);
                    FindByTag<gui::ToolButton>("file")->ShowPopup(menu);
                },
            })),
            dc::ToolButton("Edit", toolProps.CopyWith({
                .base = BaseOf(toolProps).CopyWith({ .tag = "edit" }),
                .onClick = [mnuEdit, this]() {
                    auto* menu = mnuEdit(*this);
                    FindByTag<gui::ToolButton>("edit")->ShowPopup(menu);
                },
            })),
        }),
        dc::Row({
            .base = dc::ElementProps{
                .autoSize = true,
                .flexGrow = 1.0f,
            },
            .gap = 4,
            .padding = gui::EdgeInsets::All(6),
            .align = gui::FlexAlign::Stretch,
        }, {
            toolBar,
            dc::SplitView({
                .base = dc::ElementProps{
                    .flexGrow = 1.0f,
                },
                .direction = gui::utils::Direction::Vertical,
                .splitPosition = 200,
                .align = gui::SplitViewAlign::End,
                .first = dc::Panel({
                    .base = dc::ElementProps{
                        .style = Json::parse(R"({
                            "elevation": 0.0,
                            "border": {
                                "radius": 6.0,
                                "color": null
                            },
                            "background": {
                                "color": "#333"
                            }
                        })"),
                    },
                    .showBackground = true,
                }, {
                    dc::Custom<Canvas, CanvasProps>(CanvasProps{
                        .base = dc::ElementProps{
                            .tag = "canvas",
                            .bounds = gui::Rectangle{ -999, 0, 400, 400 },
                            .style = Json::parse(R"({
                                "elevation": 8.0
                            })"),
                        },
                        .onImageChanged = [this]() {
                            saved = false;
                        },
                    }, [](Canvas& canvas, const CanvasProps& props) {
                        canvas.onImageChanged = props.onImageChanged.value_or(nullptr);
                    }),
                }),
                .second = dc::Column({
                    .gap = 12,
                    .padding = gui::EdgeInsets::All(6),
                    .align = gui::FlexAlign::Stretch,
                }, {
                    dc::Text("Color Picker", textProps),
                    dc::ColorPicker({
                        .base = dc::ElementProps{
                            .tag = "picker",
                            .bounds = gui::Rectangle::FromSize(150, 200),
                        },
                        .onChange = [this](gui::Color color) {
                            auto* canvas = FindByTag<Canvas>("canvas");
                            canvas->colors[0] = color;
                        },
                    }),
                    colorSelectionArea,
                    colorPaletteArea,
                    layersArea,
                }),
            }),
        }),
    });
    // clang-format on
}

void MainWindow::OnResize() {
    RepositionCanvas();
}

void MainWindow::OnCreate() {
    RepositionCanvas();

    // DataBinding setup
    auto* canvas = FindByTag<Canvas>("canvas");
    auto* colorView0 = FindByTag<ColorView>("color0");
    auto* colorView1 = FindByTag<ColorView>("color1");
    auto* picker = FindByTag<gui::ColorPicker>("picker");
    auto* palette = FindByTag<PaletteSelector>("palette");

    picker->selected.Bind(canvas->colors[0]);
    palette->selectedColor.Bind(canvas->colors[0]);

    canvas->colors[0].Bind(colorView0->color);
    canvas->colors[1].Bind(colorView1->color);

    auto* list = FindByTag<gui::List<gui::Image*>>("layers");
    canvas->layersOrdered.Bind(list->items);
    canvas->currentLayer.Bind(list->selectedIndex);

    canvas->palette.Bind(palette->palette);

    canvas->LoadEmpty();
}

void MainWindow::RepositionCanvas() {
    auto* canvas = FindByTag<Canvas>("canvas");
    if (!canvas)
        return;
    canvas->Reposition();
}
