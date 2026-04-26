#pragma once

#include <cstdint>
#include <string>

namespace gui {

    class Window;

    enum class WindowStyle { TopLevel, Modal, Utility, Borderless, Fullscreen };

    struct WindowConfig {
        std::string title;
        uint32_t width{800};
        uint32_t height{600};
        bool resizable{false};
        WindowStyle style{WindowStyle::TopLevel};
        Window* parent{nullptr};
    };

} // namespace gui
