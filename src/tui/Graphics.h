#pragma once

#include "cairo/cairo.h"
#include <nlohmann/json.hpp>

#include "Image.h"

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
		Graphics();

		void Clear(float r, float g, float b, float a = 1.0f);

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

		void ClipPushRect(int x, int y, int w, int h);
		void ClipPushRoundRect(int x, int y, int w, int h, float radius);
		void ClipPushPath(std::function<void()> pathFunc);
		void ClipPop();

		void GetStyledPath(Json style, int x, int y, int w, int h);

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
		cairo_surface_t* GetCairoSurface() const { return m_surface; }

		void BeginDrawing(uint width, uint height);
		void Flush();
		void EndDrawing();

	private:
		cairo_surface_t *m_surface;
		cairo_t *m_context;

		cairo_surface_t *m_measureSurface;
		cairo_t *m_measureContext;

		cairo_t* Ctx() const { return m_context ? m_context : m_measureContext; }

		uint32_t m_clipDepth{0};
		std::vector<PointI> m_pathPoints;

		// Parses a JSON paint object ({"color": [...]} or {"gradient": {...}})
		// and sets it as the cairo source. Returns a pattern that must be destroyed
		// by the caller, or nullptr for solid colors.
		cairo_pattern_t* ApplyPaint(Json paint, int x = 0, int y = 0, int w = 0, int h = 0);
		cairo_pattern_t* CreateRoundShadowPattern(float elevation, int x, int y, int w, int h, float radius);
	};
}
