#pragma once

#include "cairo/cairo.h"
#include <string>
#include <cstdint>

struct NSVGimage;
struct NSVGrasterizer;

namespace tui {
    struct Color;
    class Graphics;

    class Image {
		friend class Graphics;
	public:
        enum class Type {
            Bitmap = 0,
            SVG
        };

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
        Type GetType() const { return m_type; }

        Color GetPixel(int x, int y) const;
        void SetPixel(int x, int y, const Color& color);

		void SetPixels(const unsigned char* data, int stride);
		void Resize(int w, int h);

        bool IsValid() const;

	protected:
        Type m_type{Type::Bitmap};
		int m_width{0}, m_height{0};
		cairo_surface_t *m_surface{nullptr};

        // SVG-specific
        NSVGimage* m_svgImage{nullptr};
        NSVGrasterizer* m_svgRasterizer{nullptr};
        int m_rasterizedWidth{0}, m_rasterizedHeight{0};
        uint32_t m_strokeColorOverride{0};
        uint32_t m_fillColorOverride{0};

        // Rasterizes the SVG at the specified size and updates the cairo surface
        void RasterizeSVG(int w, int h, uint32_t strokeOverride = 0, uint32_t fillOverride = 0);

        bool IsSVGFile(const std::string& fileName) const;
        void RebuildSurfaceFromSVG(int w, int h, uint32_t strokeOverride = 0, uint32_t fillOverride = 0);
	};
}