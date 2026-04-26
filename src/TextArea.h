#pragma once
#include "Edit.h"

namespace gui {
    class TextArea : public Edit {
    public:
        TextArea();
        void OnCreate() override;
        void OnDraw(Graphics& g) override;
        void OnMouseDown(MouseEvent e) override;
        void OnMouseMove(MotionEvent e) override;
        void OnKeyDown(KeyEvent e) override;
        Size GetPreferredSize() const override;

        void Format(int from, int len, FontStyle style, float r, float g, float b);
        void Format(FontStyle style, float r, float g, float b);

    protected:
        std::vector<text::Line> m_lines;
        int m_lineHeight{0};

        void InsertChar(char c) override;
        void RemoveChar(int i) override;
        void DeleteSelected() override;
        void Rebuild() override;

    private:
        struct LineInfo {
            int lineIdx;
            int lineStart;
            int lineLen;
        };
        std::vector<int> ComputeLineStarts() const;
        LineInfo GetCaretLineInfo() const;
        int HitTestIndex(int localX, int localY) const;
    };
} // namespace gui