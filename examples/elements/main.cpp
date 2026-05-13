#include <cmath>

#include <elemental/Application.h>
#include <elemental/Declarative.h>
#include <elemental/EventSystem.h>
#include <elemental/FlexLayout.h>
#include <elemental/TextArea.h>
#include <elemental/backends/sdl3/SDL3Backend.h>

using namespace gui;
namespace dec = gui::declarative;

class App : public Window {
public:
    App()
        : Window(WindowConfig{.title = "Elements Demo", .width = 800, .height = 600}) {}

    gui::Image icons[5];
    gui::Image test;

    dec::WidgetDesc OnBuild() override {
        Show();

        icons[0] = gui::Image("folder.svg");
        icons[1] = gui::Image("file.svg");
        icons[2] = gui::Image("edit.svg");
        icons[3] = gui::Image("delete.svg");
        icons[4] = gui::Image("save.svg");
        test = gui::Image("test.png");

        // clang-format off
        const auto columnProps = dec::ColumnProps{
            .base = dec::ElementProps{
                .flexGrow = 1.0f,
            },
            .gap = 8,
            .padding = EdgeInsets::All(8),
            .align = FlexAlign::Stretch,
            .showBackground = true,
        };

        const auto sectionTextProps = dec::TextProps{
            .base = dec::ElementProps{
                .autoSize = true,
            },
            .align = Alignment::MiddleLeft,
        };

        return dec::Row(dec::RowProps{
            .gap = 10,
            .padding = EdgeInsets::All(16),
            .align = FlexAlign::Stretch,
            .showBackground = false
        }, {
            dec::Column(columnProps, {
                dec::Text("Data Input", dec::TextProps{
                    .base = dec::ElementProps{
                        .autoSize = true,
                        .style = GetApp()->GetStyle()["DefaultTextBig"],
                    },
                    .align = Alignment::MiddleCenter,
                }),

                dec::Text("Simple TextEdit", sectionTextProps),
                dec::TextEdit(dec::TextEditProps{}),

                dec::Text("Masked TextEdit", sectionTextProps),
                dec::TextEdit(dec::TextEditProps{
                    .text = "password123",
                    .masked = true,
                }),

                dec::Text("Multi-line TextEdit", sectionTextProps),
                dec::TextEdit(dec::TextEditProps{
                    .base = dec::ElementProps{
                        .tag = "multiLineEdit",
                    },
                    .text = "Hello, world!\nThis is a multi-line TextEdit.",
                    .multiLine = true,
                }),
                dec::Row(dec::RowProps{
                    .base = dec::ElementProps{
                        .autoSize = true,
                    },
                    .justify = FlexJustify::End
                }, {
                    dec::Button("Bold", dec::ButtonProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .onClick = [this]() {
                            auto* textEdit = FindByTag<gui::TextArea>("multiLineEdit");
                            if (textEdit) {
                                textEdit->Format(FontStyle::Bold, 0.88f, 0.88f, 0.88f);
                            }
                        }
                    })
                }),

                dec::Text("List", sectionTextProps),
                dec::BasicList(dec::BasicListProps{
                    .base = dec::ElementProps{ .flexGrow = 1.0f },
                    .items = std::vector<std::string>{
                        "Item 1",
                        "Item 2",
                        "Item 3",
                        "Item 4",
                        "Item 5",
                        "Item 6"
                    },
                    .onSelectionChanged = [](int index) {
                        printf("Item %d clicked!\n", index);
                    }
                }),
            }),
            dec::ScrollView(
                dec::Column(columnProps, {
                    dec::Text("Interactive", dec::TextProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                            .style = GetApp()->GetStyle()["DefaultTextBig"],
                        },
                        .align = Alignment::MiddleCenter,
                    }),

                    dec::Button("Simple Button", dec::ButtonProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .onClick = []() {
                            printf("Button clicked!\n");
                        }
                    }),
                    dec::Button("Icon Button", dec::ButtonProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .icon = &icons[0],
                        .onClick = []() {
                            printf("Button clicked!\n");
                        }
                    }),

                    // ToolButton row
                    dec::Text("ToolButtons", sectionTextProps),
                    dec::Row(dec::RowProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .gap = 5,
                        .padding = EdgeInsets::All(4),
                        .showBackground = true,
                    }, {
                        dec::ToolRadioButton("", dec::ToolButtonProps{
                            .base = dec::ElementProps{
                                .autoSize = true,
                                .flexGrow = 1.0f,
                            },
                            .icon = &icons[0],
                            .onClick = []() {
                                printf("Folder clicked!\n");
                            },
                            .group = "tools",
                        }),
                        dec::ToolRadioButton("", dec::ToolButtonProps{
                            .base = dec::ElementProps{
                                .autoSize = true,
                                .flexGrow = 1.0f,
                            },
                            .icon = &icons[1],
                            .onClick = []() {
                                printf("File clicked!\n");
                            },
                            .group = "tools",
                        }),
                        dec::ToolRadioButton("", dec::ToolButtonProps{
                            .base = dec::ElementProps{
                                .autoSize = true,
                                .flexGrow = 1.0f,
                            },
                            .icon = &icons[2],
                            .onClick = []() {
                                printf("Edit clicked!\n");
                            },
                            .group = "tools",
                        }),
                        dec::ToolRadioButton("", dec::ToolButtonProps{
                            .base = dec::ElementProps{
                                .autoSize = true,
                                .flexGrow = 1.0f,
                            },
                            .icon = &icons[3],
                            .onClick = []() {
                                printf("Delete clicked!\n");
                            },
                            .group = "tools",
                        }),
                        dec::ToolRadioButton("", dec::ToolButtonProps{
                            .base = dec::ElementProps{
                                .autoSize = true,
                                .flexGrow = 1.0f,
                            },
                            .icon = &icons[4],
                            .onClick = []() {
                                printf("Save clicked!\n");
                            },
                            .group = "tools",
                        })
                    }),

