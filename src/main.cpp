#include "tui/Application.h"
#include "tui/Panel.h"
#include "tui/Label.h"
#include "tui/Button.h"
#include "tui/Slider.h"
#include "tui/Edit.h"
#include "tui/Timer.h"
#include "tui/ScrollView.h"
#include "tui/ImageView.h"
#include "tui/FlexLayout.h"
#include "tui/Declarative.h"
#include "tui/Menu.h"

#include <cmath>

using namespace tui;
namespace decl = tui::declarative;

class App : public ApplicationAdapter {
public:
	tui::Image icPen, icSave, icCopy, icSettings;

	void OnCreate(Application& app) {
		icPen = tui::Image("pen.svg");
		icSave = tui::Image("save.svg");
		icCopy = tui::Image("copy.svg");
		icSettings = tui::Image("settings.svg");

		auto gui = decl::SplitView({
			.direction = Direction::Vertical,
			.splitPosition = 200,
			.first = decl::Column({
				.base = { .bounds = Rectangle::FromWidth(200) },
				.gap = 8,
				.padding = EdgeInsets::All(8),
				.align = FlexAlign::Stretch,
				.justify = FlexJustify::Start,
				.showBackground = true
			}, {
				decl::Text("Rotation X", { .base = { .autoSize = true } }),
				decl::Slider({ .base = { .tag = "rot_x" }, .range = { 0.0f, 360.0f }, .value = 0.0f }),
				decl::Text("Rotation Y", { .base = { .autoSize = true } }),
				decl::Slider({ .base = { .tag = "rot_y" }, .range = { 0.0f, 360.0f }, .value = 0.0f }),
				decl::Text("Rotation Z", { .base = { .autoSize = true } }),
				decl::Slider({ .base = { .tag = "rot_z" }, .range = { 0.0f, 360.0f }, .value = 0.0f }),
				decl::Button("Reset", { .base = { .bounds = Rectangle::FromHeight(32) }, .onClick = [&app]() {
					app.FindByTag<Slider>("rot_x")->SetValue(0.0f);
					app.FindByTag<Slider>("rot_y")->SetValue(0.0f);
					app.FindByTag<Slider>("rot_z")->SetValue(0.0f);
				}}),
				decl::Row({
					.base = { .autoSize = true },
					.gap = 4,
					.align = FlexAlign::Start,
					.justify = FlexJustify::Start
				}, {
					decl::ToolRadioButton("", { .base = { .flexGrow = 1.0f, .bounds = Rectangle::FromHeight(32) }, .icon = &icPen, .onClick = []() {
						printf("Pen clicked\n");
					}}),
					decl::ToolRadioButton("", { .base = { .flexGrow = 1.0f, .bounds = Rectangle::FromHeight(32) }, .icon = &icCopy, .onClick = []() {
						printf("Copy clicked\n");
					}}),
					decl::ToolRadioButton("", { .base = { .flexGrow = 1.0f, .bounds = Rectangle::FromHeight(32) }, .icon = &icSave, .onClick = []() {
						printf("Save clicked\n");
					}}),
					decl::ToolRadioButton("", { .base = { .flexGrow = 1.0f, .bounds = Rectangle::FromHeight(32) }, .icon = &icSettings, .onClick = []() {
						printf("Settings clicked\n");
					}}),
				}),
				decl::ProgressBar({ .base = { .bounds = Rectangle(0, 0, 0, 20) }, .range = { 0.0f, 100.0f }, .value = 50.0f, .indeterminate = false }),
				decl::ProgressBar({ .base = { .bounds = Rectangle(0, 0, 0, 20) }, .indeterminate = true }),
				decl::RadioButton("Option 1", { .group = "options", .onChanged = [](bool checked) {
					if (checked) printf("Option 1 selected\n");
				}}),
				decl::RadioButton("Option 2", { .group = "options", .onChanged = [](bool checked) {
					if (checked) printf("Option 2 selected\n");
				}}),
				decl::CheckBox("Check me", { .onChanged = [](bool checked) {
					printf("Checkbox is now %s\n", checked ? "checked" : "unchecked");
				}}),
				decl::BasicList({
					.base = { .flexGrow = 1.0f },
					.items = {
						"Item 1",
						"Item 2",
						"Item 3",
						"Item 4",
						"Item 5"
					},
					.onSelectionChanged = [](int index) {
						printf("Selected index: %d\n", index);
					}
				})
			}),
			.second = decl::Text("Hello, World!", { .base = { .flexGrow = 1.0f } }),
		});
		app.SetRoot(gui(app));
	}

	void OnDestroy() {
	}
};

int main(int argc, char** argv) {
	tui::Application app{ "Elemental GUI Demo", 640, 480 };
	return app.Start(new App());
}
