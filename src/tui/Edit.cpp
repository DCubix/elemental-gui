#include "Edit.h"

#include <algorithm>
#include <map>
#include <tuple>
#include <iostream>
#include "Application.h"

namespace tui {

	Edit::Edit()
		: Element(),
		  m_multiLine(false),
		  m_caretX(0), m_caretY(0),
		  m_caretIndex(0),
		  m_selectionStart(-1), m_selectionEnd(0),
		  m_state(ESNormal)
	{ text("Edit"); localBounds().h = 22; }

	void Edit::onDraw(Graphics& g) {
		if (m_style.is_null()) {
			m_style = app()->style()["DefaultText"];
		}

		Rect b = bounds();
		Rect c = intersectedBounds();

		const std::string font = m_style.value("font", "Sans");
		const double fontSize = m_style.value("fontSize", 14.0);

		g.drawCharBegin(FontStyle::Normal, font, fontSize);

		const int lineHeight = g.measureText("E[ ").height;
		int maxW = 0, maxH = 0, lineH = 0;
		if (m_multiLine) {
			auto&& lines = utils::splitString(m_textRaw, "\n");
			for (auto&& text : lines) {
				auto&& ex = g.measureText(text);
				maxH += ex.height;
				lineH = std::max(lineH, int(ex.height));
				maxW = std::max(maxW, int(ex.width));
			}
		} else {
			auto&& ex = g.measureText(m_textRaw);
			maxW = ex.width;
			maxH = ex.height;
			lineH = maxH;
		}

		lineH = std::max(lineH, lineHeight);

		int sx = b.x + 4;
		int tx = sx;
		int ty = b.y + 4 + lineH;
		if (!m_multiLine) {
			ty = b.y + b.h / 2 + lineH / 2;
		}

		g.styledRect(b.x, b.y, b.w, b.h, app()->style()["Edit"]);

		m_charRects.clear();

		g.clipPush(c.x, c.y, c.w, c.h);
		int index = 0;
		for (Char c : m_text) {
			if (c.c == '\n' && m_multiLine) {
				int ptx = tx;
				int pty = ty;
				ty += lineH;
				tx = b.x + 4;

				CharRect cr;
				cr.index = index;
				cr.rect = Rect{ ptx, pty - lineH, (b.w - 8) - (ptx - sx), lineH };
				m_charRects.push_back(cr);
			} else {
				g.color(c.r, c.g, c.b);
				g.drawCharBegin(c.style, font, fontSize);

				int ptx = tx;

				CharRect cr;
				cr.index = index;
				cr.rect = Rect{ ptx, ty - lineH, 0, lineH };
				tx = g.drawChar(c.c, tx, ty);
				cr.rect.w = tx - ptx;
				m_charRects.push_back(cr);
			}

			index++;
		}

		if (m_text.empty()) {
			tx = sx;
			ty = b.y + 4;
			if (!m_multiLine) {
				ty = b.y + (b.h / 2 - lineH / 2);
			}
			g.color(0, 0, 0);
			g.lineWidth(1.0f);
			g.line(tx, ty, tx, ty + lineH + 1);
			g.stroke();
		} else {
			for (CharRect cr : m_charRects) {
				if (focused() && m_caretIndex == cr.index) {
					m_caretX = cr.rect.x;
					m_caretY = cr.rect.y;
					g.color(0, 0, 0);
					g.lineWidth(1.0f);
					g.line(m_caretX, m_caretY, m_caretX, m_caretY + lineH + 1);
					g.stroke();
					break;
				}
			}
		}

		// Draw selection
//		if (m_selectionStart != -1 && m_selectionEnd != -1) {
//			int a = m_selectionStart,
//				b = m_selectionEnd;
//			if (a > b) std::swap(a, b);

//			for (int i = a; i <= b; i++) {
//				Rect cr = m_charRects[i].rect;
//				g.styledRect(cr.x, cr.y, cr.w, cr.h, app()->style()["Selection"]);
//			}
//		}
		g.clipPop();
	}

