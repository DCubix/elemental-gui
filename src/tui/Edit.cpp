#include "Edit.h"

#include <algorithm>
#include <map>
#include <tuple>
#include <iostream>
#include "Application.h"

namespace tui {

	Edit::Edit()
		: Element(),
		  m_multiLine(false), m_masked(false), m_editable(true),
		  m_caretX(0), m_caretY(0),
		  m_caretIndex(0),
		  m_selectionStart(-1), m_selectionEnd(0),
		  m_offsetX(0),
		  m_state(EditState::Normal)
	{ SetText("Edit"); SetLocalBounds(Rectangle(0, 0, 50, 22)); }

	void Edit::OnDraw(Graphics& g) {
		const auto textStyle = GetStyle()["DefaultText"];

		Rectangle b = GetBounds();
		Rectangle c = GetIntersectedBounds();

		const std::string font = textStyle.value("font", "Sans");
		const double fontSize = textStyle.value("fontSize", 14.0);

		g.DrawCharBegin(FontStyle::Normal, font, fontSize);

		std::string text = m_textRaw;

		const int lineHeight = g.MeasureText("E[ ").height;
		const int spaceSize = g.MeasureText(" ").width;
		int maxW = 0, maxH = 0, lineH = 0;
		if (m_multiLine) {
			auto&& lines = utils::SplitString(text, "\n");
			for (auto&& text : lines) {
				auto&& ex = g.MeasureText(text);
				maxH += ex.height;
				lineH = std::max(lineH, int(ex.height));
				maxW = std::max(maxW, int(ex.width));
			}
		} else {
			auto&& ex = g.MeasureText(text);
			maxW = ex.width;
			maxH = ex.height;
			lineH = maxH;
		}

		lineH = std::max(lineH, lineHeight);

		m_textWidth = maxW + 25;
		m_textHeight = maxH + 100;

		int sx = b.x + 4;
		int tx = sx + m_offsetX;
		int ty = b.y + 4 + lineH;
		if (!m_multiLine) {
			ty = b.y + b.h / 2 + lineH / 2;
		}

		auto editStyle = GetStyle()["Edit"];
		g.StyledRect(b.x, b.y, b.w, b.h, editStyle);

		float textR = 0.88f, textG = 0.88f, textB = 0.88f;
		if (textStyle.contains("color") && textStyle["color"].is_array() && textStyle["color"].size() >= 3) {
			textR = textStyle["color"][0].get<float>();
			textG = textStyle["color"][1].get<float>();
			textB = textStyle["color"][2].get<float>();
		}

		m_charRects.clear();

		g.ClipPushRect(c.x+4, c.y+4, c.w-8, c.h-8);
		int index = 0;
		for (Char c : m_text) {
			if (c.c == '\n') {
				if (m_multiLine) {
					int ptx = tx;
					int pty = ty;
					ty += lineH;
					tx = b.x + 4 + m_offsetX;

					CharRect cr;
					cr.index = index;
					cr.rect = Rectangle{ ptx, pty - lineH, (b.w - 8) - (ptx - sx), lineH };
					m_charRects.push_back(cr);
				} else { continue; }
			} else if (c.c == '\r') {
				continue;
			} else {
				g.Color(c.r, c.g, c.b);
				g.DrawCharBegin(c.style, font, fontSize);

				int ptx = tx;

				CharRect cr;
				cr.index = index;
				cr.rect = Rectangle{ ptx, ty - lineH, 0, lineH };

				char chr = c.c;
				if (chr == '\t') {
					tx = g.DrawChar(' ', tx, ty);
					tx = g.DrawChar(' ', tx, ty);
					tx = g.DrawChar(' ', tx, ty);
					tx = g.DrawChar(' ', tx, ty);
				} else {
					tx = g.DrawChar(m_masked ? '*' : chr, tx, ty);
				}
				cr.rect.w = (tx - ptx) + 1;
				m_charRects.push_back(cr);
			}

			index++;
		}

		CharRect cr;
		cr.index = index;
		cr.rect = Rectangle{ tx, ty - lineH, (b.w - 8) - (tx - sx), lineH };
		m_charRects.push_back(cr);

		if (m_editable) {
			if (m_text.empty()) {
				tx = sx;
				ty = b.y + 4;
				if (!m_multiLine) {
					ty = b.y + (b.h / 2 - lineH / 2);
				}
				g.Color(textR, textG, textB);
				g.LineWidth(1.0f);
				g.Line(tx, ty, tx, ty + lineH + 1);
				g.Stroke();

				m_caretX = tx;
				m_caretY = ty;
				if (!IsAutoSize())
					UpdateOffset();
				else m_offsetX = 0;
			} else {
				if (IsFocused()) {
					CharRect cr = m_charRects[m_caretIndex];
					m_caretX = cr.rect.x;
					m_caretY = cr.rect.y;
					if (!IsAutoSize())
						UpdateOffset();
					else m_offsetX = 0;
					g.Color(textR, textG, textB);
					g.LineWidth(1.0f);
					g.Line(m_caretX, m_caretY, m_caretX, m_caretY + lineH + 1);
					g.Stroke();
				}
			}
		}

		// Draw selection
		if (IsSelected()) {
			int a = m_selectionStart,
				b = m_selectionEnd;
			if (a > b) std::swap(a, b);

			int len = b-a;
			g.BeginPath();

			std::vector<CharRect> rects;
			rects.reserve(len);
			for (int i = a; i < b; i++) {
				CharRect r = m_charRects[i];
				rects.push_back(r);
			}

			std::vector<PointI> pts = BuildOrthoHull(rects);
			for (PointI& p : pts) {
				g.AddPathPoint(p.x, p.y);
			}

			g.EndPath(true);
			g.StyledPaint(GetStyle()["Selection"]);
		}

//		for (int i = 0; i < m_charRects.size(); i++) {
//			CharRect cre = m_charRects[i];
//			Rect cr = cre.rect;
//			if (i < m_charRects.size() - 1)
//				cr.x -= cr.w/2;
//			g.styledRect(cr.x, cr.y, cr.w, cr.h, app()->style()["Selection"]);
//		}

		g.ClipPop();
	}

