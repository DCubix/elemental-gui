#pragma once

#include <cassert>
#include <cstdint>
#include <format>
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

    template <typename T>
    constexpr bool is_nullable_v =
        std::is_pointer_v<T> || std::is_same_v<T, std::nullptr_t> || requires(T t) {
            { t = nullptr } -> std::same_as<T&>;
        } && !std::is_constructible_v<T, const char*>;

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
              maximum(maxv) {
            assert(minimum <= maximum && "Range: minimum must be <= maximum");
        }

        float Normalized(float value);
        float Remap(Range other, float value);
        float Constrain(float value);
    };

    template <typename... Args>
    std::string Format(std::string_view fmt, Args&&... args) {
        return std::vformat(fmt, std::make_format_args(args...));
    }

    std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter);
} // namespace gui::utils