	EventStatus Edit::onEvent(Event* event) {
		EventStatus status = Element::onEvent(event);
		if (event->type() == MouseEventType) {
			Rect b = intersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			switch (m_state) {
				case ESNormal: {
					if (e->pressed && e->button == 1) {
						for (CharRect cr : m_charRects) {
							Rect crr = cr.rect;
							crr.x -= crr.w/2;
							if (crr.hasPoint(e->x, e->y)) {
								m_caretIndex = cr.index;
								invalidate();
								break;
							}
						}
						m_state = ESSelecting;
						m_selectionStart = m_caretIndex;
						m_selectionEnd = -1;
						status = EventStatus::Consumed;
					}
				} break;
				case ESSelecting: {
					if (!e->pressed && e->button == 1) {
						std::cout << "S: " << m_selectionStart << ", E: " << m_selectionEnd << std::endl;
						m_state = ESNormal;
						invalidate();
					}
				} break;
			}
		} else if (event->type() == MotionEventType) {
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			switch (m_state) {
				default: break;
				case ESSelecting: {
					for (CharRect cr : m_charRects) {
						Rect crr = cr.rect;
						crr.x -= crr.w/2;
						if (crr.hasPoint(e->x, e->y)) {
							m_selectionEnd = cr.index;
							m_caretIndex = cr.index;
							break;
						}
					}
					invalidate();
				} break;
			}
		} else if (event->type() == TextInputEventType) {
			TextInput *e = dynamic_cast<TextInput*>(event);
			insertChar(e->inputChar);
			invalidate();
			status = EventStatus::Consumed;
		} else if (event->type() == KeyEventType) {
			KeyEvent *e = dynamic_cast<KeyEvent*>(event);
			if (e->pressed) {
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
					insertChar('\n');
				} else if (e->key == SDLK_HOME) {
					int loff = std::get<0>(lines[currLine]);
					m_caretIndex = loff;
				} else if (e->key == SDLK_END) {
					int loff = std::get<0>(lines[currLine]);
					int lsz = std::get<1>(lines[currLine]);
					m_caretIndex = loff + lsz-1;
				} else if (e->key == SDLK_BACKSPACE) {
					if (m_caretIndex-- > 0 && !m_text.empty()) {
						m_textRaw.erase(m_caretIndex, 1);
						m_text.erase(m_text.begin() + m_caretIndex);
					} else {
						m_caretIndex = 0;
					}
				} else if (e->key == SDLK_DELETE) {
					if (!m_text.empty()) {
						m_textRaw.erase(m_caretIndex, 1);
						m_text.erase(m_text.begin() + m_caretIndex);
						if (m_caretIndex >= m_textRaw.size()) {
							m_caretIndex = m_textRaw.size()-1;
						}
					} else {
						m_caretIndex = 0;
					}
				} else if (e->key == SDLK_LEFT) {
					m_caretIndex--;
					if (m_caretIndex <= 0) m_caretIndex = 0;
				} else if (e->key == SDLK_RIGHT) {
					m_caretIndex++;
					if (m_caretIndex >= m_text.size()) m_caretIndex = int(m_text.size()) - 1;
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
				}
				invalidate();
				status = EventStatus::Consumed;
			}
		} else if (event->type() == FocusEventType) {
			FocusEvent *e = dynamic_cast<FocusEvent*>(event);
			if (e->element == this) {
				app()->startInput();
				status = EventStatus::Consumed;
			}
		} else if (event->type() == BlurEventType) {
			BlurEvent *e = dynamic_cast<BlurEvent*>(event);
			if (e->element == this) {
				app()->stopInput();
				status = EventStatus::Consumed;
			}
		}
		return status;
	}

	void Edit::text(const std::string& txt) {
		std::string _txt = txt;
		if (_txt.back() != '\n') {
			_txt.push_back('\n');
		}
		m_text.clear();
		for (char c : _txt) {
			Char chr{};
			chr.c = c;
			m_text.push_back(chr);
		}
		m_textRaw = _txt;
		invalidate();
	}

	void Edit::format(int from, int len, FontStyle style, float r, float g, float b) {
		if (len < 0) len = m_text.size();
		from = std::max(from, 0);
		len = std::min(len, int(m_text.size()));

		for (int i = from; i < from+len; i++) {
			m_text[i].style = style;
			m_text[i].r = r;
			m_text[i].g = g;
			m_text[i].b = b;
		}
		invalidate();
	}

	void Edit::insertChar(char c) {
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
		}

		if (m_text.empty()) m_text.push_back(chr);
		else m_text.insert(m_text.begin() + m_caretIndex, chr);

		m_caretIndex++;
	}

	Edit::CharRect& Edit::findCharFromIndex(int index) {
		auto it = std::find_if(
					m_charRects.begin(),
					m_charRects.end(),
					[=](const CharRect& cr) {
						return cr.index == index;
					}
		);
		if (it != m_charRects.end()) {
			return *it;
		}
		return *m_charRects.begin();
	}

}
