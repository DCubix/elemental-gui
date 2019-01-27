#ifndef TUI_EDIT_H
#define TUI_EDIT_H

#include "Element.h"

#include <vector>

namespace tui {
	using ChangeCallback = std::function<void()>;
	class Edit : public Element {
	public:
		Edit();

		void onDraw(Graphics& g) override;
		EventStatus onEvent(Event *event) override;
		Size preferredSize() override;

		std::string text() const { return m_textRaw; }
		void text(const std::string& txt);

		bool multiLine() const { return m_multiLine; }
		void multiLine(bool m) { m_multiLine = m; invalidate(); }

		bool masked() const { return m_masked; }
		void masked(bool m) { m_masked = m; invalidate(); }

		bool editable() const { return m_editable; }
		void editable(bool m) { m_editable = m; invalidate(); }

		void style(Json style) { m_style = style; invalidate(); }

		void format(int from, int len, FontStyle style, float r, float g, float b);
		void format(FontStyle style, float r, float g, float b);

		void select(int from, int len = -1);
		void deselect();
		bool isSelected();

		void onChange(ChangeCallback cb) { m_onChange = cb; }

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

		bool m_multiLine, m_masked, m_editable;
		int m_caretX, m_caretY, m_caretIndex;
		int m_selectionStart, m_selectionEnd;
		int m_offsetX, m_textWidth, m_textHeight;
		EditState m_state;

		ChangeCallback m_onChange;

		Json m_style;

		void insertChar(char c);
		void removeChar(int i);
		void deleteSelected();
		void updateOffset();

		std::vector<Point> buildOrthoHull(const std::vector<CharRect>& crs);
		std::vector<Rect> buildLinesAABB(const std::vector<CharRect>& crs);
	};
}

#endif // TUI_EDIT_H
