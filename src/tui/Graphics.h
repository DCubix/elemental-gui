#ifndef TUI_GRAPHICS_H
#define TUI_GRAPHICS_H

#include "SDL3/SDL.h"
#include "cairo/cairo.h"
#include "json.hpp"

#include <functional>
#include <stack>
#include <optional>

#define PI 3.141592654

using Json = nlohmann::json;

namespace tui {
	struct Point {
		int x{}, y{};
	};

	struct Size {
		int w, h;
	};

	struct Rectangle {
		int x, y, w, h;

		Rectangle() = default;
		Rectangle(int x, int y, int w, int h)
			: x(x), y(y), w(w), h(h)
		{}

		bool HasPoint(int x, int y);
		bool Intersects(Rectangle b);
		std::optional<Rectangle> GetIntersected(Rectangle b);
	};

	class Image {
		friend class Graphics;
	public:
		Image() = default;
		~Image();
		Image(const std::string& fileName);

		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }

	private:
		int m_width, m_height;
		cairo_surface_t *m_surface;
	};

	enum class FontStyle {
		Normal = 0,
		Bold,
		Italic,
		BoldItalic
	};

	class Graphics;
	using DrawFunction = std::function<void(Graphics&)>;

	class Graphics {
		friend class Application;
	public:
		Graphics() = default;
		Graphics(SDL_Renderer* ren);

		void SetViewport(int w, int h);
		void Clear(int r = 0, int g = 0, int b = 0);

		void LineWidth(float w = 1.0f);
		void Color(float r, float g, float b, float a = 1.0f);
		void Rect(int x, int y, int w, int h);
		void RoundRect(int x, int y, int w, int h, float radius = 0.0f);
		void Line(int x1, int y1, int x2, int y2);

		void Stroke(bool preserve = false);
		void Fill(bool preserve = false);

		void DrawImage(Image* img, int x, int y, int w, int h);

		void StyledPaint(Json style);
		void StyledRect(int x, int y, int w, int h, Json style);
		void StyledTextBegin(Json style);
		void StyledTextEnd(const std::string& text, int x, int y, float rot = 0.0f);
		cairo_text_extents_t MeasureText(const std::string& text);

		void DrawCharBegin(FontStyle style, const std::string& font, double fontSize);
		int DrawChar(char c, int x, int y);

		void ClipPush(int x, int y, int w, int h);
		void ClipPop();

		void BeginPath();
		void AddPathRect(int x, int y, int w, int h);
		void AddPathPoint(int x, int y);
		void EndPath(bool close = false);

		void Save();
		void Restore();

	private:
		SDL_Renderer *m_renderer;

		SDL_Texture *m_buffer;
		cairo_surface_t *m_surface;
		cairo_t *m_context;
		int m_width, m_height;

		std::stack<Rectangle> m_clipRects;
		std::vector<Point> m_pathPoints;

		void Draw(DrawFunction func);
	};
}

#endif // TUI_GRAPHICS_H