	EventStatus Edit::OnEvent(Event* event) {
		EventStatus status = Element::OnEvent(event);
		if (event->Type() == EventType::MouseButton) {
			Rectangle b = GetIntersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			switch (m_state) {
				case EditState::Normal: {
					if (IsFocused() &&
						e->pressed && e->button == 1 &&
						m_editable &&
						b.HasPoint(e->x, e->y))
					{
						for (CharRect cr : m_charRects) {
							Rectangle crr = cr.rect;
							if (m_text.size() > 1 && cr.index < m_text.size() - 1)
								crr.x -= crr.w/2;
							if (crr.HasPoint(e->x, e->y)) {
								m_caretIndex = cr.index;
								Invalidate();
								break;
							}
						}
						m_state = EditState::Selecting;
						m_selectionStart = m_caretIndex;
						m_selectionEnd = -1;
						status = EventStatus::Consumed;
					}
				} break;
				case EditState::Selecting: {
					if (!e->pressed && e->button == 1) {
						m_state = EditState::Normal;
						Invalidate();
					}
				} break;
			}
		} else if (event->Type() == EventType::MouseMotion) {
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			switch (m_state) {
				default: break;
				case EditState::Selecting: {
					for (CharRect cr : m_charRects) {
						Rectangle crr = cr.rect;
						if (m_text.size() > 1 && cr.index < m_text.size() - 1)
							crr.x -= crr.w/2;
						if (crr.HasPoint(e->x, e->y)) {
							m_selectionEnd = cr.index;
							m_caretIndex = cr.index;
							break;
						} else { continue; }
					}
					Invalidate();
				} break;
			}
		} else if (event->Type() == EventType::TextInput) {
			TextInput *e = dynamic_cast<TextInput*>(event);
			if (IsFocused() && m_editable) {
				InsertChar(e->inputChar);
				Invalidate();
				status = EventStatus::Consumed;
			}
		} else if (event->Type() == EventType::Key) {
			KeyEvent *e = dynamic_cast<KeyEvent*>(event);
			if (e->pressed && IsFocused() && m_editable) {
				// Map out the lines start offset and length.
				// Since the text is just a linear
				// array, we just check for line breaks (\n)
				std::map<int, std::tuple<int, int>> lines;
				int curr = 0, currChar = 0, idx = 0, line = 0, currLine = 0;
				for (auto&& c : m_text) {
					curr++;
					if (idx == m_caretIndex) {
						currLine = line;
					}
					idx++;
					if (c.c == '\n') {
						lines[line] = std::make_tuple(currChar, curr);
						currChar += curr;
						curr = 0;
						line++;
					}
				}

				if ((e->key == SDLK_RETURN || e->key == SDLK_KP_ENTER) && m_multiLine) {
					InsertChar('\n');
				} else if (e->key == SDLK_HOME) {
					int loff = std::get<0>(lines[currLine]);
					m_caretIndex = loff;
				} else if (e->key == SDLK_END) {
					int loff = std::get<0>(lines[currLine]);
					int lsz = std::get<1>(lines[currLine]);
					m_caretIndex = loff + lsz-1;
				} else if (e->key == SDLK_BACKSPACE) {
					if (IsSelected()) {
						DeleteSelected();
					} else {
						if (m_caretIndex-- > 0 && !m_text.empty()) {
							RemoveChar(m_caretIndex);
						} else {
							m_caretIndex = 0;
						}
					}
				} else if (e->key == SDLK_DELETE) {
					if (IsSelected()) {
						DeleteSelected();
					} else {
						if (!m_text.empty()) {
							RemoveChar(m_caretIndex);
							if (m_caretIndex >= m_textRaw.size()) {
								m_caretIndex = m_textRaw.size()-1;
							}
						} else {
							m_caretIndex = 0;
						}
					}
				} else if (e->key == SDLK_LEFT) {
					m_caretIndex--;
					if (m_caretIndex <= 0) m_caretIndex = 0;
				} else if (e->key == SDLK_RIGHT) {
					m_caretIndex++;
					if (m_caretIndex >= m_text.size()) m_caretIndex = int(m_text.size());
				} else if (e->key == SDLK_UP && m_multiLine) {
					int loff = std::get<0>(lines[currLine]);
					int dist = m_caretIndex - loff;

					if (--currLine < 0) currLine = 0;
					loff = std::get<0>(lines[currLine]);

					int off = loff + dist;
					int lsz = std::get<1>(lines[currLine]);
					if (off >= loff + lsz) {
						off = loff + lsz-1;
					}
					m_caretIndex = off;
				} else if (e->key == SDLK_DOWN && m_multiLine) {
					int loff = std::get<0>(lines[currLine]);
					int dist = m_caretIndex - loff;

					if (++currLine >= lines.size()) currLine = lines.size()-1;
					loff = std::get<0>(lines[currLine]);

					int off = loff + dist;
					int lsz = std::get<1>(lines[currLine]);
					if (off >= loff + lsz) {
						off = loff + lsz-1;
					}
					m_caretIndex = off;
				} else if (e->key == SDLK_C && GetApp()->GetMod() & SDL_KMOD_CTRL && IsSelected()) {
					int a = m_selectionStart,
						b = m_selectionEnd;
					if (a > b) std::swap(a, b);
					std::string selTxt = m_textRaw.substr(a, b - a);
					GetApp()->SetClipboard(selTxt);
				} else if (e->key == SDLK_X && GetApp()->GetMod() & SDL_KMOD_CTRL && IsSelected()) {
					int a = m_selectionStart,
						b = m_selectionEnd;
					if (a > b) std::swap(a, b);
					std::string selTxt = m_textRaw.substr(a, b - a);
					GetApp()->SetClipboard(selTxt);
					DeleteSelected();
				} else if (e->key == SDLK_V && GetApp()->GetMod() & SDL_KMOD_CTRL) {
					if (IsSelected()) {
						DeleteSelected();
					}
					std::string ntxt = GetApp()->GetClipboard();
					for (char c : ntxt) {
						InsertChar(c);
					}
				} else if (e->key == SDLK_A && GetApp()->GetMod() & SDL_KMOD_CTRL) {
					Select(0);
				}
				Invalidate();
				status = EventStatus::Consumed;
			} else if (!e->pressed && IsFocused()) {
				Invalidate();
			}
		} else if (event->Type() == EventType::Focus) {
			FocusEvent *e = dynamic_cast<FocusEvent*>(event);
			if (e->element == this) {
				GetApp()->StartInput();
				status = EventStatus::Consumed;
			}
		} else if (event->Type() == EventType::Blur) {
			BlurEvent *e = dynamic_cast<BlurEvent*>(event);
			if (e->element == this) {
				GetApp()->StopInput();
				status = EventStatus::Consumed;
			}
		}
		return status;
	}

