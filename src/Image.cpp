#include "Image.h"

#include "Graphics.h"
#include "nanosvg.h"
#include "nanosvgrast.h"

#include <cstring>
#include <filesystem>
#include <vector>

namespace gui {
    Image::~Image() {
        if (m_surface) {
            cairo_surface_destroy(m_surface);
            m_surface = nullptr;
        }
        if (m_svgRasterizer) {
            nsvgDeleteRasterizer(m_svgRasterizer);
            m_svgRasterizer = nullptr;
        }
        if (m_svgImage) {
            nsvgDelete(m_svgImage);
            m_svgImage = nullptr;
        }
    }

    Image::Image(Image&& other) noexcept
        : m_width(other.m_width),
          m_height(other.m_height),
          m_surface(other.m_surface),
          m_svgImage(other.m_svgImage),
          m_svgRasterizer(other.m_svgRasterizer),
          m_rasterizedWidth(other.m_rasterizedWidth),
          m_rasterizedHeight(other.m_rasterizedHeight),
          m_type(other.m_type),
          m_strokeColorOverride(other.m_strokeColorOverride),
          m_fillColorOverride(other.m_fillColorOverride) {
        other.m_surface = nullptr;
        other.m_width = 0;
        other.m_height = 0;
        other.m_svgImage = nullptr;
        other.m_svgRasterizer = nullptr;
        other.m_rasterizedWidth = 0;
        other.m_rasterizedHeight = 0;
    }

    Image& Image::operator=(Image&& other) noexcept {
        if (this != &other) {
            if (m_surface) {
                cairo_surface_destroy(m_surface);
            }
            if (m_svgRasterizer) {
                nsvgDeleteRasterizer(m_svgRasterizer);
            }
            if (m_svgImage) {
                nsvgDelete(m_svgImage);
            }
            m_surface = other.m_surface;
            m_width = other.m_width;
            m_height = other.m_height;
            m_svgImage = other.m_svgImage;
            m_svgRasterizer = other.m_svgRasterizer;
            m_rasterizedWidth = other.m_rasterizedWidth;
            m_rasterizedHeight = other.m_rasterizedHeight;
            m_type = other.m_type;
            m_strokeColorOverride = other.m_strokeColorOverride;
            m_fillColorOverride = other.m_fillColorOverride;
            other.m_surface = nullptr;
            other.m_width = 0;
            other.m_height = 0;
            other.m_svgImage = nullptr;
            other.m_svgRasterizer = nullptr;
            other.m_rasterizedWidth = 0;
            other.m_rasterizedHeight = 0;
        }
        return *this;
    }

    Image::Image(const std::string& fileName) {
        if (IsSVGFile(fileName)) {
            m_type = Type::SVG;
            m_svgImage = nsvgParseFromFile(fileName.c_str(), "px", 96);
            if (m_svgImage) {
                m_width = static_cast<int>(m_svgImage->width);
                m_height = static_cast<int>(m_svgImage->height);
                m_svgRasterizer = nsvgCreateRasterizer();

                const uint32_t defaultStrokeColor = 0xFF000000; // Opaque black
                const uint32_t defaultFillColor = 0xFF000000;   // Opaque black
                RebuildSurfaceFromSVG(m_width, m_height, defaultStrokeColor, defaultFillColor);
            }
        } else {
            m_type = Type::Bitmap;
            m_surface = cairo_image_surface_create_from_png(fileName.c_str());
            m_width = cairo_image_surface_get_width(m_surface);
            m_height = cairo_image_surface_get_height(m_surface);
        }
    }

    Image::Image(int width, int height) {
        m_width = width;
        m_height = height;
        m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    }

    Color Image::GetPixel(int x, int y) const {
        cairo_surface_flush(m_surface);
        unsigned char* data = cairo_image_surface_get_data(m_surface);
        int stride = cairo_image_surface_get_stride(m_surface);
        // Assuming ARGB format (4 bytes per pixel)
        unsigned char* pixel = data + y * stride + x * 4;
        float b = pixel[0] / 255.0f;
        float g = pixel[1] / 255.0f;
        float r = pixel[2] / 255.0f;
        float a = pixel[3] / 255.0f;
        return Color(r, g, b, a);
    }

