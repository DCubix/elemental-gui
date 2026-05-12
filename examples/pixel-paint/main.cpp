#include "MainWindow.h"

#include <elemental/Application.h>
#include <elemental/Declarative.h>
#include <elemental/EventSystem.h>
#include <elemental/FlexLayout.h>
#include <elemental/backends/sdl3/SDL3Backend.h>

int main() {
    gui::Application app{new gui::SDL3Backend()};
    app.CreateWindow<MainWindow>();
    return app.Start();
}