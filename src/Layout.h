#pragma once

#include "Graphics.h"

#include <cstdint>

namespace gui {

    struct EdgeInsets {
        uint32_t left{0}, top{0}, right{0}, bottom{0};

        uint32_t GetHorizontal() const { return left + right; }
        uint32_t GetVertical() const { return top + bottom; }

        Rectangle Apply(const Rectangle& rect) const;

        static EdgeInsets All(uint32_t v = 0);
        static EdgeInsets Symmetric(uint32_t horizontal = 0, uint32_t vertical = 0);
        static EdgeInsets
        Only(uint32_t left = 0, uint32_t top = 0, uint32_t right = 0, uint32_t bottom = 0);

        static EdgeInsets FromStyle(const Json& style);

        static const EdgeInsets Zero;
    };

    class Layout {
    public:
        virtual ~Layout() = default;
        virtual void Apply(const Rectangle& bounds) = 0;
        virtual Size GetLaidOutSize() const = 0;

        EdgeInsets GetPadding() const { return m_padding; }
        void SetPadding(const EdgeInsets& p) { m_padding = p; }

        int GetGap() const { return m_gap; }
        void SetGap(int g) { m_gap = g; }

    protected:
        EdgeInsets m_padding{0, 0, 0, 0};
        uint32_t m_gap{0};
    };
} // namespace gui
