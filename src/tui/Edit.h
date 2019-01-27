#ifndef TUI_EDIT_H
#define TUI_EDIT_H

#include "Element.h"

#include <vector>

namespace tui {
	class Edit : public Element {
	public:
		Edit();

		void onDraw(Graphics& g) override;
		EventStatus onEvent(Event *event) override;

		std::string text() const { return m_textRaw; }
		void text(const std::string& txt);

		bool multiLine() const { return m_multiLine; }
		void multiLine(bool m) { m_multiLine = m; invalidate(); }

		void format(int from, int len, FontStyle style, float r, float g, float b);
		void format(FontStyle style, float r, float g, float b);
		void select(int from, int len = -1);

	private:
		enum EditState {
			ESNormal = 0,
			ESSelecting
		};

		struct Char {
			char c = '\0';
			float r = 0.0f, g = 0.0f, b = 0.0f;
			FontStyle style = Normal;
		};
		struct CharRect {
			Rect rect;
			int index;
		};

		std::string m_textRaw;
		std::vector<Char> m_text;
		std::vector<CharRect> m_charRects;

		bool m_multiLine;
		int m_caretX, m_caretY, m_caretIndex;
		int m_selectionStart, m_selectionEnd;
		EditState m_state;

		Json m_style;

		void insertChar(char c);
		CharRect& findCharFromIndex(int index);

		std::vector<Point> buildOrthoHull(const std::vector<CharRect>& crs);
		std::vector<Rect> buildLinesAABB(const std::vector<CharRect>& crs);
	};
}

#endif // TUI_EDIT_H
