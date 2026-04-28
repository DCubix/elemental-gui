#pragma once

#include <Declarative.h>
#include <Element.h>
#include <Graphics.h>

namespace dc = gui::declarative;

class Canvas : public gui::Element {
public:
    Canvas();

    void OnDraw(gui::Graphics& g) override;
};

struct CanvasProps {
    dc::opt<dc::ElementProps> base;
};