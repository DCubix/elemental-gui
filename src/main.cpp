#include "tui/Application.h"
#include "tui/Panel.h"
#include "tui/Label.h"
#include "tui/Button.h"
#include "tui/Slider.h"
#include "tui/Edit.h"
#include "tui/ScrollView.h"
#include "tui/ImageView.h"
#include "tui/FlexLayout.h"
#include "tui/GLView.h"

#include <SDL3/SDL_opengl.h>

using namespace tui;

class TriangleView : public GLView {
public:
	TriangleView() : GLView([] {
		GLContextConfig cfg;
		cfg.majorVersion = 1;
		cfg.minorVersion = 1;
		cfg.profile = GLContextConfig::Profile::Compatibility;
		return cfg;
	}()) {}

	void OnRender() override {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBegin(GL_TRIANGLES);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex2f( 0.0f,  0.5f);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex2f(-0.5f, -0.5f);
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex2f( 0.5f, -0.5f);
		glEnd();
	}
};

class App : public ApplicationAdapter {
public:
	void OnCreate(Application& app) {
		// --- Title ---
		Label& title = app.Create<Label>();
		title.SetText("Elemental GUI Demo");
		title.SetAlignment(Alignment::MiddleCenter);
		title.GetLocalBounds().h = 30;
		app.GetRoot().Add(&title);

		// --- Main content panel ---
		Panel& mainPanel = app.Create<Panel>();
		mainPanel.SetFlexGrow(1.0f);
		mainPanel.SetBackgroundVisible(false);

		FlexLayout* mainLayout = mainPanel.GetLayout<FlexLayout>();
		mainLayout->SetAlignItems(FlexAlign::Stretch);
		mainLayout->SetJustifyContent(FlexJustify::Start);
		mainLayout->SetPadding(0);
		app.GetRoot().Add(&mainPanel);

		// --- Left controls panel (FlexLayout Column demo) ---
		Panel& controls = app.Create<Panel>();
		controls.GetLocalBounds().w = 200;
		mainPanel.Add(&controls);

		FlexLayout* controlsFlex = controls.GetLayout<FlexLayout>();
		controlsFlex->SetPadding(6);
		controlsFlex->SetGap(4);
		controlsFlex->SetDirection(FlexDirection::Column);

		// Click Me button
		Button& btn1 = app.Create<Button>();
		btn1.SetText("Click Me");
		btn1.GetLocalBounds() = Rectangle(0, 0, 0, 30);
		btn1.SetOnClick([&btn1]() {
			btn1.SetText("Clicked!");
		});
		controls.Add(&btn1);

		// Reset button
		Button& btn2 = app.Create<Button>();
		btn2.SetText("Reset");
		btn2.GetLocalBounds() = Rectangle(0, 0, 0, 30);
		btn2.SetOnClick([&btn1]() {
			btn1.SetText("Click Me");
		});
		controls.Add(&btn2);

		// Slider label
		Label& sliderLabel = app.Create<Label>();
		sliderLabel.SetText("Slider: 50");
		sliderLabel.SetAlignment(Alignment::MiddleLeft);
		sliderLabel.GetLocalBounds() = Rectangle(0, 0, 0, 20);
		controls.Add(&sliderLabel);

		// Slider
		Slider& slider = app.Create<Slider>();
		slider.SetOrientation(Slider::Horizontal);
		slider.SetRange(0, 100);
		slider.SetValue(50);
		slider.SetStep(1);
		slider.GetLocalBounds() = Rectangle(0, 0, 0, 24);
		slider.SetOnValueChange([&slider, &sliderLabel]() {
			sliderLabel.SetText("Slider: " + std::to_string((int)slider.GetValue()));
		});
		controls.Add(&slider);

		// Edit label
		Label& editLabel = app.Create<Label>();
		editLabel.SetText("Text Input:");
		editLabel.SetAlignment(Alignment::MiddleLeft);
		editLabel.GetLocalBounds() = Rectangle(0, 0, 0, 20);
		controls.Add(&editLabel);

		// Edit (grows to fill remaining space)
		Edit& edit = app.Create<Edit>();
		edit.SetMultiLine(true);
		edit.SetText("Type here...\nLine 2\nLine 3");
		edit.GetLocalBounds() = Rectangle(0, 0, 0, 60);
		edit.SetFlexGrow(1.0f);
		controls.Add(&edit);

		// --- Right: ScrollView ---
		ScrollView& sv = app.Create<ScrollView>();
		sv.SetFlexGrow(1.0f);
		mainPanel.Add(&sv);

		Panel& scrollContent = app.Create<Panel>();
		scrollContent.SetLayout(nullptr);  // Absolute positioning
		scrollContent.SetBackgroundVisible(false);
		const int itemHeight = 24;
		const int itemCount = 30;
		const int glViewSize = 200;
		const int contentHeight = itemCount * itemHeight + 16 + glViewSize + 8;
		scrollContent.GetLocalBounds() = Rectangle(0, 0, 400, contentHeight);

		// GL triangle
		TriangleView& triangle = app.Create<TriangleView>();
		triangle.GetLocalBounds() = Rectangle(8, 8, glViewSize, glViewSize);
		scrollContent.Add(&triangle);

		const int labelsOffsetY = 8 + glViewSize + 8;
		for (int i = 0; i < itemCount; i++) {
			Label& item = app.Create<Label>();
			item.SetText("Scroll Item #" + std::to_string(i + 1));
			item.SetAlignment(Alignment::MiddleLeft);
			item.GetLocalBounds() = Rectangle(8, labelsOffsetY + i * itemHeight, 384, itemHeight);
			scrollContent.Add(&item);
		}

		sv.SetElement(&scrollContent);

		
		// --- Bottom toolbar (FlexLayout Row demo) ---
		Panel& toolbar = app.Create<Panel>();
		toolbar.GetLocalBounds().h = 40;
		{
			auto flex = std::make_unique<FlexLayout>(FlexDirection::Row, FlexJustify::SpaceEvenly, FlexAlign::Center);
			flex->SetPadding(4);
			flex->SetGap(8);
			toolbar.SetLayout(std::move(flex));
		}
		app.GetRoot().Add(&toolbar);

		Button& tbBtn1 = app.Create<Button>();
		tbBtn1.SetText("Action 1");
		tbBtn1.GetLocalBounds() = Rectangle(0, 0, 80, 28);
		toolbar.Add(&tbBtn1);

		Button& tbBtn2 = app.Create<Button>();
		tbBtn2.SetText("Action 2");
		tbBtn2.GetLocalBounds() = Rectangle(0, 0, 80, 28);
		toolbar.Add(&tbBtn2);

		Button& tbBtn3 = app.Create<Button>();
		tbBtn3.SetText("Action 3");
		tbBtn3.GetLocalBounds() = Rectangle(0, 0, 80, 28);
		toolbar.Add(&tbBtn3);
	}

	void OnDestroy() {
	}
};

int main(int argc, char** argv) {
	tui::Application app{ "Elemental GUI Demo", 640, 480 };
	return app.Start(new App());
}
