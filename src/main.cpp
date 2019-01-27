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
	void onCreate(Application *app) {
		ScrollView *sv = app->create<ScrollView>(Layout::Center);
		ImageView *img = app->create<ImageView>();
		bg = new Image("bg.png");
		img->image(bg);
		sv->element(img);
	}

	void onDestroy() {
		if (icon) delete icon;
		if (bg) delete bg;
	}

	Image *icon = nullptr, *bg = nullptr;
};

int main(int argc, char** argv) {
	tui::Application app{ "Elemental", 320, 320 };
	return app.start(new App());
}
