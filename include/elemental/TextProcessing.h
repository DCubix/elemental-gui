#pragma once

#include "Graphics.h"
#include "Utils.h"

namespace gui::text {
    enum class EditState { Normal = 0, Selecting };

    struct Char {
        int index;
        Rectangle bounds;
        char value{'\0'};
        Color color{1.0f, 1.0f, 1.0f, 1.0f};
        FontStyle style{FontStyle::Normal};
    };

    struct Line {
        int index;
        Rectangle bounds;
        std::vector<Char> chars;
    };

    void ApplyTextStyle(Graphics& g, Json style);
    Size ComputeTextSize(Graphics& g, Json style, const std::string& text);
    std::vector<Line> ComputeLines(Graphics& g, Json style, const std::string& text);
    std::vector<PointI> BuildOrthoHull(const std::vector<Line>& lines);
} // namespace gui::text
