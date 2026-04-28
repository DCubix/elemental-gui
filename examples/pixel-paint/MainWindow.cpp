#include "MainWindow.h"

#include "Canvas.h"

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
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_curve" }),
            .icon = &icons[icCurve],
            .group = "tools",
        })),
        dc::ToolRadioButton("", toolProps.CopyWith({
            .base = BaseOf(toolProps).CopyWith({ .tag = "tool_fill" }),
            .icon = &icons[icFill],
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
                .second = dc::Text("Properties", {
                    .align = gui::utils::Alignment::MiddleCenter,
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

    auto* parent = canvas->GetParent();

    auto cs = canvas->GetSize();
    auto ps = parent->GetSize();
    int cx = (ps.w / 2 - cs.w / 2) + viewPosition.x;
    int cy = (ps.h / 2 - cs.h / 2) + viewPosition.y;

    canvas->SetPosition({cx, cy});
}
