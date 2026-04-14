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
#include "tui/Declarative.h"
#include "tui/Menu.h"

#include <SDL3/SDL_opengl.h>
#include <cmath>

using namespace tui;
namespace decl = tui::declarative;

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

struct CubeViewProps {
	decl::ElementProps base{};
};

class App : public ApplicationAdapter {
public:
	void OnCreate(Application& app) {
		auto gui = decl::Column({
			.gap = 8,
			.padding = EdgeInsets::All(8),
			.align = FlexAlign::Center,
			.justify = FlexJustify::Center,
			.showBackground = true
		}, {
			decl::Button({
				.base = {
					.tag = "menuBtn",
					.autoSize = true
				},
				.text = "Open Menu",
				.onClick = [&app]() {
					auto* menu = app.FindByTag<tui::Menu>("contextMenu");
					if (menu && !menu->IsOpen()) {
						auto* btn = app.FindByTag<Button>("menuBtn");
						auto bounds = btn->GetBounds();
						app.ShowPopup(menu);
						menu->Show(bounds.x, bounds.y + bounds.h + 2);
					}
				}
			}),
			decl::Text("Elemental GUI Demo", {
				.base = {
					.tag = "title",
					.autoSize = true
				},
				.align = Alignment::MiddleCenter
			}),
			decl::Button({
				.base = {
					.autoSize = true
				},
				.text = "Click Me",
				.onClick = [&app]() {
					app.FindByTag<Label>("title")->SetText("Button Clicked!");
				}
			}),
			decl::Custom<CubeView, CubeViewProps>(CubeViewProps{
				.base = {
					.bounds = Rectangle(0, 0, 200, 200)
				}
			}),
			decl::Slider({
				.base = {
					.tag = "scale",
					.bounds = Rectangle(0, 0, 200, 20)
				},
				.direction = Direction::Horizontal,
				.range = { 0, 100 },
				.value = 50,
				.step = 1
			}),
			decl::Switch({
				.base = {
					.tag = "switch",
					.autoSize = true
				},
				.checked = false,
				.onChanged = [](bool checked) {}
			}),
			decl::CheckBox({
				.base = {
					.tag = "checkbox",
					.autoSize = true
				},
				.text = "Check me",
				.checked = false,
				.onChanged = [](bool checked) {}
			})
		});
		app.SetRoot(gui(app));

		// --- Context Menu ---
		auto ctxMenu = decl::Menu({
			.base = { .tag = "contextMenu" },
			.onDismiss = [&app]() {
				auto* menu = app.FindByTag<tui::Menu>("contextMenu");
				app.DismissPopup(menu);
			}
		}, {
			decl::MenuItem({ .text = "Cut", .onClick = [&app]() {
				app.FindByTag<Label>("title")->SetText("Cut!");
			}}),
			decl::MenuItem({ .text = "Copy", .onClick = [&app]() {
				app.FindByTag<Label>("title")->SetText("Copy!");
			}}),
			decl::MenuItem({ .text = "Paste", .onClick = [&app]() {
				app.FindByTag<Label>("title")->SetText("Paste!");
			}}),
			decl::MenuSeparator(),
			decl::MenuItem({ .text = "Select All", .onClick = [&app]() {
				app.FindByTag<Label>("title")->SetText("Select All!");
			}}),
			decl::MenuSeparator(),
			decl::MenuItem({
				.text = "Sub Menu",
				.onClick = [&app]() {},
				.subMenu = decl::Menu({
					.onDismiss = []() {}
				}, {
					decl::MenuItem({ .text = "Option 1", .onClick = [&app]() {
						app.FindByTag<Label>("title")->SetText("Option 1!");
					}}),
					decl::MenuItem({ .text = "Option 2", .onClick = [&app]() {
						app.FindByTag<Label>("title")->SetText("Option 2!");
					}})
				})
			})
		});
		ctxMenu(app);
	}

	void OnDestroy() {
	}
};

int main(int argc, char** argv) {
	tui::Application app{ "Elemental GUI Demo", 640, 480 };
	return app.Start(new App());
}