                    dec::Text("Switch", sectionTextProps),
                    dec::Row(dec::RowProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .gap = 10,
                        .align = FlexAlign::Center,
                    }, {
                        dec::Text("Enable feature", dec::TextProps{
                            .base = dec::ElementProps{
                                .autoSize = true,
                                .flexGrow = 1.0f,
                            },
                            .align = Alignment::MiddleLeft,
                        }),
                        dec::Switch(dec::SwitchProps{
                            .onChanged = [](bool checked) {
                                printf("Switch is now %s\n", checked ? "ON" : "OFF");
                            }
                        }),
                    }),

                    dec::Text("Sliders", sectionTextProps),
                    dec::Slider(dec::SliderProps{
                        .direction = Direction::Horizontal,
                        .range = utils::Range{0, 100},
                        .value = 50,
                        .onValueChange = [](float value) {
                            printf("Slider value: %.2f\n", value);
                        },
                    }),
                    dec::Row(dec::RowProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .gap = 10,
                        .justify = FlexJustify::Center,
                    }, {
                        dec::Slider(dec::SliderProps{
                            .base = dec::ElementProps{
                                .autoSize = true,
                                .bounds = Rectangle{0, 0, 30, 100},
                            },
                            .direction = Direction::Vertical,
                            .range = utils::Range{0, 100},
                            .value = 25,
                            .onValueChange = [](float value) {
                                printf("Vertical slider value: %.2f\n", value);
                            },
                        }),
                        dec::Slider(dec::SliderProps{
                            .base = dec::ElementProps{
                                .autoSize = true,
                                .bounds = Rectangle{0, 0, 30, 100},
                            },
                            .direction = Direction::Vertical,
                            .range = utils::Range{0, 100},
                            .value = 50,
                            .onValueChange = [](float value) {
                                printf("Vertical slider value: %.2f\n", value);
                            },
                        }),
                        dec::Slider(dec::SliderProps{
                            .base = dec::ElementProps{
                                .autoSize = true,
                                .bounds = Rectangle{0, 0, 30, 100},
                            },
                            .direction = Direction::Vertical,
                            .range = utils::Range{0, 100},
                            .value = 75,
                            .onValueChange = [](float value) {
                                printf("Vertical slider value: %.2f\n", value);
                            },
                        }),
                    }),

                    dec::Text("Radio Buttons", sectionTextProps),
                    dec::RadioButton("Option 1", dec::RadioButtonProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .group = "options",
                        .onChanged = [](bool checked) {
                            if (checked)
                                printf("Option 1 selected\n");
                        }
                    }),
                    dec::RadioButton("Option 2", dec::RadioButtonProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .group = "options",
                        .onChanged = [](bool checked) {
                            if (checked)
                                printf("Option 2 selected\n");
                        }
                    }),
                    dec::RadioButton("Option 3", dec::RadioButtonProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .group = "options",
                        .onChanged = [](bool checked) {
                            if (checked)
                                printf("Option 3 selected\n");
                        }
                    }),

                    dec::Text("Check Boxes", sectionTextProps),
                    dec::CheckBox("Check 1", dec::CheckBoxProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .onChanged = [](bool checked) {
                            printf("Check 1 is now %s\n", checked ? "Checked" : "Unchecked");
                        }
                    }),
                    dec::CheckBox("Check 2", dec::CheckBoxProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .onChanged = [](bool checked) {
                            printf("Check 2 is now %s\n", checked ? "Checked" : "Unchecked");
                        },
                    }),
                    dec::CheckBox("Check 3", dec::CheckBoxProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .onChanged = [](bool checked) {
                            printf("Check 3 is now %s\n", checked ? "Checked" : "Unchecked");
                        },
                    }),

                    dec::Spinner(),
                }), dec::ScrollViewProps{
                    .base = dec::ElementProps{
                        .flexGrow = 1.0f,
                    },
                    .scrollDirection = Direction::Vertical
                }
            ),
            dec::Column(columnProps, {
                dec::Text("Data Display", dec::TextProps{
                    .base = dec::ElementProps{
                        .autoSize = true,
                        .style = GetApp()->GetStyle()["DefaultTextBig"],
                    },
                    .align = Alignment::MiddleCenter,
                }),

                dec::Text("Image View (Stretch)", sectionTextProps),
                dec::Image(dec::ImageProps{
                    .image = &test,
                    .scaling = ImageScalingMode::Stretch
                }),
                dec::Text("Image View (Contain)", sectionTextProps),
                dec::Image(dec::ImageProps{
                    .image = &test,
                    .scaling = ImageScalingMode::Contain
                }),
                dec::Text("Image View (Cover)", sectionTextProps),
                dec::Image(dec::ImageProps{
                    .image = &test,
                    .scaling = ImageScalingMode::Cover
                }),

                dec::Text("Progress Bars", sectionTextProps),
                dec::ProgressBar(dec::ProgressBarProps{
                    .base = dec::ElementProps{
                        .autoSize = true,
                    },
                    .value = 0.5f
                }),
                dec::ProgressBar(dec::ProgressBarProps{
                    .base = dec::ElementProps{
                        .autoSize = true,
                    },
                    .indeterminate = true,
                }),
                dec::Spacer(),
                dec::Row(dec::RowProps{
                    .base = dec::ElementProps{
                        .autoSize = true,
                    },
                    .gap = 5,
                    .justify = FlexJustify::End,
                }, {
                    dec::Button("Light", dec::ButtonProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .onClick = [this]() {
                            GetApp()->LoadTheme("LightStyle.json");
                        }
                    }),
                    dec::Button("Dark", dec::ButtonProps{
                        .base = dec::ElementProps{
                            .autoSize = true,
                        },
                        .onClick = [this]() {
                            GetApp()->ResetStyle();
                        }
                    }),
                }),
            }),
        });
        // clang-format on
    }
};

int main(int argc, char** argv) {
    gui::Application app{new gui::SDL3Backend()};
    app.CreateWindow<App>();
    return app.Start();
}
