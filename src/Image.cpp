#include "Image.h"

#include "Graphics.h"
#include "nanosvg.h"
#include "nanosvgrast.h"

#include <cstring>
#include <filesystem>
#include <vector>

namespace gui {

    Image::Image(const std::string& fileName)
        : m_impl(std::make_shared<ImageImpl>()) {
        if (IsSVGFile(fileName)) {
            m_impl->type = ImageType::SVG;
            m_impl->svgImage = nsvgParseFromFile(fileName.c_str(), "px", 96);
            if (m_impl->svgImage) {
                m_impl->width = static_cast<int>(m_impl->svgImage->width);
                m_impl->height = static_cast<int>(m_impl->svgImage->height);
                m_impl->svgRasterizer = nsvgCreateRasterizer();

                const uint32_t defaultStrokeColor = 0xFF000000; // Opaque black
                const uint32_t defaultFillColor = 0xFF000000;   // Opaque black
                RebuildSurfaceFromSVG(
                    m_impl->width,
                    m_impl->height,
                    defaultStrokeColor,
                    defaultFillColor
                );
            }
        } else {
            m_impl->type = ImageType::Bitmap;
            m_impl->surface = cairo_image_surface_create_from_png(fileName.c_str());
            m_impl->width = cairo_image_surface_get_width(m_impl->surface);
            m_impl->height = cairo_image_surface_get_height(m_impl->surface);
        }
    }

    Image::Image(int width, int height)
        : m_impl(std::make_shared<ImageImpl>()) {
        m_impl->width = width;
        m_impl->height = height;
        m_impl->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    }

    Size Image::GetSize() const {
        return {m_impl->width, m_impl->height};
    }

    void Image::Lock() {
        if (m_impl->lockedData.data) {
            throw std::runtime_error("Image is already locked.");
        }
        cairo_surface_flush(m_impl->surface);
        m_impl->lockedData.data = cairo_image_surface_get_data(m_impl->surface);
        m_impl->lockedData.stride = cairo_image_surface_get_stride(m_impl->surface);
    }

    void Image::Unlock() {
        if (!m_impl->lockedData.data) {
            throw std::runtime_error("Image was not locked.");
        }
        cairo_surface_mark_dirty(m_impl->surface);
        m_impl->lockedData.data = nullptr;
        m_impl->lockedData.stride = 0;
    }

    Color Image::GetPixel(int x, int y) const {
        if (!m_impl->lockedData.data) {
            throw std::runtime_error("Image data must be locked.");
        }
        // Assuming ARGB format (4 bytes per pixel)
        unsigned char* pixel = m_impl->lockedData.data + y * m_impl->lockedData.stride + x * 4;
        float b = pixel[0] / 255.0f;
        float g = pixel[1] / 255.0f;
        float r = pixel[2] / 255.0f;
        float a = pixel[3] / 255.0f;
        return Color(r, g, b, a);
    }

    void Image::SetPixel(int x, int y, const Color& color) {
        if (!m_impl->lockedData.data) {
            throw std::runtime_error("Image data must be locked.");
        }
        if (x < 0 || y < 0 || x >= m_impl->width || y >= m_impl->height) {
            return;
        }
        // Assuming ARGB format (4 bytes per pixel)
        unsigned char* pixel = m_impl->lockedData.data + y * m_impl->lockedData.stride + x * 4;
        pixel[0] = static_cast<unsigned char>(color.b * 255.0f);
        pixel[1] = static_cast<unsigned char>(color.g * 255.0f);
        pixel[2] = static_cast<unsigned char>(color.r * 255.0f);
        pixel[3] = static_cast<unsigned char>(color.a * 255.0f);
    }

    void Image::SetPixels(const unsigned char* data, int stride) {
        if (!m_impl->surface) {
            throw std::runtime_error("Image data must be locked.");
        }
        for (int y = 0; y < m_impl->height; y++) {
            std::memcpy(
                m_impl->lockedData.data + y * m_impl->lockedData.stride,
                data + y * stride,
                std::min(m_impl->lockedData.stride, stride)
            );
        }
    }

    void Image::Resize(int w, int h) {
        if (m_impl->surface) {
            cairo_surface_destroy(m_impl->surface);
            m_impl->surface = nullptr;
        }

        m_impl->width = w;
        m_impl->height = h;

        m_impl->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    }

    void Image::WriteToPNG(const std::string& fileName) {
        cairo_surface_write_to_png(m_impl->surface, fileName.c_str());
    }

    bool Image::IsValid() const {
        if (m_impl->width <= 0 || m_impl->height <= 0)
            return false;

        bool valid = (m_impl->surface != nullptr);
        if (m_impl->type == ImageType::SVG) {
            valid = valid && (m_impl->svgImage != nullptr) && (m_impl->svgRasterizer != nullptr);
        }
        return valid;
    }

    bool Image::operator==(const Image& other) const {
        return m_impl == other.m_impl;
    }

    void Image::RasterizeSVG(int w, int h, uint32_t strokeOverride, uint32_t fillOverride) {
        if (m_impl->type != ImageType::SVG)
            return;
        if (!m_impl->svgImage || !m_impl->svgRasterizer)
            return;
        if (w == m_impl->rasterizedWidth && h == m_impl->rasterizedHeight &&
            strokeOverride == m_impl->strokeColorOverride &&
            fillOverride == m_impl->fillColorOverride)
            return; // No need to re-rasterize if size and color overrides are unchanged
        RebuildSurfaceFromSVG(w, h, strokeOverride, fillOverride);
    }

    bool Image::IsSVGFile(const std::string& fileName) const {
        std::filesystem::path path(fileName);
        return path.extension() == ".svg";
    }

    void
    Image::RebuildSurfaceFromSVG(int w, int h, uint32_t strokeOverride, uint32_t fillOverride) {
        if (!m_impl->svgImage || !m_impl->svgRasterizer)
            return;

        for (NSVGshape* shape = m_impl->svgImage->shapes; shape; shape = shape->next) {
            if (shape->fill.type == NSVG_PAINT_COLOR) {
                shape->fill.color = fillOverride;
            }
            if (shape->stroke.type == NSVG_PAINT_COLOR) {
                shape->stroke.color = strokeOverride;
            }
        }

        if (m_impl->surface) {
            cairo_surface_destroy(m_impl->surface);
            m_impl->surface = nullptr;
        }

        float scaleX = static_cast<float>(w) / m_impl->svgImage->width;
        float scaleY = static_cast<float>(h) / m_impl->svgImage->height;
        float scale = std::min(scaleX, scaleY);

        std::vector<unsigned char> rgba(w * h * 4);
        nsvgRasterize(
            m_impl->svgRasterizer,
            m_impl->svgImage,
            0,
            0,
            scale,
            rgba.data(),
            w,
            h,
            w * 4
        );

        m_impl->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        cairo_surface_flush(m_impl->surface);

        unsigned char* dst = cairo_image_surface_get_data(m_impl->surface);
        int dstStride = cairo_image_surface_get_stride(m_impl->surface);

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
        cairo_surface_mark_dirty(m_impl->surface);
        m_impl->rasterizedWidth = w;
        m_impl->rasterizedHeight = h;
        m_impl->width = w;
        m_impl->height = h;
        m_impl->strokeColorOverride = strokeOverride;
        m_impl->fillColorOverride = fillOverride;
    }

    ImageImpl::~ImageImpl() {
        if (surface)
            cairo_surface_destroy(surface);
        if (svgRasterizer)
            nsvgDeleteRasterizer(svgRasterizer);
        if (svgImage)
            nsvgDelete(svgImage);
    }
} // namespace gui