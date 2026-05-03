#pragma once

#include <Declarative.h>
#include <FlexLayout.h>
#include <Window.h>

namespace dc = gui::declarative;

class MainWindow : public gui::Window {
public:
    MainWindow();

    dc::WidgetDesc OnBuild() override;
    void OnResize() override;
    void OnCreate() override;

    void RepositionCanvas();

    enum Icons {
        icPencil = 0,
        icEraser,
        icEyedrop,
        icCurve,
        icSquare,
        icCircle,
        icFill,
        icPalette,
        icSwap,
        icAdd,
        icRemove,
        icTrash,
        icNewLayer,
        icUp,
        icDown,
        icCount
    };

    gui::Image icons[icCount];
    bool saved{true};
    std::string fileName;
};