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

	CubeView() : GLView([] {
		GLContextConfig cfg;
		cfg.majorVersion = 1;
		cfg.minorVersion = 1;
		cfg.profile = GLContextConfig::Profile::Compatibility;
		return cfg;
	}()) {}

	void OnRender() override {
		float angleX = GetApp()->FindByTag<Slider>("rot_x")->GetValue() / 360.0f;
		float angleY = GetApp()->FindByTag<Slider>("rot_y")->GetValue() / 360.0f;
		float angleZ = GetApp()->FindByTag<Slider>("rot_z")->GetValue() / 360.0f;

		angleX = (angleX * 2.0f - 1.0f) * 180.0f;
		angleY = (angleY * 2.0f - 1.0f) * 180.0f;
		angleZ = (angleZ * 2.0f - 1.0f) * 180.0f;

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
		glRotatef(angleX, 1.0f, 0.0f, 0.0f);
		glRotatef(angleY, 0.0f, 1.0f, 0.0f);
		glRotatef(angleZ, 0.0f, 0.0f, 1.0f);

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
			.second = decl::Custom<CubeView, CubeViewProps>(CubeViewProps{
				.base = {
					.flexGrow = 1.0f,
					.bounds = Rectangle::FromSize(200, 200)
				}
			}),
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
