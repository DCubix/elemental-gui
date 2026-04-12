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
#include "tui/GLView.h"

#include <SDL3/SDL_opengl.h>
#include <cmath>

using namespace tui;

class CubeView : public GLView {
public:
	Timer timer{};
	float angle = 0.0f;

	CubeView() : GLView([] {
		GLContextConfig cfg;
		cfg.majorVersion = 1;
		cfg.minorVersion = 1;
		cfg.profile = GLContextConfig::Profile::Compatibility;
		return cfg;
	}()) {
		timer.Start(33, [this]() {
			angle += 2.0f;
			if (angle >= 360.0f) angle -= 360.0f;
			Invalidate();
		});
	}

	void OnRender() override {
		float sliderValue = GetApp()->FindByTag<Slider>("scale")->GetValue() / 100.0f;
		sliderValue = 0.1f + sliderValue * 0.9f; // Scale from 0.1 to 1.0

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float aspect = 1.0f;
		auto bounds = GetLocalBounds();
		if (bounds.h > 0) aspect = (float)bounds.w / (float)bounds.h;
		float fov = 45.0f;
		float near = 0.1f, far = 100.0f;
		float top = near * tanf(fov * 3.14159265f / 360.0f);
		float right = top * aspect;
		glFrustum(-right, right, -top, top, near, far);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -4.0f);
		glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(angle, 0.0f, 1.0f, 0.0f);
		glScalef(sliderValue, sliderValue, sliderValue);

		glBegin(GL_QUADS);
			// Front (red)
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(-1, -1,  1);
			glVertex3f( 1, -1,  1);
			glVertex3f( 1,  1,  1);
			glVertex3f(-1,  1,  1);
			// Back (green)
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3f(-1, -1, -1);
			glVertex3f(-1,  1, -1);
			glVertex3f( 1,  1, -1);
			glVertex3f( 1, -1, -1);
			// Top (blue)
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f(-1,  1, -1);
			glVertex3f(-1,  1,  1);
			glVertex3f( 1,  1,  1);
			glVertex3f( 1,  1, -1);
			// Bottom (yellow)
			glColor3f(1.0f, 1.0f, 0.0f);
			glVertex3f(-1, -1, -1);
			glVertex3f( 1, -1, -1);
			glVertex3f( 1, -1,  1);
			glVertex3f(-1, -1,  1);
			// Right (magenta)
			glColor3f(1.0f, 0.0f, 1.0f);
			glVertex3f( 1, -1, -1);
			glVertex3f( 1,  1, -1);
			glVertex3f( 1,  1,  1);
			glVertex3f( 1, -1,  1);
			// Left (cyan)
			glColor3f(0.0f, 1.0f, 1.0f);
			glVertex3f(-1, -1, -1);
			glVertex3f(-1, -1,  1);
			glVertex3f(-1,  1,  1);
			glVertex3f(-1,  1, -1);
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
		title.SetLocalBounds(Rectangle(0, 0, 50, 30));
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
		controls.SetLocalBounds(Rectangle(0, 0, 200, 50));
		mainPanel.Add(&controls);

		FlexLayout* controlsFlex = controls.GetLayout<FlexLayout>();
		controlsFlex->SetPadding(6);
		controlsFlex->SetGap(4);
		controlsFlex->SetDirection(FlexDirection::Column);

		// Click Me button
		Button& btn1 = app.Create<Button>();
		btn1.SetText("Click Me");
		btn1.SetLocalBounds(Rectangle(0, 0, 0, 30));
		btn1.SetOnClick([&btn1]() {
			btn1.SetText("Clicked!");
		});
		controls.Add(&btn1);

		// Reset button
		Button& btn2 = app.Create<Button>();
		btn2.SetText("Reset");
		btn2.SetLocalBounds(Rectangle(0, 0, 0, 30));
		btn2.SetOnClick([&btn1]() {
			btn1.SetText("Click Me");
		});
		controls.Add(&btn2);

