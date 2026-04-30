#include "MainWindow.h"

#include "Canvas.h"

#include <ToolButton.h>

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

    Show();

    // clang-format off
    const auto toolProps = dc::ToolButtonProps{
        .base = dc::ElementProps{
            .autoSize = true,
        },
        .iconSize = 20,
    };

    auto colorPickerMenu = dc::Menu({}, {
        dc::ColorPicker({
            .base = dc::ElementProps{
                .bounds = gui::Rectangle::FromSize(150, 150),
            },
            .onChange = [this](gui::Color color) {
                FindByTag<Canvas>("canvas")->colors[0] = color;
            },
        }),
    });

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
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_eyedrop" }),
            .icon = &icons[icEyedrop],
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_square" }),
            .icon = &icons[icSquare],
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_circle" }),
            .icon = &icons[icCircle],
            .group = "tools",
        })),
        dc::ToolButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_palette" }),
            .icon = &icons[icPalette],
            .onClick = [colorPickerMenu, this]() {
                auto* menu = colorPickerMenu(*this);
                FindByTag<gui::ToolButton>("tool_palette")->ShowPopup(menu);
            },
        })),
        dc::Spacer(),
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
            dc::ToolButton("File", toolProps.CopyWith({ .base = BaseOf(toolProps).CopyWith({ .tag = "file" }) })),
            dc::ToolButton("Edit", toolProps.CopyWith({ .base = BaseOf(toolProps).CopyWith({ .tag = "edit" }) })),
            dc::ToolButton("View", toolProps.CopyWith({ .base = BaseOf(toolProps).CopyWith({ .tag = "view" }) })),
            dc::ToolButton("Help", toolProps.CopyWith({ .base = BaseOf(toolProps).CopyWith({ .tag = "help" }) })),
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
                    }, [](Canvas& canvas, const CanvasProps& props) {}),
                }),
                .second = dc::Column({
                    .align = gui::FlexAlign::Stretch,
                }, {
                    
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
