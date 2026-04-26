#include "TextProcessing.h"

namespace gui::text {
    void ApplyTextStyle(Graphics& g, Json style) {
        const std::string font = style.value("font", "Sans");
        const double fontSize = style.value("fontSize", 14.0);
        FontStyle fontStyle = FontStyle::Normal;

        const std::string fontStyleStr = style.value("fontStyle", "normal");
        if (fontStyleStr == "bold")
            fontStyle = FontStyle::Bold;
        else if (fontStyleStr == "italic")
            fontStyle = FontStyle::Italic;
        else if (fontStyleStr == "bolditalic")
            fontStyle = FontStyle::BoldItalic;

        g.Font(fontStyle, font, fontSize);
    }

    Size ComputeTextSize(Graphics& g, Json style, const std::string& text) {
        ApplyTextStyle(g, style);

        bool isMultiLine = text.find('\n') != std::string::npos;

        int maxW = 0, maxH = 0;
        if (isMultiLine) {
            auto lines = utils::SplitString(text, "\n");
            for (auto&& text : lines) {
                auto ex = g.MeasureText(text);
                maxH += ex.size.h;
                maxW = std::max(maxW, int(ex.size.w));
            }
        } else {
            auto ex = g.MeasureText(text);
            maxW = ex.size.w;
            maxH = ex.size.h;
        }

        return Size(maxW, maxH);
    }

    static std::vector<Char>
    ComputeChars(Graphics& g, int y, const std::string& text, int lineHeight, int indentSize) {
        std::vector<Char> out;

        const int spaceWidth = static_cast<int>(g.MeasureText(" ").size.w);

        int cx = 0;
        for (size_t i = 0; i < text.size(); i++) {
            int prevCx = cx;
            char c = text[i];

            if (c == '\r')
                continue;

            auto ex = g.MeasureText(std::string(1, c));
            Rectangle rect{cx, y, 0, lineHeight};

            if (c == '\t')
                cx += spaceWidth * indentSize;
            else
                cx += static_cast<int>(ex.xAdvance);

            rect.w = (cx - prevCx) + 1;
            out.push_back(Char{static_cast<int>(i), rect, c});
        }

        Rectangle lastRect{cx, y, 9999, lineHeight};
        out.push_back(Char{static_cast<int>(text.size()), lastRect, '\0'});

        return out;
    }

    std::vector<Line> ComputeLines(Graphics& g, Json style, const std::string& text) {
        ApplyTextStyle(g, style);

        const int lineHeight = static_cast<int>(g.GetFontExtents().height);
        const int indentSize = style.value("indentSize", 4);

        bool isMultiLine = text.find('\n') != std::string::npos;
        if (!isMultiLine) {
            auto ex = g.MeasureText(text);
            return {Line{
                0,
                Rectangle{0, 0, ex.size.w, ex.size.h},
                ComputeChars(g, 0, text, lineHeight, indentSize)
            }};
        }

        auto lines = utils::SplitString(text, "\n");
        std::vector<Line> lineRects;
        int y = 0;
        for (size_t i = 0; i < lines.size(); ++i) {
            auto ex = g.MeasureText(lines[i]);
            lineRects.push_back(Line{
                static_cast<int>(i),
                Rectangle{0, y, ex.size.w, lineHeight},
                ComputeChars(g, y, lines[i], lineHeight, indentSize)
            });
            y += lineHeight;
        }

        return lineRects;
    }

    std::vector<PointI> BuildOrthoHull(const std::vector<Line>& lines) {
        std::vector<PointI> out;

        if (lines.empty())
            return out;

        std::vector<Rectangle> lineRects;
        lineRects.reserve(lines.size());
        for (const Line& line : lines) {
            lineRects.push_back(line.bounds);
        }

        // 1. Add the first 3 points from the first line
        out.push_back({lineRects[0].x, lineRects[0].y});                                   // 1st
        out.push_back({lineRects[0].x + lineRects[0].w, lineRects[0].y});                  // 2nd
        out.push_back({lineRects[0].x + lineRects[0].w, lineRects[0].y + lineRects[0].h}); // 3rd

        // 2. While theres any other lines, add the
        //    second and the third points, and push the lines
        //    to a stack, so we add the first and fourth points
        //    later

        if (lineRects.size() > 1) {
            std::stack<PointI> points;
            for (int i = 1; i < lineRects.size(); i++) {
                Rectangle lr = lineRects[i];
                out.push_back({lr.x + lr.w, lr.y});        // 2nd
                out.push_back({lr.x + lr.w, lr.y + lr.h}); // 3rd

                // Push the points to be added later
                points.push({lr.x, lr.y});        // 1st
                points.push({lr.x, lr.y + lr.h}); // 4th
            }

            // Push the rest of the points
            while (!points.empty()) {
                PointI pt = points.top();
                points.pop();
                out.push_back(pt);
            }
        }

        // 3. Add the 4th point of the first line
        out.push_back({lineRects[0].x, lineRects[0].y + lineRects[0].h}); // 4th

        return out;
    }
} // namespace gui::text
