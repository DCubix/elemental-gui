#include "Graphics.h"

#include <cctype>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace tui {

	Graphics::Graphics(SDL_Renderer* ren)
		: m_renderer(ren), m_buffer(nullptr), m_surface(nullptr), m_context(nullptr)
	{
		m_measureSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
		m_measureContext = cairo_create(m_measureSurface);
	}

    void Graphics::SetViewport(int w, int h) {
		if (m_buffer != nullptr) {
			SDL_DestroyTexture(m_buffer);
		}
		m_buffer = SDL_CreateTexture(
					m_renderer,
					SDL_PIXELFORMAT_ARGB8888,
					SDL_TEXTUREACCESS_STREAMING,
					w, h
		);
		SDL_SetTextureScaleMode(m_buffer, SDL_SCALEMODE_NEAREST);
		m_width = w;
		m_height = h;
	}

	void Graphics::Clear(int r, int g, int b) {
		SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
		SDL_RenderClear(m_renderer);
	}

	void Graphics::LineWidth(float w) {
		cairo_set_line_width(m_context, w);
	}

	void Graphics::Color(float r, float g, float b, float a) {
		cairo_set_source_rgba(m_context, r, g, b, a);
	}

	void Graphics::Rect(int x, int y, int w, int h) {
		cairo_rectangle(m_context, double(x) + 0.5, double(y) + 0.5, w, h);
	}

	void Graphics::RoundRect(int x, int y, int w, int h, float radius) {
		if (radius <= 0) {
			Rect(x, y, w, h);
			return;
		}

		double degrees = PI / 180.0;

		double _x = double(x) + 0.5;
		double _y = double(y) + 0.5;

		cairo_new_path(m_context);
		cairo_arc(m_context, _x + w - radius, _y + radius, radius, -90 * degrees, 0 * degrees);
		cairo_arc(m_context, _x + w - radius, _y + h - radius, radius, 0 * degrees, 90 * degrees);
		cairo_arc(m_context, _x + radius, _y + h - radius, radius, 90 * degrees, 180 * degrees);
		cairo_arc(m_context, _x + radius, _y + radius, radius, 180 * degrees, 270 * degrees);
		cairo_close_path(m_context);
	}

	void Graphics::Line(int x1, int y1, int x2, int y2) {
		cairo_move_to(m_context, x1+0.5, y1);
		cairo_line_to(m_context, x2+0.5, y2);
	}

	void Graphics::Stroke(bool preserve) {
		if (!preserve) cairo_stroke(m_context);
		else cairo_stroke_preserve(m_context);
	}

	void Graphics::Fill(bool preserve) {
		if (!preserve) cairo_fill(m_context);
		else cairo_fill_preserve(m_context);
	}

	void Graphics::DrawImage(Image* img, int x, int y, int w, int h) {
		cairo_save(m_context);
		cairo_translate(m_context, x, y);
		cairo_scale(m_context, double(w) / double(img->GetWidth()), double(h) / double(img->GetHeight()));
		cairo_set_source_surface(m_context, img->m_surface, 0, 0);
		cairo_paint(m_context);
		cairo_restore(m_context);
	}

	cairo_pattern_t* Graphics::ApplyPaint(Json paint, int x, int y, int w, int h) {
		cairo_pattern_t *pat = nullptr;
		if (paint["color"].is_array()) {
			Json col = paint["color"];
			cairo_set_source_rgba(
					m_context,
					col[0].get<double>(),
					col[1].get<double>(),
					col[2].get<double>(),
					col[3].get<double>()
			);
		} else if (paint["gradient"].is_object()) {
			Json grad = paint["gradient"];
			Json startPos = grad.value("startPos", Json::array({ 0.0, 0.0 }));
			Json endPos = grad.value("endPos", Json::array({ 1.0, 0.0 }));

			pat = cairo_pattern_create_linear(
					startPos[0].get<double>() * w + x,
					startPos[1].get<double>() * h + y,
					endPos[0].get<double>() * w + x,
					endPos[1].get<double>() * h + y
			);

			if (grad["colors"].is_array()) {
				for (auto&& col : grad["colors"]) {
					Json color = col.value("color", Json::array({ 0.0, 0.0, 0.0, 1.0 }));
					Json offset = col.value("offset", 0.0);
					cairo_pattern_add_color_stop_rgba(
							pat,
							offset.get<double>(),
							color[0].get<double>(),
							color[1].get<double>(),
							color[2].get<double>(),
							color[3].get<double>()
					);
				}
			}
			cairo_set_source(m_context, pat);
		}
		return pat;
	}

	void Graphics::StyledPaint(Json style) {
		if (style["background"].is_object()) {
			auto pat = ApplyPaint(style["background"]);
			Fill(true);
			if (pat) cairo_pattern_destroy(pat);
		}
		if (style["border"].is_object()) {
			Json border = style["border"];
			double width = border.value("width", 1.0);
			auto pat = ApplyPaint(border);
			cairo_set_line_width(m_context, width);
			Stroke(false);
			if (pat) cairo_pattern_destroy(pat);
		}
	}

	void Graphics::StyledRect(int x, int y, int w, int h, Json style) {
		cairo_save(m_context);

		double borderRadius = 0.0;
		if (style["border"].is_object()) {
			Json border = style["border"];
			borderRadius = border.value("radius", 0.0);
			// constrain radius to half of the smaller dimension
			borderRadius = std::min(borderRadius, std::min(w, h) / 2.0);
		}

		if (style["background"].is_object()) {
			RoundRect(x, y, w-1, h-1, borderRadius);
			auto pat = ApplyPaint(style["background"], x, y, w, h);
			Fill();
			if (pat) cairo_pattern_destroy(pat);
		}
		if (style["border"].is_object()) {
			Json border = style["border"];
			double width = border.value("width", 1.0);
			RoundRect(x, y, w-1, h-1, borderRadius);
			auto pat = ApplyPaint(border, x, y, w, h);
			cairo_set_line_width(m_context, width);
			Stroke();
			if (pat) cairo_pattern_destroy(pat);
		}
		cairo_restore(m_context);
	}

	void Graphics::StyledTextBegin(Json style) {
		/*
			{
				"font": "Sans",
				"fontSize": 14.0,
				"fontSlant": "normal",
				"fontWeight": "normal",
				"color": [0.0, 0.0, 0.0, 1.0],
			}
		*/
		std::string font = "", fontSlant = "", fontWeight = "";
		double fontSize = 0.0;
		double color[4] = { 0 };

		if (style["font"].is_string()) {
			font = style["font"].get<std::string>();
		}
		if (style["fontSize"].is_number()) {
			fontSize = style["fontSize"].get<double>();
		}
		if (style["fontSlant"].is_string()) {
			fontSlant = style["fontSlant"].get<std::string>();
			std::transform(fontSlant.begin(), fontSlant.end(), fontSlant.begin(), ::tolower);
		}
		if (style["fontWeight"].is_string()) {
			fontWeight = style["fontWeight"].get<std::string>();
			std::transform(fontWeight.begin(), fontWeight.end(), fontWeight.begin(), ::tolower);
		}
		if (style["color"].is_array()) {
			color[0] = style["color"][0].get<double>();
			color[1] = style["color"][1].get<double>();
			color[2] = style["color"][2].get<double>();
			color[3] = style["color"][3].get<double>();
		}

		if (font.empty() || fontSize <= 0.0) {
			return;
		}

		cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
		cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
		if      (fontSlant == "normal")  slant = CAIRO_FONT_SLANT_NORMAL;
		else if (fontSlant == "italic")  slant = CAIRO_FONT_SLANT_ITALIC;
		else if (fontSlant == "oblique") slant = CAIRO_FONT_SLANT_OBLIQUE;

		if      (fontWeight == "normal") weight = CAIRO_FONT_WEIGHT_NORMAL;
		else if (fontWeight == "bold")   weight = CAIRO_FONT_WEIGHT_BOLD;

		cairo_select_font_face(Ctx(), font.c_str(), slant, weight);
		cairo_set_font_size(Ctx(), fontSize);
		cairo_set_source_rgba(Ctx(), color[0], color[1], color[2], color[3]);
	}

	void Graphics::StyledTextEnd(const std::string& text, int x, int y, float rot) {
		cairo_move_to(m_context, x, y);
		cairo_rotate(m_context, rot);
		cairo_text_path(m_context, text.c_str());
		cairo_fill(m_context);
		cairo_rotate(m_context, -rot);
		cairo_move_to(m_context, -x, -y);
	}

	cairo_text_extents_t Graphics::MeasureText(const std::string& text) {
		cairo_text_extents_t extents;
		cairo_text_extents(Ctx(), text.c_str(), &extents);
		return extents;
	}

	void Graphics::DrawCharBegin(FontStyle style, const std::string& font, double fontSize) {
		cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
		cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
		switch (style) {
			case FontStyle::Normal: break;
			case FontStyle::Bold: weight = CAIRO_FONT_WEIGHT_BOLD; break;
			case FontStyle::Italic: slant = CAIRO_FONT_SLANT_ITALIC; break;
			case FontStyle::BoldItalic:
				weight = CAIRO_FONT_WEIGHT_BOLD;
				slant = CAIRO_FONT_SLANT_ITALIC;
				break;
		}
		cairo_select_font_face(m_context, font.c_str(), slant, weight);
		cairo_set_font_size(m_context, fontSize);
	}

	int Graphics::DrawChar(char c, int x, int y) {
		std::string txt = std::string(1, c);
		auto&& ex = MeasureText(txt);

		cairo_move_to(m_context, x, y);
		cairo_text_path(m_context, txt.c_str());
		cairo_fill(m_context);
		cairo_move_to(m_context, -x, -y);

		return x + ex.x_advance;
	}

    void Graphics::DrawSVG(Json svgStyle, int x, int y, int w, int h)
    {
		if (!svgStyle["svg"].is_string()) return;

		std::string svgPath = svgStyle["svg"].get<std::string>();
		std::istringstream ss(svgPath);

		cairo_save(m_context);
		cairo_translate(m_context, x, y);

		// Scale from 24x24 viewbox to target size
		double viewWidth = svgStyle.value("width", 24.0);
		double viewHeight = svgStyle.value("height", 24.0);
		cairo_scale(m_context, w / viewWidth, h / viewHeight);

		// Build the cairo path
		cairo_new_path(m_context);
		char cmd;
		double currentX = 0, currentY = 0;
		double startX = 0, startY = 0;
		// Last control point for smooth curves (S/s, T/t)
		double lastCpX = 0, lastCpY = 0;
		char lastCmd = 0;

		auto skipSep = [&]() {
			while (ss.peek() == ' ' || ss.peek() == ',') ss.get();
		};
		auto hasCoords = [&]() -> bool {
			skipSep();
			int c = ss.peek();
			return c != EOF && !std::isalpha(c);
		};

		while (ss >> cmd) {
			switch (cmd) {
				case 'M': {
					double x1, y1;
					ss >> x1 >> y1;
					cairo_move_to(m_context, x1, y1);
					currentX = x1; currentY = y1;
					startX = x1; startY = y1;
					while (hasCoords()) {
						ss >> x1 >> y1;
						cairo_line_to(m_context, x1, y1);
						currentX = x1; currentY = y1;
					}
					break;
				}
				case 'm': {
					double dx, dy;
					ss >> dx >> dy;
					currentX += dx; currentY += dy;
					cairo_move_to(m_context, currentX, currentY);
					startX = currentX; startY = currentY;
					while (hasCoords()) {
						ss >> dx >> dy;
						currentX += dx; currentY += dy;
						cairo_line_to(m_context, currentX, currentY);
					}
					break;
				}
				case 'L': {
					double x1, y1;
					ss >> x1 >> y1;
					cairo_line_to(m_context, x1, y1);
					currentX = x1; currentY = y1;
					while (hasCoords()) {
						ss >> x1 >> y1;
						cairo_line_to(m_context, x1, y1);
						currentX = x1; currentY = y1;
					}
					break;
				}
				case 'l': {
					double dx, dy;
					ss >> dx >> dy;
					currentX += dx; currentY += dy;
					cairo_line_to(m_context, currentX, currentY);
					while (hasCoords()) {
						ss >> dx >> dy;
						currentX += dx; currentY += dy;
						cairo_line_to(m_context, currentX, currentY);
					}
					break;
				}
				case 'H': {
					double x1;
					ss >> x1;
					cairo_line_to(m_context, x1, currentY);
					currentX = x1;
					break;
				}
				case 'h': {
					double dx;
					ss >> dx;
					currentX += dx;
					cairo_line_to(m_context, currentX, currentY);
					break;
				}
				case 'V': {
					double y1;
					ss >> y1;
					cairo_line_to(m_context, currentX, y1);
					currentY = y1;
					break;
				}
				case 'v': {
					double dy;
					ss >> dy;
					currentY += dy;
					cairo_line_to(m_context, currentX, currentY);
					break;
				}
				case 'C': {
					double cx1, cy1, cx2, cy2, ex, ey;
					do {
						ss >> cx1 >> cy1 >> cx2 >> cy2 >> ex >> ey;
						cairo_curve_to(m_context, cx1, cy1, cx2, cy2, ex, ey);
						lastCpX = cx2; lastCpY = cy2;
						currentX = ex; currentY = ey;
					} while (hasCoords());
					break;
				}
				case 'c': {
					double cx1, cy1, cx2, cy2, dx, dy;
					do {
						ss >> cx1 >> cy1 >> cx2 >> cy2 >> dx >> dy;
						cairo_curve_to(m_context,
							currentX + cx1, currentY + cy1,
							currentX + cx2, currentY + cy2,
							currentX + dx, currentY + dy);
						lastCpX = currentX + cx2; lastCpY = currentY + cy2;
						currentX += dx; currentY += dy;
					} while (hasCoords());
					break;
				}
				case 'S': {
					double cx2, cy2, ex, ey;
					do {
						double cx1, cy1;
						if (lastCmd == 'C' || lastCmd == 'c' || lastCmd == 'S' || lastCmd == 's') {
							cx1 = 2 * currentX - lastCpX;
							cy1 = 2 * currentY - lastCpY;
						} else {
							cx1 = currentX; cy1 = currentY;
						}
						ss >> cx2 >> cy2 >> ex >> ey;
						cairo_curve_to(m_context, cx1, cy1, cx2, cy2, ex, ey);
						lastCpX = cx2; lastCpY = cy2;
						currentX = ex; currentY = ey;
					} while (hasCoords());
					break;
				}
				case 's': {
					double cx2, cy2, dx, dy;
					do {
						double cx1, cy1;
						if (lastCmd == 'C' || lastCmd == 'c' || lastCmd == 'S' || lastCmd == 's') {
							cx1 = 2 * currentX - lastCpX;
							cy1 = 2 * currentY - lastCpY;
						} else {
							cx1 = currentX; cy1 = currentY;
						}
						ss >> cx2 >> cy2 >> dx >> dy;
						cairo_curve_to(m_context, cx1, cy1,
							currentX + cx2, currentY + cy2,
							currentX + dx, currentY + dy);
						lastCpX = currentX + cx2; lastCpY = currentY + cy2;
						currentX += dx; currentY += dy;
					} while (hasCoords());
					break;
				}
				case 'Q': {
					double qx, qy, ex, ey;
					do {
						ss >> qx >> qy >> ex >> ey;
						// Convert quadratic to cubic: CP1 = P0 + 2/3*(QP-P0), CP2 = P1 + 2/3*(QP-P1)
						double cx1 = currentX + 2.0/3.0 * (qx - currentX);
						double cy1 = currentY + 2.0/3.0 * (qy - currentY);
						double cx2 = ex + 2.0/3.0 * (qx - ex);
						double cy2 = ey + 2.0/3.0 * (qy - ey);
						cairo_curve_to(m_context, cx1, cy1, cx2, cy2, ex, ey);
						lastCpX = qx; lastCpY = qy;
						currentX = ex; currentY = ey;
					} while (hasCoords());
					break;
				}
				case 'q': {
					double dqx, dqy, dx, dy;
					do {
						ss >> dqx >> dqy >> dx >> dy;
						double qx = currentX + dqx, qy = currentY + dqy;
						double ex = currentX + dx, ey = currentY + dy;
						double cx1 = currentX + 2.0/3.0 * (qx - currentX);
						double cy1 = currentY + 2.0/3.0 * (qy - currentY);
						double cx2 = ex + 2.0/3.0 * (qx - ex);
						double cy2 = ey + 2.0/3.0 * (qy - ey);
						cairo_curve_to(m_context, cx1, cy1, cx2, cy2, ex, ey);
						lastCpX = qx; lastCpY = qy;
						currentX = ex; currentY = ey;
					} while (hasCoords());
					break;
				}
				case 'T': {
					double ex, ey;
					do {
						double qx, qy;
						if (lastCmd == 'Q' || lastCmd == 'q' || lastCmd == 'T' || lastCmd == 't') {
							qx = 2 * currentX - lastCpX;
							qy = 2 * currentY - lastCpY;
						} else {
							qx = currentX; qy = currentY;
						}
						ss >> ex >> ey;
						double cx1 = currentX + 2.0/3.0 * (qx - currentX);
						double cy1 = currentY + 2.0/3.0 * (qy - currentY);
						double cx2 = ex + 2.0/3.0 * (qx - ex);
						double cy2 = ey + 2.0/3.0 * (qy - ey);
						cairo_curve_to(m_context, cx1, cy1, cx2, cy2, ex, ey);
						lastCpX = qx; lastCpY = qy;
						currentX = ex; currentY = ey;
					} while (hasCoords());
					break;
				}
				case 't': {
					double dx, dy;
					do {
						double qx, qy;
						if (lastCmd == 'Q' || lastCmd == 'q' || lastCmd == 'T' || lastCmd == 't') {
							qx = 2 * currentX - lastCpX;
							qy = 2 * currentY - lastCpY;
						} else {
							qx = currentX; qy = currentY;
						}
						ss >> dx >> dy;
						double ex = currentX + dx, ey = currentY + dy;
						double cx1 = currentX + 2.0/3.0 * (qx - currentX);
						double cy1 = currentY + 2.0/3.0 * (qy - currentY);
						double cx2 = ex + 2.0/3.0 * (qx - ex);
						double cy2 = ey + 2.0/3.0 * (qy - ey);
						cairo_curve_to(m_context, cx1, cy1, cx2, cy2, ex, ey);
						lastCpX = qx; lastCpY = qy;
						currentX = ex; currentY = ey;
					} while (hasCoords());
					break;
				}
				case 'A': case 'a': {
					double rx, ry, xRot, ex, ey;
					int largeArc, sweep;
					do {
						ss >> rx >> ry >> xRot >> largeArc >> sweep >> ex >> ey;
						if (cmd == 'a') { ex += currentX; ey += currentY; }
						// Approximate arc with cairo_arc for circular arcs, otherwise lineto as fallback
						if (rx == ry && rx > 0) {
							// Circular arc — compute center parameterization
							double dx2 = (currentX - ex) / 2.0;
							double dy2 = (currentY - ey) / 2.0;
							double r = rx;
							double d = dx2 * dx2 + dy2 * dy2;
							double sf = std::sqrt(std::max(0.0, r * r / d - 1.0));
							if (largeArc == sweep) sf = -sf;
							double cx = (currentX + ex) / 2.0 + sf * dy2;
							double cy = (currentY + ey) / 2.0 - sf * dx2;
							double a1 = std::atan2(currentY - cy, currentX - cx);
							double a2 = std::atan2(ey - cy, ex - cx);
							if (sweep)
								cairo_arc(m_context, cx, cy, r, a1, a2);
							else
								cairo_arc_negative(m_context, cx, cy, r, a1, a2);
						} else {
							cairo_line_to(m_context, ex, ey);
						}
						currentX = ex; currentY = ey;
					} while (hasCoords());
					break;
				}
				case 'Z':
				case 'z': {
					cairo_close_path(m_context);
					currentX = startX;
					currentY = startY;
					break;
				}
				default: break;
			}
			lastCmd = cmd;
		}

		// Parse line-cap and line-join
		cairo_line_cap_t lineCap = CAIRO_LINE_CAP_BUTT;
		cairo_line_join_t lineJoin = CAIRO_LINE_JOIN_MITER;
		if (svgStyle["line-cap"].is_string()) {
			std::string cap = svgStyle["line-cap"].get<std::string>();
			if (cap == "round") lineCap = CAIRO_LINE_CAP_ROUND;
			else if (cap == "square") lineCap = CAIRO_LINE_CAP_SQUARE;
		}
		if (svgStyle["line-join"].is_string()) {
			std::string join = svgStyle["line-join"].get<std::string>();
			if (join == "round") lineJoin = CAIRO_LINE_JOIN_ROUND;
			else if (join == "bevel") lineJoin = CAIRO_LINE_JOIN_BEVEL;
		}

		// Apply fill and/or stroke using ApplyPaint
		// Use viewbox coordinates since we've translated and scaled
		if (svgStyle["fill"].is_object()) {
			auto pat = ApplyPaint(svgStyle["fill"], 0, 0, viewWidth, viewHeight);
			Fill(svgStyle.contains("stroke"));
			if (pat) cairo_pattern_destroy(pat);
		}
		if (svgStyle["stroke"].is_object()) {
			Json strokeStyle = svgStyle["stroke"];
			double lineWidth = strokeStyle.value("width", 1.0);
			cairo_set_line_width(m_context, lineWidth);
			cairo_set_line_cap(m_context, lineCap);
			cairo_set_line_join(m_context, lineJoin);
			auto pat = ApplyPaint(strokeStyle, 0, 0, viewWidth, viewHeight);
			Stroke();
			if (pat) cairo_pattern_destroy(pat);
		}

		cairo_restore(m_context);
	}

    void Graphics::ClipPush(int x, int y, int w, int h) {
		cairo_save(m_context);
		cairo_rectangle(m_context, x, y, w, h);
		cairo_clip(m_context);
		m_clipRects.push(Rectangle(x, y, w, h));
	}

	void Graphics::ClipPop() {
		if (!m_clipRects.empty()) {
			Rectangle b = m_clipRects.top();
			m_clipRects.pop();
			cairo_rectangle(m_context, b.x, b.y, b.w, b.h);
			cairo_clip(m_context);
		} else {
			cairo_reset_clip(m_context);
		}
		cairo_restore(m_context);
	}

	void Graphics::BeginPath() {
		m_pathPoints.clear();
	}

	void Graphics::AddPathRect(int x, int y, int w, int h) {
		m_pathPoints.push_back({ x, y });
		m_pathPoints.push_back({ x + w, y });
		m_pathPoints.push_back({ x + w, y + h });
		m_pathPoints.push_back({ x, y + h });
	}

	void Graphics::AddPathPoint(int x, int y) {
		m_pathPoints.push_back({ x, y });
	}

	void Graphics::EndPath(bool close) {
		if (m_pathPoints.empty()) return;
		cairo_new_path(m_context);
		cairo_move_to(m_context, m_pathPoints[0].x, m_pathPoints[0].y);
		for (int i = 1; i < m_pathPoints.size(); i++) {
			cairo_line_to(m_context, m_pathPoints[i].x, m_pathPoints[i].y);
		}
		if (close) cairo_close_path(m_context);
	}

    void Graphics::Translate(double tx, double ty)
    {
		cairo_translate(m_context, tx, ty);
    }

    void Graphics::Rotate(double angle)
    {
		cairo_rotate(m_context, angle);
    }

    void Graphics::Scale(double sx, double sy)
    {
		cairo_scale(m_context, sx, sy);
    }

    void Graphics::Save() {
		cairo_save(Ctx());
	}

	void Graphics::Restore() {
		cairo_restore(Ctx());
	}

	void Graphics::Draw(DrawFunction func) {
		if (func == nullptr) return;

		const SDL_FRect dst = { 0, 0, (float)m_width, (float)m_height };

		Uint8 *pixels;
		int pitch;
		SDL_LockTexture(m_buffer, nullptr, (void**)&pixels, &pitch);
		m_surface = cairo_image_surface_create_for_data(
					pixels,
					CAIRO_FORMAT_ARGB32,
					m_width, m_height,
					pitch
		);
		m_context = cairo_create(m_surface);

		func(*this);

		cairo_surface_flush(m_surface);
		cairo_destroy(m_context);
		cairo_surface_destroy(m_surface);
		m_context = nullptr;
		m_surface = nullptr;

		SDL_UnlockTexture(m_buffer);

		SDL_RenderTexture(m_renderer, m_buffer, nullptr, &dst);
		SDL_RenderPresent(m_renderer);
	}

	Image::~Image() {
		if (m_surface) {
			cairo_surface_destroy(m_surface);
			m_surface = nullptr;
		}
	}

	Image::Image(Image&& other) noexcept
		: m_width(other.m_width), m_height(other.m_height), m_surface(other.m_surface)
	{
		other.m_surface = nullptr;
		other.m_width = 0;
		other.m_height = 0;
	}

	Image& Image::operator=(Image&& other) noexcept {
		if (this != &other) {
			if (m_surface) {
				cairo_surface_destroy(m_surface);
			}
			m_surface = other.m_surface;
			m_width = other.m_width;
			m_height = other.m_height;
			other.m_surface = nullptr;
			other.m_width = 0;
			other.m_height = 0;
		}
		return *this;
	}

	Image::Image(const std::string& fileName) {
		m_surface = cairo_image_surface_create_from_png(fileName.c_str());
		m_width = cairo_image_surface_get_width(m_surface);
		m_height = cairo_image_surface_get_height(m_surface);
	}

    Image::Image(int width, int height)
    {
		m_width = width;
		m_height = height;
		m_surface = cairo_image_surface_create(
			CAIRO_FORMAT_ARGB32,
			width, height
		);
    }

    void Image::SetPixels(const unsigned char *data, int stride)
    {
		if (!m_surface) return;

		cairo_surface_flush(m_surface);

		unsigned char *surfaceData = cairo_image_surface_get_data(m_surface);
		int surfaceStride = cairo_image_surface_get_stride(m_surface);

		for (int y = 0; y < m_height; y++) {
			std::memcpy(
				surfaceData + y * surfaceStride,
				data + y * stride,
				std::min(surfaceStride, stride)
			);
		}
		cairo_surface_mark_dirty(m_surface);
    }

    void Image::Resize(int w, int h)
    {
		if (m_surface) {
			cairo_surface_destroy(m_surface);
			m_surface = nullptr;
		}

		m_width = w;
		m_height = h;

		m_surface = cairo_image_surface_create(
			CAIRO_FORMAT_ARGB32,
			w, h
		);
    }

    bool Rectangle::HasPoint(int x, int y) {
		return x >= this->x &&
			x <= this->x + w &&
			y >= this->y &&
			y <= this->y + h;
	}

	bool Rectangle::Intersects(Rectangle b) {
		int hwa = w / 2;
		int hwb = b.w / 2;
		int hha = h / 2;
		int hhb = b.h / 2;

		int cax = x + hwa;
		int cay = y + hha;
		int cbx = b.x + hwb;
		int cby = b.y + hhb;

		if (std::abs(cax - cbx) > (hwa + hwb)) return false;
		if (std::abs(cay - cby) > (hha + hhb)) return false;

		return true;
	}

	std::optional<Rectangle> Rectangle::Intersect(Rectangle b) {
		int xmin = std::max(x, b.x);
		int xmax1 = x + w, xmax2 = b.x + b.w;
		int xmax = std::min(xmax1, xmax2);
		if (xmax > xmin) {
			int ymin = std::max(y, b.y);
			int ymax1 = y + h, ymax2 = b.y + b.h;
			int ymax = std::min(ymax1, ymax2);
			if (ymax > ymin) {
				return Rectangle(xmin, ymin, xmax - xmin, ymax - ymin);
			}
		}
		return {};
	}

}
