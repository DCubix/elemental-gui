#pragma once

#include "cairo/cairo.h"

#include <cstdint>
#include <memory>
#include <string>

struct NSVGimage;
struct NSVGrasterizer;

namespace gui {
    struct Color;
    class Graphics;
    struct Size;

    enum class ImageType { Bitmap = 0, SVG };

    struct ImageImpl {
        ImageType type{ImageType::Bitmap};
        int width{0}, height{0};
        cairo_surface_t* surface{nullptr};

        struct {
            unsigned char* data{nullptr};
            int stride{0};
        } lockedData;

        NSVGimage* svgImage{nullptr};
        NSVGrasterizer* svgRasterizer{nullptr};
        int rasterizedWidth{0}, rasterizedHeight{0};
        uint32_t strokeColorOverride{0};
        uint32_t fillColorOverride{0};

        // The destructor here handles the actual cleanup
        ~ImageImpl();
    };

    class Image {
        friend class Graphics;

    public:
        Image() = default;
        ~Image() = default;

        Image(const Image&) = default;
        Image& operator=(const Image&) = default;
        Image(Image&&) noexcept = default;
        Image& operator=(Image&&) noexcept = default;

        Image(const std::string& fileName);
        Image(int width, int height);

        int GetWidth() const { return m_impl->width; }
        int GetHeight() const { return m_impl->height; }
        ImageType GetType() const { return m_impl->type; }
        Size GetSize() const;

        void Lock();
        void Unlock();

        Color GetPixel(int x, int y) const;
        void SetPixel(int x, int y, const Color& color);

        void SetPixels(const unsigned char* data, int stride);
        void Resize(int w, int h);

        void WriteToPNG(const std::string& fileName);

        bool IsValid() const;

        bool operator==(const Image& other) const;

    protected:
        std::shared_ptr<ImageImpl> m_impl;

        // Rasterizes the SVG at the specified size and updates the cairo surface
        void RasterizeSVG(int w, int h, uint32_t strokeOverride = 0, uint32_t fillOverride = 0);

        bool IsSVGFile(const std::string& fileName) const;
        void
        RebuildSurfaceFromSVG(int w, int h, uint32_t strokeOverride = 0, uint32_t fillOverride = 0);
    };
} // namespace gui