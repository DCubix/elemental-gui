#ifndef TUI_GRAPHICS_H
#define TUI_GRAPHICS_H

#include "SDL2/SDL.h"
#include "cairo/cairo.h"
#include "json.hpp"

#include <functional>
#include <stack>

#define PI 3.141592654

using Json = nlohmann::json;

namespace tui {
	struct Point {
		int x{}, y{};
	};

	struct Size {
		int w, h;
	};

	struct Rect {
		int x, y, w, h;

		Rect() = default;
		Rect(int x, int y, int w, int h)
			: x(x), y(y), w(w), h(h)
		{}

		bool hasPoint(int x, int y);
		bool intersects(Rect b);
		std::optional<Rect> getIntersected(Rect b);
	};

	class Image {
		friend class Graphics;
	public:
		Image() = default;
		~Image();
		Image(const std::string& fileName);

		int width() const { return m_width; }
		int height() const { return m_height; }

	private:
		int m_width, m_height;
		cairo_surface_t *m_surface;
	};

	enum FontStyle {
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

		void setViewport(int w, int h);
		void clear(int r = 0, int g = 0, int b = 0);

		void lineWidth(float w = 1.0f);
		void color(float r, float g, float b, float a = 1.0f);
		void rect(int x, int y, int w, int h);
		void roundRect(int x, int y, int w, int h, float radius = 0.0f);
		void line(int x1, int y1, int x2, int y2);

		void stroke(bool preserve = false);
		void fill(bool preserve = false);

		void image(Image* img, int x, int y, int w, int h);

		void styledPaint(Json style);
		void styledRect(int x, int y, int w, int h, Json style);
		void styledTextBegin(Json style);
		void styledTextEnd(const std::string& text, int x, int y, float rot = 0.0f);
		cairo_text_extents_t measureText(const std::string& text);

		void drawCharBegin(FontStyle style, const std::string& font, double fontSize);
		int drawChar(char c, int x, int y);

		void clipPush(int x, int y, int w, int h);
		void clipPop();

		void beginPath();
		void addPathRect(int x, int y, int w, int h);
		void addPathPoint(int x, int y);
		void endPath(bool close = false);

		void save();
		void restore();

	private:
		SDL_Renderer *m_renderer;

		SDL_Texture *m_buffer;
		cairo_surface_t *m_surface;
		cairo_t *m_context;
		int m_width, m_height;

		std::stack<Rect> m_clipRects;
		std::vector<Point> m_pathPoints;

		void draw(DrawFunction func);
	};
}

#endif // TUI_GRAPHICS_H
