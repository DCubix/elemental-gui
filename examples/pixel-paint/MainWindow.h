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
        icCount
    };

    gui::Image icons[icCount];
};