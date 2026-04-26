#pragma once
#include "Edit.h"

namespace gui {
    class TextArea : public Edit {
      public:
        TextArea();
        void OnCreate() override;
        void OnDraw(Graphics& g) override;
        void OnKeyDown(KeyEvent e) override;
        Size GetPreferredSize() const override;

        void Format(int from, int len, FontStyle style, float r, float g, float b);
        void Format(FontStyle style, float r, float g, float b);

      protected:
        struct CharFormat {
            Color color{0.88f, 0.88f, 0.88f, 1.0f};
            FontStyle fontStyle{FontStyle::Normal};
        };
        std::vector<text::Line> m_lines;
        std::vector<CharFormat> m_formats;

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
    };
} // namespace gui