	Size Edit::GetPreferredSize() const {
		return { m_textWidth, m_textHeight };
	}

	void Edit::SetText(const std::string& txt) {
		std::string _txt = txt;
		m_caretIndex = 0;
		m_text.clear();
		m_textRaw.clear();
		for (char c : _txt) {
			InsertChar(c);
		}
		Invalidate();
	}

	void Edit::Format(int from, int len, FontStyle style, float r, float g, float b) {
		if (len < 0) len = m_text.size();
		from = std::max(from, 0);
		len = std::min(len, int(m_text.size()) - from);
		if (len <= 0) return;

		for (int i = from; i < from+len; i++) {
			m_text[i].style = style;
			m_text[i].r = r;
			m_text[i].g = g;
			m_text[i].b = b;
		}
		Invalidate();
	}

	void Edit::Format(FontStyle style, float r, float g, float b) {
		if (!IsSelected())
			return;
		int from = m_selectionStart;
		int len = m_selectionEnd - from;
		Format(from, len, style, r, g, b);
	}

	void Edit::Select(int from, int len) {
		if (len < 0) len = m_text.size();
		m_selectionStart = from;
		m_selectionEnd = from + len;
		Invalidate();
	}

	void Edit::Deselect() {
		m_selectionStart = -1;
		m_selectionEnd = -1;
		Invalidate();
	}

