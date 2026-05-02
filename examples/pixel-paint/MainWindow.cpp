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
        : gui::Element() {}

    void OnDraw(gui::Graphics& g) override {
        const gui::Size sz = GetSize();

        g.ClipPushRoundRect(0, 0, sz.w, sz.h, 8.0f);

        g.DrawCheckerboard(0, 0, sz.w, sz.h, 8);

        g.Rect(sz.w / 2 - 2, 0, sz.w / 2 + 4, sz.h);
        g.Color(m_color.r, m_color.g, m_color.b, m_color.a);
        g.Fill();
        g.Rect(0, 0, sz.w / 2, sz.h);
        g.Color(m_color.r, m_color.g, m_color.b, 1.0f);
        g.Fill();

        g.ClipPop();
    }

    gui::Color GetColor() const { return m_color; }
    void SetColor(const gui::Color& color) { m_color = color; }

private:
    gui::Color m_color{gui::Color::FromHex("#000")};
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
        for (const auto& col : m_palette) {
            if (x + cellSize > sz.w) {
                x = 0;
                y += cellSize;
            }
            fnDrawBlock(x, y, cellSize, index == m_selected, col);
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
        for (const auto& col : m_palette) {
            if (x + cellSize > sz.w) {
                x = 0;
                y += cellSize;
            }
            if (gui::Rectangle{x, y, cellSize, cellSize}.HasPoint(e.x, e.y)) {
                SetSelected(index);
                break;
            }
            x += cellSize;
            index++;
        }
    }

    void Add(const gui::Color& color) {
        m_palette.push_back(color);
        Invalidate();
    }

    void Remove(size_t index) {
        if (index == (size_t)-1)
            return;
        m_palette.erase(m_palette.begin() + index);
        m_selected = index == m_selected ? -1 : m_selected;
        Invalidate();
    }

    void Clear() {
        m_palette.clear();
        m_selected = -1;
        Invalidate();
    }

    int GetSelected() const { return m_selected; }
    void SetSelected(int index) {
        m_selected = index;
        m_selected = std::clamp(m_selected, -1, int(m_palette.size()) - 1);
        if (m_onSelect && m_selected >= 0)
            m_onSelect(m_palette[m_selected]);
        Invalidate();
    }

    gui::Size GetPreferredSize() const override {
        const gui::Size sz = GetSize();
        const int cellsPerRow = sz.w / cellSize;
        int rowCount = (int(m_palette.size()) / cellsPerRow) + 1;
        return {cellSize * cellsPerRow, cellSize * rowCount};
    }

    void SetOnSelect(ValueChanged<gui::Color> onSelect) { m_onSelect = onSelect; }

