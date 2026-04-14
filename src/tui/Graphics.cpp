#include "Graphics.h"

#include <cctype>
#include <algorithm>

namespace tui {

	Graphics::Graphics(SDL_Renderer* ren)
		: m_renderer(ren), m_buffer(nullptr), m_surface(nullptr)
	{}

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

	void Graphics::StyledPaint(Json style) {
		if (style["background"].is_object()) {
			cairo_pattern_t *pat = nullptr;
			Json bg = style["background"];
			if (bg["color"].is_array()) {
				Json col = bg["color"];
				cairo_set_source_rgba(
						m_context,
						col[0].get<double>(),
						col[1].get<double>(),
						col[2].get<double>(),
						col[3].get<double>()
				);
			}

			Fill(true);
			if (pat) cairo_pattern_destroy(pat);
		}
		if (style["border"].is_object()) {
			Json border = style["border"];
			Json color = border.value("color", Json::array({ 0.0, 0.0, 0.0, 1.0 }));
			double width = border.value("width", 1.0);
			cairo_set_source_rgba(
						m_context,
						color[0].get<double>(),
						color[1].get<double>(),
						color[2].get<double>(),
						color[3].get<double>()
			);
			cairo_set_line_width(m_context, width);
			Stroke(false);
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
			RoundRect(x, y, w-1, h-1, borderRadius);
		}

		if (style["background"].is_object()) {
			cairo_pattern_t *pat = nullptr;
			Json bg = style["background"];
			if (bg["color"].is_array()) {
				Json col = bg["color"];
				cairo_set_source_rgba(
						m_context,
						col[0].get<double>(),
						col[1].get<double>(),
						col[2].get<double>(),
						col[3].get<double>()
				);
			} else if (bg["gradient"].is_object()) {
				Json grad = bg["gradient"];
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

			Fill();
			if (pat) cairo_pattern_destroy(pat);
		}
		if (style["border"].is_object()) {
			Json border = style["border"];
			Json color = border.value("color", Json::array({ 0.0, 0.0, 0.0, 1.0 }));
			double width = border.value("width", 1.0);
			RoundRect(x, y, w-1, h-1, borderRadius);
			cairo_set_source_rgba(
						m_context,
						color[0].get<double>(),
						color[1].get<double>(),
						color[2].get<double>(),
						color[3].get<double>()
			);
			cairo_set_line_width(m_context, width);
			Stroke();
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

		cairo_select_font_face(m_context, font.c_str(), slant, weight);
		cairo_set_font_size(m_context, fontSize);
		cairo_set_source_rgba(m_context, color[0], color[1], color[2], color[3]);
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
		cairo_text_extents(m_context, text.c_str(), &extents);
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

	void Graphics::Save() {
		cairo_save(m_context);
	}

	void Graphics::Restore() {
		cairo_restore(m_context);
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
