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
		ScrollView& sv = app->Create<ScrollView>(Layout::Center);
		ImageView& img = app->Create<ImageView>();
		bg = new Image("bg.png");
		img.SetImage(bg);
		sv.SetElement(&img);
	}

	void OnDestroy() {
		if (icon) delete icon;
		if (bg) delete bg;
	}

	Image *icon = nullptr, *bg = nullptr;
};

int main(int argc, char** argv) {
	tui::Application app{ "Elemental", 320, 320 };
	return app.Start(new App());
}
