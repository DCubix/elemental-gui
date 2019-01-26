#include "tui/Application.h"
#include "tui/Panel.h"
#include "tui/Label.h"
#include "tui/Button.h"
#include "tui/Slider.h"
#include "tui/Edit.h"

using namespace tui;

class App : public ApplicationAdapter {
public:
	void onCreate(Application *app) {
		Panel *mainPanel = app->create<Panel>(Layout::Center);
		Label *lblTest = app->create<Label>();
		lblTest->text(R"(Hello World!
This is a test, which shows you the text
rendering capabilities of Cairo.)");
		lblTest->alignment((Alignment) align);

		icon = new Image("music.png");
		lblTest->icon(icon);

		mainPanel->add(lblTest, Layout::Center);

		Button *btButton = app->create<Button>();
		btButton->onClick([=](){
			align += 1;
			align %= Alignment::AlignmentCount;
			lblTest->alignment((Alignment) align);
		});
		mainPanel->add(btButton, Layout::Top);

//		Slider *slSlider = app->create<Slider>();
//		slSlider->step(0.1f);
//		slSlider->range().maximum = 1.0f;
//		mainPanel->add(slSlider, Layout::Bottom);

		Edit *edt = app->create<Edit>();
		edt->localBounds(Rect(0, 0, 120, 60));
		edt->text("Hello Wrld!\nMultiline!");
		edt->multiLine(true);
		edt->format(6, 6, FontStyle::Bold, 1, 0, 0);
		mainPanel->add(edt, Layout::Bottom);

		Slider *slSlider2 = app->create<Slider>();
		slSlider2->localBounds(Rect(0, 0, 22, 120));
		slSlider2->orientation(Slider::Vertical);
		slSlider2->step(0.1f);
		slSlider2->range(0.0f, 1.0f);
		mainPanel->add(slSlider2, Layout::Right);
	}

	void onDestroy() {
		delete icon;
	}

	Image* icon;
	int align = Alignment::TopLeft;
};

int main(int argc, char** argv) {
	tui::Application app{ "TUI Test", 320, 320 };
	return app.start(new App());
}
