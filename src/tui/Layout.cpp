#include "Layout.h"

namespace tui {
    const EdgeInsets EdgeInsets::Zero = EdgeInsets{ 0, 0, 0, 0 };

    Rectangle EdgeInsets::Apply(const Rectangle& rect) const {
    	return Rectangle{
    		static_cast<int>(rect.x + left),
    		static_cast<int>(rect.y + top),
    		static_cast<int>(rect.w - GetHorizontal()),
    		static_cast<int>(rect.h - GetVertical())
    	};
    }

    EdgeInsets EdgeInsets::All(uint32_t v)
    {
        return EdgeInsets{v, v, v, v};
    }

    EdgeInsets EdgeInsets::Symmetric(uint32_t horizontal, uint32_t vertical)
    {
        return EdgeInsets{horizontal, vertical, horizontal, vertical};
    }

    EdgeInsets EdgeInsets::Only(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
    {
        return EdgeInsets{left, top, right, bottom};
    }
    
    EdgeInsets EdgeInsets::FromStyle(const Json &style)
    {
        // 1. handle single value (e.g. "padding": 10)
        if (style.is_number()) {
            uint32_t v = style.get<uint32_t>();
            return EdgeInsets::All(v);
        }

        // 2. handle array (e.g. "padding": [10, 20])
        if (style.is_array()) {
            if (style.size() == 2) {
                uint32_t horizontal = style[0].get<uint32_t>();
                uint32_t vertical = style[1].get<uint32_t>();
                return EdgeInsets::Symmetric(horizontal, vertical);
            } else if (style.size() == 4) {
                uint32_t left = style[0].get<uint32_t>();
                uint32_t top = style[1].get<uint32_t>();
                uint32_t right = style[2].get<uint32_t>();
                uint32_t bottom = style[3].get<uint32_t>();
                return EdgeInsets::Only(left, top, right, bottom);
            }
        }

        // 3. handle object (e.g. "padding": {"left": 10, "top": 20, "right": 30, "bottom": 40})
        if (style.is_object()) {
            if (style.contains("left") && style.contains("top") && style.contains("right") && style.contains("bottom")) {
                uint32_t left = style["left"].get<uint32_t>();
                uint32_t top = style["top"].get<uint32_t>();
                uint32_t right = style["right"].get<uint32_t>();
                uint32_t bottom = style["bottom"].get<uint32_t>();
                return EdgeInsets::Only(left, top, right, bottom);
            } else if (style.contains("horizontal") && style.contains("vertical")) {
                uint32_t horizontal = style["horizontal"].get<uint32_t>();
                uint32_t vertical = style["vertical"].get<uint32_t>();
                return EdgeInsets::Symmetric(horizontal, vertical);
            }
        }

        return EdgeInsets::Zero;
    }
}