    void Image::SetPixel(int x, int y, const Color& color) {
        cairo_surface_flush(m_surface);
        unsigned char* data = cairo_image_surface_get_data(m_surface);
        int stride = cairo_image_surface_get_stride(m_surface);
        // Assuming ARGB format (4 bytes per pixel)
        unsigned char* pixel = data + y * stride + x * 4;
        pixel[0] = static_cast<unsigned char>(color.b * 255.0f);
        pixel[1] = static_cast<unsigned char>(color.g * 255.0f);
        pixel[2] = static_cast<unsigned char>(color.r * 255.0f);
        pixel[3] = static_cast<unsigned char>(color.a * 255.0f);
        cairo_surface_mark_dirty(m_surface);
    }

    void Image::SetPixels(const unsigned char* data, int stride) {
        if (!m_surface)
            return;

        cairo_surface_flush(m_surface);

        unsigned char* surfaceData = cairo_image_surface_get_data(m_surface);
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

    void Image::Resize(int w, int h) {
        if (m_surface) {
            cairo_surface_destroy(m_surface);
            m_surface = nullptr;
        }

        m_width = w;
        m_height = h;

        m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    }

    bool Image::IsValid() const {
        if (m_width <= 0 || m_height <= 0)
            return false;

        bool valid = (m_surface != nullptr);
        if (m_type == Type::SVG) {
            valid = valid && (m_svgImage != nullptr) && (m_svgRasterizer != nullptr);
        }
        return valid;
    }

    void Image::RasterizeSVG(int w, int h, uint32_t strokeOverride, uint32_t fillOverride) {
        if (m_type != Type::SVG)
            return;
        if (!m_svgImage || !m_svgRasterizer)
            return;
        if (w == m_rasterizedWidth && h == m_rasterizedHeight &&
            strokeOverride == m_strokeColorOverride && fillOverride == m_fillColorOverride)
            return; // No need to re-rasterize if size and color overrides are unchanged
        RebuildSurfaceFromSVG(w, h, strokeOverride, fillOverride);
    }

    bool Image::IsSVGFile(const std::string& fileName) const {
        std::filesystem::path path(fileName);
        return path.extension() == ".svg";
    }

    void
    Image::RebuildSurfaceFromSVG(int w, int h, uint32_t strokeOverride, uint32_t fillOverride) {
        if (!m_svgImage || !m_svgRasterizer)
            return;

        for (NSVGshape* shape = m_svgImage->shapes; shape; shape = shape->next) {
            if (shape->fill.type == NSVG_PAINT_COLOR) {
                shape->fill.color = fillOverride;
            }
            if (shape->stroke.type == NSVG_PAINT_COLOR) {
                shape->stroke.color = strokeOverride;
            }
        }

        if (m_surface) {
            cairo_surface_destroy(m_surface);
            m_surface = nullptr;
        }

        float scaleX = static_cast<float>(w) / m_svgImage->width;
        float scaleY = static_cast<float>(h) / m_svgImage->height;
        float scale = std::min(scaleX, scaleY);

        std::vector<unsigned char> rgba(w * h * 4);
        nsvgRasterize(m_svgRasterizer, m_svgImage, 0, 0, scale, rgba.data(), w, h, w * 4);

        m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        cairo_surface_flush(m_surface);

        unsigned char* dst = cairo_image_surface_get_data(m_surface);
        int dstStride = cairo_image_surface_get_stride(m_surface);

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                unsigned char* src = rgba.data() + (y * w + x) * 4;
                unsigned char* d = &dst[y * dstStride + x * 4];
                unsigned char r = src[0], g = src[1], b = src[2], a = src[3];
                d[0] = (b * a) / 255;
                d[1] = (g * a) / 255;
                d[2] = (r * a) / 255;
                d[3] = a;
            }
        }
        cairo_surface_mark_dirty(m_surface);
        m_rasterizedWidth = w;
        m_rasterizedHeight = h;
        m_width = w;
        m_height = h;
        m_strokeColorOverride = strokeOverride;
        m_fillColorOverride = fillOverride;
    }
} // namespace gui