	bool Edit::IsSelected() {
		return (m_selectionStart != -1 && m_selectionEnd != -1 &&
			m_selectionStart != m_selectionEnd);
	}

	void Edit::InsertChar(char c) {
		if (c == '\r') return;

		if (m_caretIndex > m_textRaw.size()) m_caretIndex = m_textRaw.size();
		else if (m_caretIndex < 0) m_caretIndex = 0;

		if (m_textRaw.empty()) m_textRaw.push_back(c);
		else m_textRaw.insert(m_caretIndex, 1, c);

		Char chr{};
		chr.c = c;
		if (m_caretIndex > 0) { // Inherit styling from previous character
			Char prev = m_text[m_caretIndex - 1];
			chr.r = prev.r;
			chr.g = prev.g;
			chr.b = prev.b;
			chr.style = prev.style;
		} else {
			auto style = GetStyle()["DefaultText"];
			if (style.contains("color") && style["color"].is_array() && style["color"].size() >= 3) {
				chr.r = style["color"][0].get<float>();
				chr.g = style["color"][1].get<float>();
				chr.b = style["color"][2].get<float>();
			}
		}

		if (m_text.empty()) m_text.push_back(chr);
		else m_text.insert(m_text.begin() + m_caretIndex, chr);

		m_caretIndex++;
		if (m_onChange) m_onChange(m_textRaw);
	}

	void Edit::RemoveChar(int i) {
		m_textRaw.erase(i, 1);
		m_text.erase(m_text.begin() + i);
		if (m_onChange) m_onChange(m_textRaw);
	}

	void Edit::DeleteSelected() {
		int a = m_selectionStart,
			b = m_selectionEnd;
		if (a > b) std::swap(a, b);
		for (int i = a; i < b; i++) {
			RemoveChar(a);
		}
		m_caretIndex = a;
		Deselect();
	}

	void Edit::UpdateOffset() {
		Rectangle b = GetBounds();
		int cx = m_caretX - (b.x + 4);
		int max = b.w - 10;
		if (cx > max-1) {
			int off = cx - max;
			m_offsetX -= off;
			if (m_offsetX > 0) m_offsetX = 0;
		} else if (cx < 0) {
			m_offsetX += -cx;
			if (m_offsetX > 0) m_offsetX = 0;
		}
	}

	std::vector<PointI> Edit::BuildOrthoHull(const std::vector<CharRect>& crs) {
		std::vector<Rectangle> lines = BuildLinesAABB(crs);
		std::vector<PointI> out;

		if (lines.empty()) return out;

		// 1. Add the first 3 points from the first line
		out.push_back({ lines[0].x, lines[0].y }); // 1st
		out.push_back({ lines[0].x + lines[0].w, lines[0].y }); // 2nd
		out.push_back({ lines[0].x + lines[0].w, lines[0].y + lines[0].h }); // 3rd

		// 2. While theres any other lines, add the
		//    second and the third points, and push the lines
		//    to a stack, so we add the first and fourth points
		//    later

		if (lines.size() > 1) {
			std::stack<PointI> points;
			for (int i = 1; i < lines.size(); i++) {
				Rectangle lr = lines[i];
				out.push_back({ lr.x + lr.w, lr.y }); // 2nd
				out.push_back({ lr.x + lr.w, lr.y + lr.h }); // 3rd

				// Push the points to be added later
				points.push({ lr.x, lr.y }); // 1st
				points.push({ lr.x, lr.y + lr.h }); // 4th
			}

			// Push the rest of the points
			while (!points.empty()) {
				PointI pt = points.top();
				points.pop();
				out.push_back(pt);
			}
		}

		// 3. Add the 4th point of the first line
		out.push_back({ lines[0].x, lines[0].y + lines[0].h }); // 4th

		return out;
	}

	std::vector<Rectangle> Edit::BuildLinesAABB(const std::vector<CharRect>& crs) {
		std::vector<Rectangle> out;
		Rectangle rect{};

		bool newRect = true;
		for (CharRect cr : crs) {
			if (m_text[cr.index].c == '\n') {
				out.push_back(rect);
				newRect = true;
			} else {
				if (newRect) {
					rect = Rectangle(cr.rect.x, cr.rect.y+1, 0, 0);
					newRect = false;
				}
				rect.w = std::max(rect.w, (cr.rect.x + cr.rect.w) - rect.x);
				rect.h = std::max(rect.h, cr.rect.h);
			}
		}
		if (rect.w > 0 && rect.h > 0)
			out.push_back(rect);

		return out;
	}

}
