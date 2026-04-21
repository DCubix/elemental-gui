#include "Graphics.h"

#include <cairo/cairo.h>

#include <cctype>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <stdexcept>

constexpr int SHAPE_SIZE = 64;

namespace tui {
	
	Graphics::Graphics()
	{
		m_measureSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
		m_measureContext = cairo_create(m_measureSurface);
	}

	void Graphics::Clear(float r, float g, float b, float a) {
		cairo_set_source_rgba(m_context, r, g, b, a);
		cairo_paint(m_context);
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
		if (!img->IsValid()) return;

		if (img->GetType() == Image::Type::SVG) {
			// get current cairo color
			cairo_pattern_t* pat = cairo_get_source(m_context);
			cairo_pattern_type_t patType = cairo_pattern_get_type(pat);
			double r=0, g=0, b=0, a=1;
			if (patType == CAIRO_PATTERN_TYPE_SOLID) {
				cairo_pattern_get_rgba(pat, &r, &g, &b, &a);
			} else if (patType == CAIRO_PATTERN_TYPE_LINEAR || patType == CAIRO_PATTERN_TYPE_RADIAL) {
				double offset;
				cairo_pattern_get_color_stop_rgba(pat, 0, &offset, &r, &g, &b, &a);
			}

			uint8_t ur = static_cast<uint8_t>(std::clamp(r * 255.0, 0.0, 255.0));
			uint8_t ug = static_cast<uint8_t>(std::clamp(g * 255.0, 0.0, 255.0));
			uint8_t ub = static_cast<uint8_t>(std::clamp(b * 255.0, 0.0, 255.0));
			uint8_t ua = static_cast<uint8_t>(std::clamp(a * 255.0, 0.0, 255.0));
			uint32_t colOverride =
				(ua << 24) | (ub << 16) | (ug << 8) | ur;

			img->RasterizeSVG(w, h, colOverride, colOverride);
		}

		cairo_save(m_context);
		cairo_translate(m_context, x, y);
		cairo_scale(m_context,
			double(w) / double(img->GetWidth()),
			double(h) / double(img->GetHeight())
		);
		cairo_set_source_surface(m_context, img->m_surface, 0, 0);
		cairo_paint(m_context);
		cairo_restore(m_context);
	}