		// Slider label
		Label& sliderLabel = app.Create<Label>();
		sliderLabel.SetText("Slider: 50");
		sliderLabel.SetAlignment(Alignment::MiddleLeft);
		sliderLabel.SetLocalBounds(Rectangle(0, 0, 0, 20));
		controls.Add(&sliderLabel);

		// Slider
		Slider& slider = app.Create<Slider>();
		slider.SetTag("scale");
		slider.SetOrientation(Slider::Horizontal);
		slider.SetRange(0, 100);
		slider.SetValue(50);
		slider.SetStep(1);
		slider.SetLocalBounds(Rectangle(0, 0, 0, 24));
		slider.SetOnValueChange([&slider, &sliderLabel]() {
			sliderLabel.SetText("Slider: " + std::to_string((int)slider.GetValue()));
		});
		controls.Add(&slider);

		// Edit label
		Label& editLabel = app.Create<Label>();
		editLabel.SetText("Text Input:");
		editLabel.SetAlignment(Alignment::MiddleLeft);
		editLabel.SetLocalBounds(Rectangle(0, 0, 0, 20));
		controls.Add(&editLabel);

		// Edit (grows to fill remaining space)
		Edit& edit = app.Create<Edit>();
		edit.SetMultiLine(true);
		edit.SetText("Type here...\nLine 2\nLine 3");
		edit.SetLocalBounds(Rectangle(0, 0, 0, 60));
		edit.SetFlexGrow(1.0f);
		controls.Add(&edit);

		// --- Right: ScrollView ---
		ScrollView& sv = app.Create<ScrollView>();
		sv.SetFlexGrow(1.0f);
		mainPanel.Add(&sv);

		Panel& scrollContent = app.Create<Panel>();
		scrollContent.SetBackgroundVisible(false);

		FlexLayout* scrollLayout = scrollContent.GetLayout<FlexLayout>();
		scrollLayout->SetDirection(FlexDirection::Column);
		
		const int itemHeight = 24;
		const int itemCount = 30;
		const int glViewSize = 200;

		// GL triangle
		CubeView& triangle = app.Create<CubeView>();
		triangle.SetLocalBounds(Rectangle(8, 8, glViewSize, glViewSize));
		scrollContent.Add(&triangle);

		const int labelsOffsetY = 8 + glViewSize + 8;
		for (int i = 0; i < itemCount; i++) {
			Label& item = app.Create<Label>();
			item.SetText("Scroll Item #" + std::to_string(i + 1));
			item.SetAlignment(Alignment::MiddleLeft);
			item.SetLocalBounds(Rectangle(8, labelsOffsetY + i * itemHeight, 384, itemHeight));
			scrollContent.Add(&item);
		}

		sv.SetElement(&scrollContent);

		// --- Bottom toolbar (FlexLayout Row demo) ---
		Panel& toolbar = app.Create<Panel>();
		toolbar.SetLocalBounds(Rectangle(0, 0, 50, 40));
		{
			auto flex = std::make_unique<FlexLayout>(FlexDirection::Row, FlexJustify::SpaceEvenly, FlexAlign::Center);
			flex->SetPadding(4);
			flex->SetGap(8);
			toolbar.SetLayout(std::move(flex));
		}
		app.GetRoot().Add(&toolbar);

		Button& tbBtn1 = app.Create<Button>();
		tbBtn1.SetText("Action 1");
		tbBtn1.SetLocalBounds(Rectangle(0, 0, 80, 28));
		toolbar.Add(&tbBtn1);

		Button& tbBtn2 = app.Create<Button>();
		tbBtn2.SetText("Action 2");
		tbBtn2.SetLocalBounds(Rectangle(0, 0, 80, 28));
		toolbar.Add(&tbBtn2);

		Button& tbBtn3 = app.Create<Button>();
		tbBtn3.SetText("Action 3");
		tbBtn3.SetLocalBounds(Rectangle(0, 0, 80, 28));
		toolbar.Add(&tbBtn3);
	}

	void OnDestroy() {
	}
};

int main(int argc, char** argv) {
	tui::Application app{ "Elemental GUI Demo", 640, 480 };
	return app.Start(new App());
}