private:
    const int cellSize = 32;
    int m_selected{-1};
    std::vector<gui::Color> m_palette;
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
              .width = 1024,
              .height = 768,
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
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_pencil" }),
            .icon = &icons[icPencil],
            .toggled = true,
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Pencil;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_curve" }),
            .icon = &icons[icCurve],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Curve;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_fill" }),
            .icon = &icons[icFill],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Fill;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_eraser" }),
            .icon = &icons[icEraser],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Eraser;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_eyedrop" }),
            .icon = &icons[icEyedrop],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Eyedrop;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_square" }),
            .icon = &icons[icSquare],
            .onClick = [this]() {
                if (auto* c = FindByTag<Canvas>("canvas"))
                    c->selectedTool = Canvas::ToolType::Square;
            },
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_circle" }),
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
        }, [this](ColorView& el, const DefaultProps& props) {
            auto* canvas = FindByTag<Canvas>("canvas");
            el.SetColor(canvas->colors[0]);
        }),
        dc::Button("", {
            .base = dc::ElementProps{
                .bounds = gui::Rectangle::FromSize(32, 32),
            },
            .icon = &icons[icSwap],
            .iconSize = 20,
            .onClick = [this]() {
                auto* canvas = FindByTag<Canvas>("canvas");
                auto* colorView0 = FindByTag<ColorView>("color0");
                auto* colorView1 = FindByTag<ColorView>("color1");
                auto* picker = FindByTag<gui::ColorPicker>("picker");
                std::swap(canvas->colors[0], canvas->colors[1]);
                colorView0->SetColor(canvas->colors[0]);
                colorView1->SetColor(canvas->colors[1]);
                picker->SetSelected(canvas->colors[0]);
            },
        }),
        dc::Custom<ColorView, DefaultProps>({
            .base = dc::ElementProps{
                .tag = "color1",
                .flexGrow = 1.0f,
            },
        }, [this](ColorView& el, const DefaultProps& props) {
            auto* canvas = FindByTag<Canvas>("canvas");
            el.SetColor(canvas->colors[1]);
        }),
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
                },
                .icon = &icons[icAdd],
                .onClick = [this]() {
                    auto* picker = FindByTag<gui::ColorPicker>("picker");
                    auto* palette = FindByTag<PaletteSelector>("palette");
                    palette->Add(picker->GetSelected());
                },
            }),
            dc::Button("", {
                .base = dc::ElementProps{
                    .bounds = gui::Rectangle::FromSize(24, 24),
                },
                .icon = &icons[icRemove],
                .onClick = [this]() {
                    auto* palette = FindByTag<PaletteSelector>("palette");
                    palette->Remove(palette->GetSelected());
                },
            }),
        }),
        dc::Custom<PaletteSelector, PaletteSelectorProps>({
            .base = dc::ElementProps{
                .tag = "palette",
                .bounds = gui::Rectangle::FromHeight(100),
            },
            .onSelect = [this](gui::Color color) {
                if (FindByTag<PaletteSelector>("palette")->GetSelected() < 0) return;
                auto* canvas = FindByTag<Canvas>("canvas");
                auto* colorView0 = FindByTag<ColorView>("color0");
                auto* picker = FindByTag<gui::ColorPicker>("picker");
                canvas->colors[0] = color;
                colorView0->SetColor(color);
                picker->SetSelected(color);
            },
        }, [](PaletteSelector& el, const PaletteSelectorProps& props) {
            el.SetOnSelect(props.onSelect.value_or(nullptr));
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
                    "Open Image", ".",
                    { "Portable Network Graphics", "*.png" },
                    pfd::opt::none
                ).result();
                if (!res.empty()) {
                    auto* canvas = FindByTag<Canvas>("canvas");
                    auto* palette = FindByTag<PaletteSelector>("palette");
                    canvas->LoadFromFile(res.front());
                    palette->Clear();
                    for (auto col : canvas->ExtractPalette()) {
                        palette->Add(col);
                    }
                    saved = true;
                    fileName = res.front();
                }
            };
            if (!saved) {
                auto res = pfd::message(
                    "Open Image",
                    "Your current image is going to be lost. Continue?",
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
        dc::MenuItem({ .text = "Save", .onClick = []() {
            // TODO: saving...
        } }),
        dc::MenuItem({ .text = "Save As...", .onClick = []() {
            // TODO: saving...
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
                        .onColorPicked = [this]() {
                            auto* canvas = FindByTag<Canvas>("canvas");
                            auto* colorView0 = FindByTag<ColorView>("color0");
                            auto* colorView1 = FindByTag<ColorView>("color1");
                            auto* picker = FindByTag<gui::ColorPicker>("picker");
                            colorView0->SetColor(canvas->colors[0]);
                            colorView1->SetColor(canvas->colors[1]);
                            picker->SetSelected(canvas->colors[0]);
                        },
                        .onImageChanged = [this]() {
                            saved = false;
                        },
                    }, [](Canvas& canvas, const CanvasProps& props) {
                        canvas.onColorPicked = props.onColorPicked.value_or(nullptr);
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
                            auto* colorView0 = FindByTag<ColorView>("color0");
                            canvas->colors[0] = color;
                            colorView0->SetColor(color);
                        },
                    }),
                    colorSelectionArea,
                    colorPaletteArea,
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
}

void MainWindow::RepositionCanvas() {
    auto* canvas = FindByTag<Canvas>("canvas");
    if (!canvas)
        return;
    canvas->Reposition();
}