	static void ParseColor(Json style, double& r, double& g, double& b, double& a) {
		if (style.is_array()) {
			r = style[0].get<double>();
			g = style[1].get<double>();
			b = style[2].get<double>();
			if (style.size() > 3)
				a = style[3].get<double>();
			else
				a = 1.0;
		} else if (style.is_string()) {
			std::string hex = style.get<std::string>();
			tui::Color col = Color::FromHex(hex);
			r = col.r;
			g = col.g;
			b = col.b;
			a = col.a;
		} else {
			r = g = b = 0.0;
			a = 1.0;
		}
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
		} else if (paint["color"].is_string()) {
			double r, g, b, a;
			ParseColor(paint["color"], r, g, b, a);
			cairo_set_source_rgba(m_context, r, g, b, a);
		} else if (paint["linearGradient"].is_object()) {
			Json grad = paint["linearGradient"];
			Json startPos = grad.value("start", Json::array({ 0.0, 0.0 }));
			Json endPos = grad.value("end", Json::array({ 1.0, 0.0 }));

			pat = cairo_pattern_create_linear(
					startPos[0].get<double>() * w + x,
					startPos[1].get<double>() * h + y,
					endPos[0].get<double>() * w + x,
					endPos[1].get<double>() * h + y
			);
			cairo_pattern_set_dither(pat, CAIRO_DITHER_BEST);

			std::vector<std::tuple<double, double, double, double>> colors;
			std::vector<double> stops;

			if (grad["colors"].is_array()) {
				for (auto&& col : grad["colors"]) {
					double r, g, b, a;
					ParseColor(col, r, g, b, a);
					colors.emplace_back(r, g, b, a);
				}
			}

			if (grad["stops"].is_array()) {
				for (auto&& stop : grad["stops"]) {
					stops.push_back(stop.get<double>());
				}
			}

			if (colors.size() == stops.size() && !colors.empty()) {
				for (size_t i = 0; i < colors.size(); ++i) {
					const auto& [r, g, b, a] = colors[i];
					cairo_pattern_add_color_stop_rgba(
							pat,
							stops[i],
							r, g, b, a
					);
				}
			}

			cairo_set_source(m_context, pat);
		} else if (paint["radialGradient"].is_object()) {
			Json grad = paint["radialGradient"];
			Json center = grad.value("center", Json::array({ 0.5, 0.5 }));
			double radius = grad.value("radius", 0.5);

			pat = cairo_pattern_create_radial(
					center[0].get<double>() * w + x,
					center[1].get<double>() * h + y,
					0,
					center[0].get<double>() * w + x,
					center[1].get<double>() * h + y,
					radius * std::max(w, h)
			);
			cairo_pattern_set_dither(pat, CAIRO_DITHER_BEST);

			std::vector<std::tuple<double, double, double, double>> colors;
			std::vector<double> stops;

			if (grad["colors"].is_array()) {
				for (auto&& col : grad["colors"]) {
					double r, g, b, a;
					ParseColor(col, r, g, b, a);
					colors.emplace_back(r, g, b, a);
				}
			}

			if (grad["stops"].is_array()) {
				for (auto&& stop : grad["stops"]) {
					stops.push_back(stop.get<double>());
				}
			}

			if (colors.size() == stops.size() && !colors.empty()) {
				for (size_t i = 0; i < colors.size(); ++i) {
					const auto& [r, g, b, a] = colors[i];
					cairo_pattern_add_color_stop_rgba(
							pat,
							stops[i],
							r, g, b, a
					);
				}
			}

			cairo_set_source(m_context, pat);
		}
		return pat;
	}

    void Graphics::DrawShadow(float elevation, int x, int y, int w, int h, float radius)
    {
		cairo_pattern_t* pat = CreateRoundShadowPattern(elevation, x, y, w, h, radius);
		if (pat) {
			cairo_set_source(m_context, pat);
			cairo_paint(m_context);
			cairo_pattern_destroy(pat);
		}
    }

    void Graphics::StyledPaint(Json style) {
		if (style["background"].is_object()) {
			auto pat = ApplyPaint(style["background"]);
			bool shouldPreserve =
				style["border"].is_object() && style["border"].value("width", 0.0) > 0.0;
			Fill(shouldPreserve);
			if (pat) cairo_pattern_destroy(pat);
		}
		if (style["border"].is_object()) {
			Json border = style["border"];
			double width = border.value("width", 1.0);
			if (width > 0.0) {
				auto pat = ApplyPaint(border);
				cairo_set_line_width(m_context, width);
				Stroke(false);
				if (pat) cairo_pattern_destroy(pat);
			}
		}
	}

	void Graphics::StyledRect(int x, int y, int w, int h, Json style) {
		cairo_save(m_context);

		if (style["elevation"].is_number()) {
			float elev = style["elevation"].get<float>();
			float radius = style["border"].is_object()
				? style["border"].value("radius", 0.0f) : 0.0f;

			// bypass clipping
			cairo_save(m_context);
			cairo_reset_clip(m_context);
			DrawShadow(elev, x, y, w, h, radius);
			cairo_restore(m_context);
		}

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
		} else if (style["color"].is_string()) {
			std::string hex = style["color"].get<std::string>();
			tui::Color col = Color::FromHex(hex);
			color[0] = col.r;
			color[1] = col.g;
			color[2] = col.b;
			color[3] = col.a;
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
		if (svgStyle["lineCap"].is_string()) {
			std::string cap = svgStyle["lineCap"].get<std::string>();
			if (cap == "round") lineCap = CAIRO_LINE_CAP_ROUND;
			else if (cap == "square") lineCap = CAIRO_LINE_CAP_SQUARE;
		}
		if (svgStyle["lineJoin"].is_string()) {
			std::string join = svgStyle["lineJoin"].get<std::string>();
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

    void Graphics::ClipPushRect(int x, int y, int w, int h) {
		ClipPushPath([&]() { cairo_rectangle(m_context, x, y, w, h); });
	}

    void Graphics::ClipPushRoundRect(int x, int y, int w, int h, float radius)
    {
		ClipPushPath([&]() { RoundRect(x, y, w, h, radius); });
    }

    void Graphics::ClipPushPath(std::function<void()> pathFunc)
    {
		if (!m_context || !pathFunc) return;
		cairo_save(m_context);
		pathFunc();
		cairo_clip(m_context);
		m_clipDepth++;
    }

    void Graphics::ClipPop() {
		if (m_clipDepth == 0) return;
		m_clipDepth--;
		cairo_restore(m_context);
	}

    void Graphics::GetStyledPath(Json style, int x, int y, int w, int h)
    {
		double borderRadius = 0.0;
		if (style["border"].is_object()) {
			Json border = style["border"];
			borderRadius = border.value("radius", 0.0);
		}
		RoundRect(x, y, w-1, h-1, borderRadius);
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

    void Graphics::BeginDrawing(uint width, uint height)
    {
		m_surface = cairo_image_surface_create(
			CAIRO_FORMAT_ARGB32,
			width, height
		);
		m_context = cairo_create(m_surface);
    }

    void Graphics::Flush()
    {
		cairo_surface_flush(m_surface);
    }

    void Graphics::EndDrawing()
    {
		cairo_destroy(m_context);
		cairo_surface_destroy(m_surface);
		m_context = nullptr;
		m_surface = nullptr;
    }

    cairo_pattern_t* Graphics::CreateRoundShadowPattern(float elevation, int x, int y, int w, int h, float radius)
    {
		float cornerR = std::clamp(radius, 1.0f, std::min(w, h) / 2.0f);
		float offsetY = elevation * 0.3f;
		float edgeThickness = elevation * 2.0f;
		float shadowAlpha = std::clamp(0.6f - elevation * 0.1f, 0.15f, 0.5f);

		float inR = cornerR;
		float outR = cornerR + edgeThickness;

		const float e = 0.5f;
		const float th = std::acos(2.0f * std::pow((1.0f - e / cornerR), 2) - 1.0f);
		const int stepsPerCorner = std::max(2, static_cast<int>(std::ceil((M_PI / 2) / th)));

		// Corner centers: inset by cornerR so inner ring aligns with rect edges
		struct Vec2 { float x, y; };
		Vec2 centers[4] = {
			{ (float)x + cornerR,       (float)y + cornerR + offsetY },
			{ (float)(x + w) - cornerR, (float)y + cornerR + offsetY },
			{ (float)(x + w) - cornerR, (float)(y + h) - cornerR + offsetY },
			{ (float)x + cornerR,       (float)(y + h) - cornerR + offsetY }
		};
		float startAngles[4] = {
			(float)M_PI,             // top-left: π → 3π/2
			(float)(3 * M_PI / 2),   // top-right: 3π/2 → 2π
			0.0f,                    // bottom-right: 0 → π/2
			(float)(M_PI / 2)        // bottom-left: π/2 → π
		};

		// Generate inner and outer ring points
		std::vector<Vec2> innerPts, outerPts;
		for (int c = 0; c < 4; c++) {
			for (int i = 0; i <= stepsPerCorner; i++) {
				float t = (float)i / stepsPerCorner;
				float angle = startAngles[c] + t * (float)(M_PI / 2);
				float cs = std::cos(angle);
				float sn = std::sin(angle);
				innerPts.push_back({ centers[c].x + inR * cs, centers[c].y + inR * sn });
				outerPts.push_back({ centers[c].x + outR * cs, centers[c].y + outR * sn });
			}
		}

		cairo_pattern_t* pattern = cairo_pattern_create_mesh();
		int n = (int)innerPts.size();

		// Gradient ring: one quad patch per segment
		for (int i = 0; i < n; i++) {
			int j = (i + 1) % n;
			cairo_mesh_pattern_begin_patch(pattern);
			cairo_mesh_pattern_move_to(pattern, innerPts[i].x, innerPts[i].y);
			cairo_mesh_pattern_line_to(pattern, innerPts[j].x, innerPts[j].y);
			cairo_mesh_pattern_line_to(pattern, outerPts[j].x, outerPts[j].y);
			cairo_mesh_pattern_line_to(pattern, outerPts[i].x, outerPts[i].y);
			cairo_mesh_pattern_set_corner_color_rgba(pattern, 0, 0, 0, 0, shadowAlpha);
			cairo_mesh_pattern_set_corner_color_rgba(pattern, 1, 0, 0, 0, shadowAlpha);
			cairo_mesh_pattern_set_corner_color_rgba(pattern, 2, 0, 0, 0, 0.0f);
			cairo_mesh_pattern_set_corner_color_rgba(pattern, 3, 0, 0, 0, 0.0f);
			cairo_mesh_pattern_end_patch(pattern);
		}

		// Solid fill: triangle fan from center to inner ring
		float cx = x + w * 0.5f;
		float cy = y + h * 0.5f + offsetY;
		for (int i = 0; i < n; i++) {
			int j = (i + 1) % n;
			cairo_mesh_pattern_begin_patch(pattern);
			cairo_mesh_pattern_move_to(pattern, cx, cy);
			cairo_mesh_pattern_line_to(pattern, innerPts[i].x, innerPts[i].y);
			cairo_mesh_pattern_line_to(pattern, innerPts[j].x, innerPts[j].y);
			cairo_mesh_pattern_line_to(pattern, cx, cy);
			cairo_mesh_pattern_set_corner_color_rgba(pattern, 0, 0, 0, 0, shadowAlpha);
			cairo_mesh_pattern_set_corner_color_rgba(pattern, 1, 0, 0, 0, shadowAlpha);
			cairo_mesh_pattern_set_corner_color_rgba(pattern, 2, 0, 0, 0, shadowAlpha);
			cairo_mesh_pattern_set_corner_color_rgba(pattern, 3, 0, 0, 0, shadowAlpha);
			cairo_mesh_pattern_end_patch(pattern);
		}

		return pattern;
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

    Rectangle Rectangle::FromLRTB(int left, int right, int top, int bottom)
    {
        return Rectangle(left, top, right - left, bottom - top);
    }

    Rectangle Rectangle::FromWidth(int w)
    {
        return Rectangle(0, 0, w, 0);
    }

    Rectangle Rectangle::FromHeight(int h)
    {
        return Rectangle(0, 0, 0, h);
    }

    Rectangle Rectangle::FromSize(int w, int h)
    {
        return Rectangle(0, 0, w, h);
    }

    std::string Color::ToHex() const
    {
		std::stringstream ss;
		ss << "#" << std::hex << std::uppercase;
		ss << (int)(r * 255) << (int)(g * 255) << (int)(b * 255);
		return ss.str();
    }

    Color Color::Darken(float amount) const
    {
		return Color(
			std::max(0.0f, r - amount),
			std::max(0.0f, g - amount),
			std::max(0.0f, b - amount),
			a
		);
    }
    
	Color Color::Lighten(float amount) const
    {
		return Color(
			std::min(1.0f, r + amount),
			std::min(1.0f, g + amount),
			std::min(1.0f, b + amount),
			a
		);
    }
    
	Color Color::FromHex(const std::string &hex)
    {
		std::string cleanHex = hex;
		if (cleanHex[0] == '#') {
			cleanHex = cleanHex.substr(1);
		}
		if (cleanHex.length() == 6) {
			int r = std::stoi(cleanHex.substr(0, 2), nullptr, 16);
			int g = std::stoi(cleanHex.substr(2, 2), nullptr, 16);
			int b = std::stoi(cleanHex.substr(4, 2), nullptr, 16);
			return Color::FromRGB(r / 255.0f, g / 255.0f, b / 255.0f);
		} else if (cleanHex.length() == 8) {
			int a = std::stoi(cleanHex.substr(0, 2), nullptr, 16);
			int r = std::stoi(cleanHex.substr(2, 2), nullptr, 16);
			int g = std::stoi(cleanHex.substr(4, 2), nullptr, 16);
			int b = std::stoi(cleanHex.substr(6, 2), nullptr, 16);
			return Color::FromRGBA(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
		} else if (cleanHex.length() == 3) {
			int r = std::stoi(std::string(2, cleanHex[0]), nullptr, 16);
			int g = std::stoi(std::string(2, cleanHex[1]), nullptr, 16);
			int b = std::stoi(std::string(2, cleanHex[2]), nullptr, 16);
			return Color::FromRGB(r / 255.0f, g / 255.0f, b / 255.0f);
		}
		throw std::invalid_argument("Invalid hex color format: " + hex);
    }
    
	Color Color::FromRGBA(float r, float g, float b, float a)
    {
        return Color(r, g, b, a);
    }
    
	Color Color::FromRGB(float r, float g, float b)
    {
        return Color(r, g, b, 1.0f);
    }
    
	Color Color::FromHSLA(float h, float s, float l, float a)
    {
		h = std::fmod(h, 360.0f) / 60.0f;
		s = std::clamp(s, 0.0f, 1.0f);
		l = std::clamp(l, 0.0f, 1.0f);
		a = std::clamp(a, 0.0f, 1.0f);

		float c = (1 - std::abs(2 * l - 1)) * s;
		float x = c * (1 - std::abs(std::fmod(h, 2) - 1));
		float m = l - c / 2;

		float r1, g1, b1;
		if (h < 1) { r1 = c; g1 = x; b1 = 0; }
		else if (h < 2) { r1 = x; g1 = c; b1 = 0; }
		else if (h < 3) { r1 = 0; g1 = c; b1 = x; }
		else if (h < 4) { r1 = 0; g1 = x; b1 = c; }
		else if (h < 5) { r1 = x; g1 = 0; b1 = c; }
		else { r1 = c; g1 = 0; b1 = x; }

		return Color(r1 + m, g1 + m, b1 + m, a);
    }
    
	Color Color::FromHSL(float h, float s, float l)
    {
        return Color::FromHSLA(h, s, l, 1.0f);
    }
    
}