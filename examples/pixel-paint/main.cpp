#include "MainWindow.h"

#include <Application.h>
#include <Declarative.h>
#include <EventSystem.h>
#include <FlexLayout.h>
#include <backends/sdl3/SDL3Backend.h>

int main() {
    gui::Application app{new gui::SDL3Backend()};
    app.CreateWindow<MainWindow>();
    return app.Start();
}