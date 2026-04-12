#include "tui/Application.h"
#include "tui/Panel.h"
#include "tui/Label.h"
#include "tui/Button.h"
#include "tui/Slider.h"
#include "tui/Edit.h"
#include "tui/ScrollView.h"
#include "tui/ImageView.h"

using namespace tui;

class App : public ApplicationAdapter {
public:
	void OnCreate(Application *app) {
		// --- Title ---
		Label& title = app->Create<Label>(Layout::Top);
		title.SetText("Elemental GUI Demo");
		title.SetAlignment(Alignment::MiddleCenter);
		title.GetLocalBounds().h = 30;

		// --- Left controls panel ---
		Panel& controls = app->Create<Panel>(Layout::Left);
		controls.GetLocalBounds().w = 200;
		controls.GetLayout().SetPadding(6);
		controls.GetLayout().SetGap(4);

		// Row 1: Click Me button
		Button& btn1 = app->Create<Button>();
		btn1.SetText("Click Me");
		btn1.SetOnClick([&btn1]() {
			btn1.SetText("Clicked!");
		});
		controls.Add(&btn1, Layout::Top);

		// Nest: remaining rows
		Panel& row2 = app->Create<Panel>();
		row2.SetBackgroundVisible(false);
		row2.GetLayout().SetPadding(0);
		row2.GetLayout().SetGap(4);
		controls.Add(&row2, Layout::Center);

		Button& btn2 = app->Create<Button>();
		btn2.SetText("Reset");
		btn2.SetOnClick([&btn1]() {
			btn1.SetText("Click Me");
		});
		row2.Add(&btn2, Layout::Top);

		Panel& row3 = app->Create<Panel>();
		row3.SetBackgroundVisible(false);
		row3.GetLayout().SetPadding(0);
		row3.GetLayout().SetGap(4);
		row2.Add(&row3, Layout::Center);

		Label& sliderLabel = app->Create<Label>();
		sliderLabel.SetText("Slider: 50");
		sliderLabel.SetAlignment(Alignment::MiddleLeft);
		sliderLabel.GetLocalBounds().h = 20;
		row3.Add(&sliderLabel, Layout::Top);

		Panel& row4 = app->Create<Panel>();
		row4.SetBackgroundVisible(false);
		row4.GetLayout().SetPadding(0);
		row4.GetLayout().SetGap(4);
		row3.Add(&row4, Layout::Center);

		Slider& slider = app->Create<Slider>();
		slider.SetOrientation(Slider::Horizontal);
		slider.SetRange(0, 100);
		slider.SetValue(50);
		slider.SetStep(1);
		slider.SetOnValueChange([&slider, &sliderLabel]() {
			sliderLabel.SetText("Slider: " + std::to_string((int)slider.GetValue()));
		});
		row4.Add(&slider, Layout::Top);

		Panel& row5 = app->Create<Panel>();
		row5.SetBackgroundVisible(false);
		row5.GetLayout().SetPadding(0);
		row5.GetLayout().SetGap(4);
		row4.Add(&row5, Layout::Center);

		Label& editLabel = app->Create<Label>();
		editLabel.SetText("Text Input:");
		editLabel.SetAlignment(Alignment::MiddleLeft);
		editLabel.GetLocalBounds().h = 20;
		row5.Add(&editLabel, Layout::Top);

		Edit& edit = app->Create<Edit>();
		edit.SetMultiLine(true);
		edit.SetText("Type here...\nLine 2\nLine 3");
		row5.Add(&edit, Layout::Center);

		// --- Right: ScrollView ---
		ScrollView& sv = app->Create<ScrollView>(Layout::Center);

		Panel& scrollContent = app->Create<Panel>();
		scrollContent.SetBackgroundVisible(false);
		const int itemHeight = 24;
		const int itemCount = 30;
		const int contentHeight = itemCount * itemHeight + 16;
		scrollContent.GetLocalBounds() = Rectangle(0, 0, 400, contentHeight);

		for (int i = 0; i < itemCount; i++) {
			Label& item = app->Create<Label>();
			item.SetText("Scroll Item #" + std::to_string(i + 1));
			item.SetAlignment(Alignment::MiddleLeft);
			item.GetLocalBounds() = Rectangle(8, 8 + i * itemHeight, 384, itemHeight);
			scrollContent.Add(&item, Layout::None);
		}

		sv.SetElement(&scrollContent);
	}

	void OnDestroy() {
	}
};

int main(int argc, char** argv) {
	tui::Application app{ "Elemental GUI Demo", 640, 480 };
	return app.Start(new App());
}
