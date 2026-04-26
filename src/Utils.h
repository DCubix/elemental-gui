#pragma once

#include <cstdint>
#include <functional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace gui::utils {
    using uint = uint32_t;

    using VoidCallback = std::function<void()>;

    template <typename T>
    using ValueChanged = std::function<void(const T&)>;

    enum class ButtonState { Normal = 0, Hover, Click };

    enum class Alignment {
        TopLeft = 0,
        TopCenter,
        TopRight,
        MiddleLeft,
        MiddleCenter,
        MiddleRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
        AlignmentCount
    };

    enum class Direction { Horizontal = 0, Vertical };

    struct Range {
        float minimum, maximum;

        Range() = default;
        Range(float minv, float maxv)
            : minimum(minv),
              maximum(maxv) {}

        float Normalized(float value);
        float Remap(Range other, float value);
        float Constrain(float value);
    };

    inline std::string formatHelper(const std::string& string_to_update, const size_t) {
        return string_to_update;
    }

    template <typename T, typename... Args>
    inline std::string formatHelper(
        const std::string& string_to_update,
        const size_t index_to_replace,
        T&& val,
        Args&&... args
    ) {
        std::regex pattern{"{" + std::to_string(index_to_replace) + "}"};
        std::string replacement_string{(std::ostringstream{} << val).str()};
        return formatHelper(
            std::regex_replace(string_to_update, pattern, replacement_string),
            index_to_replace + 1,
            std::forward<Args>(args)...
        );
    }

    template <typename... Args>
    inline std::string Format(const std::string& fmt, Args&&... args) {
        return formatHelper(fmt, 1, std::forward<Args>(args)...);
    }

    std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter);
} // namespace gui::utils