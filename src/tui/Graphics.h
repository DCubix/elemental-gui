#pragma once

#include "SDL3/SDL.h"
#include "cairo/cairo.h"
#include <nlohmann/json.hpp>

#include <functional>
#include <stack>
#include <optional>
#include <concepts>
#include <vector>

#define PI 3.141592654

using Json = nlohmann::json;

namespace tui {
	template <typename T>
	concept IsNumber = std::is_arithmetic_v<T>;

	template <IsNumber T>
	struct Point {
		T x{}, y{};
	};

	using PointF = Point<float>;
	using PointI = Point<int>;

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
		std::optional<Rectangle> Intersect(Rectangle b);

		static Rectangle FromLRTB(int left, int right, int top, int bottom);
		static Rectangle FromWidth(int w);
		static Rectangle FromHeight(int h);
		static Rectangle FromSize(int w, int h);
	};

    struct Color {
        float r, g, b, a;

        std::string ToHex() const;

        Color Darken(float amount) const;
        Color Lighten(float amount) const;

        static Color FromHex(const std::string& hex);
        static Color FromRGBA(float r, float g, float b, float a = 1.0f);
        static Color FromRGB(float r, float g, float b);
        static Color FromHSLA(float h, float s, float l, float a = 1.0f);
        static Color FromHSL(float h, float s, float l);
    };

	class Image {
		friend class Graphics;
	public:
		Image() = default;
		~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		Image(Image&& other) noexcept;
		Image& operator=(Image&& other) noexcept;

		Image(const std::string& fileName);
		Image(int width, int height);

		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }

		void SetPixels(const unsigned char* data, int stride);
		void Resize(int w, int h);

	private:
		int m_width{0}, m_height{0};
		cairo_surface_t *m_surface{nullptr};
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
		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }
		void Clear(int r = 0, int g = 0, int b = 0);

		void LineWidth(float w = 1.0f);
		void Color(float r, float g, float b, float a = 1.0f);
		void Rect(int x, int y, int w, int h);
		void RoundRect(int x, int y, int w, int h, float radius = 0.0f);
		void Line(int x1, int y1, int x2, int y2);

		void Stroke(bool preserve = false);
		void Fill(bool preserve = false);

		void DrawImage(Image* img, int x, int y, int w, int h);
		void DrawShadow(float elevation, int x, int y, int w, int h, float radius);

		void StyledPaint(Json style);
		void StyledRect(int x, int y, int w, int h, Json style);
		void StyledTextBegin(Json style);
		void StyledTextEnd(const std::string& text, int x, int y, float rot = 0.0f);
		cairo_text_extents_t MeasureText(const std::string& text);

		void DrawCharBegin(FontStyle style, const std::string& font, double fontSize);
		int DrawChar(char c, int x, int y);

		void DrawSVG(Json svgStyle, int x, int y, int w, int h);

		void ClipPush(int x, int y, int w, int h);
		void ClipPop();

		void BeginPath();
		void AddPathRect(int x, int y, int w, int h);
		void AddPathPoint(int x, int y);
		void EndPath(bool close = false);

		// Transformations
		void Translate(double tx, double ty);
		void Rotate(double angle);
		void Scale(double sx, double sy);

		void Save();
		void Restore();

		cairo_t* GetCairoContext() const { return Ctx(); }

	private:
		SDL_Renderer *m_renderer;

		SDL_Texture *m_buffer;
		cairo_surface_t *m_surface;
		cairo_t *m_context;
		int m_width, m_height;

		cairo_surface_t *m_measureSurface;
		cairo_t *m_measureContext;

		cairo_t* Ctx() const { return m_context ? m_context : m_measureContext; }

		std::stack<Rectangle> m_clipRects;
		std::vector<PointI> m_pathPoints;

		// Parses a JSON paint object ({"color": [...]} or {"gradient": {...}})
		// and sets it as the cairo source. Returns a pattern that must be destroyed
		// by the caller, or nullptr for solid colors.
		cairo_pattern_t* ApplyPaint(Json paint, int x = 0, int y = 0, int w = 0, int h = 0);
		
		void Draw(DrawFunction func);

		cairo_pattern_t* CreateRoundShadowPattern(float elevation, int x, int y, int w, int h, float radius);
	};
}
