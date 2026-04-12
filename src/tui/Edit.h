#pragma once

#include "Element.h"

#include <vector>

namespace tui {
	using ChangeCallback = std::function<void()>;
	class Edit : public Element {
	public:
		Edit();

		void OnDraw(Graphics& g) override;
		EventStatus OnEvent(Event *event) override;
		Size GetPreferredSize() override;

		std::string GetText() const { return m_textRaw; }
		void SetText(const std::string& txt);

		bool IsMultiLine() const { return m_multiLine; }
		void SetMultiLine(bool m) { m_multiLine = m; Invalidate(); }

		bool IsMasked() const { return m_masked; }
		void SetMasked(bool m) { m_masked = m; Invalidate(); }

		bool IsEditable() const { return m_editable; }
		void SetEditable(bool m) { m_editable = m; Invalidate(); }

		void Format(int from, int len, FontStyle style, float r, float g, float b);
		void Format(FontStyle style, float r, float g, float b);

		void Select(int from, int len = -1);
		void Deselect();
		bool IsSelected();

		void SetOnChange(ChangeCallback cb) { m_onChange = cb; }

	private:
		enum class EditState {
			Normal = 0,
			Selecting
		};

		struct Char {
			char c = '\0';
			float r = 0.0f, g = 0.0f, b = 0.0f;
			FontStyle style = FontStyle::Normal;
		};
		struct CharRect {
			Rectangle rect;
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

		void InsertChar(char c);
		void RemoveChar(int i);
		void DeleteSelected();
		void UpdateOffset();

		std::vector<Point> BuildOrthoHull(const std::vector<CharRect>& crs);
		std::vector<Rectangle> BuildLinesAABB(const std::vector<CharRect>& crs);
	};
}
