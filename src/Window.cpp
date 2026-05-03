#include "Window.h"

#include "Application.h"
#include "Tooltip.h"

#include <algorithm>

namespace gui {

    Backend& Window::GetBackend() {
        return m_application->GetBackend();
    }

    Window::~Window() {
        if (m_handle && m_application) {
            GetBackend().DestroyRenderBuffer(m_handle);
            GetBackend().DestroyWindow(m_handle);
        }
    }

    Window::Window(const WindowConfig& config)
        : m_config(config) {
        m_graphics = Graphics::CreateGraphics();
    }

    void Window::Resize(uint32_t width, uint32_t height, bool reshape) {
        m_config.width = width;
        m_config.height = height;
        if (reshape) {
            GetBackend().ResizeWindow(m_handle, width, height);
        }
        OnResize();
        ResizeScreenBuffer();
        RequestRedrawAll();
    }

    void Window::Show() {
        auto& b = GetBackend();
        b.ShowWindow(m_handle);
        b.RaiseWindow(m_handle);
        if (m_config.parent) {
            int x, y, pw, ph;
            b.GetWindowPosition(m_config.parent->m_handle, x, y);
            b.GetWindowSize(m_config.parent->m_handle, pw, ph);
            b.SetWindowPosition(
                m_handle,
                x + pw / 2 - (int)m_config.width / 2,
                y + ph / 2 - (int)m_config.height / 2
            );
        }
        RequestRedraw();
    }

    void Window::Hide() {
        GetBackend().HideWindow(m_handle);
    }

    void Window::Close() {
        m_closeRequested = true;
    }

    void Window::RequestRedraw() {
        m_shouldRedraw = true;
    }

    void Window::Focus(Element* e) {
        auto& esys = m_application->GetEventSystem();
        if (m_focused != nullptr) {
            m_focused->m_focused = false;
            esys.Broadcast<BlurEvent>(m_focused);
            if (m_focused->NeedsTextInput())
                StopInput();
        }
        if (e != nullptr) {
            e->m_focused = true;
            esys.Broadcast<FocusEvent>(e);
            if (e->NeedsTextInput())
                StartInput();
        }
        m_focused = e;
    }

    void Window::StartInput() {
        GetBackend().StartTextInput(m_handle);
    }

    void Window::StopInput() {
        GetBackend().StopTextInput(m_handle);
    }

    void Window::ShowPopup(Element* popup) {
        for (auto* p : m_popups) {
            if (p == popup)
                return;
        }
        popup->m_window = this;
        m_popups.push_back(popup);
        RequestRedraw();
    }

    void Window::DismissPopup(Element* popup) {
        m_popups.erase(std::remove(m_popups.begin(), m_popups.end(), popup), m_popups.end());
        RequestRedraw();
    }

    void Window::ScheduleDestroy(Element* el) {
        m_pendingDestroy.push_back(el);
    }

    void Window::SetRoot(Element* root) {
        m_root = root;
        RequestRedraw();
    }

    void Window::SetTitle(const std::string& title) {
        m_config.title = title;
        GetBackend().SetWindowTitle(m_handle, title);
    }

    void Window::SetResizable(bool resizable) {
        m_config.resizable = resizable;
        GetBackend().SetWindowResizable(m_handle, resizable);
    }

    void Window::SetParent(Window* parent) {
        m_config.parent = parent;
        GetBackend().SetWindowParent(m_handle, parent ? parent->m_handle : nullptr);
    }

    void Window::SetWindowStyle(WindowStyle style) {
        m_config.style = style;
        GetBackend().SetWindowStyle(m_handle, style);
    }

    Size Window::GetSize() const {
        int w, h;
        m_application->GetBackend().GetWindowSize(m_handle, w, h);
        return {w, h};
    }

    WindowId Window::GetId() const {
        return m_application->GetBackend().GetWindowId(m_handle);
    }

    void Window::StartTooltip(const std::string& text, Element* anchor) {
        if (m_tooltip && m_tooltip->IsVisible()) {
            DismissPopup(m_tooltip);
            m_tooltip->SetVisible(false);
        }
        m_tooltipAnchor = anchor;
        m_tooltipPendingText = text;
        m_tooltipHoverStart = std::chrono::steady_clock::now();
        m_tooltipShown = false;
    }

    void Window::CancelTooltip() {
        m_tooltipAnchor = nullptr;
        m_tooltipPendingText.clear();
        m_tooltipShown = false;
        if (m_tooltip && m_tooltip->IsVisible()) {
            DismissPopup(m_tooltip);
            m_tooltip->SetVisible(false);
        }
    }

    void Window::ShowTooltipNow() {
        if (!m_tooltipAnchor || m_tooltipPendingText.empty())
            return;

        if (!m_tooltip)
            m_tooltip = &Create<Tooltip>();

        m_tooltip->text = m_tooltipPendingText;
        m_tooltip->SetAutoSize(true);
        m_tooltip->SetVisible(true);
        m_tooltip->Invalidate();

        Size tipSize = m_tooltip->GetPreferredSize();
        Rectangle ab = m_tooltipAnchor->GetBounds();
        Size wsz = GetSize();

        const int gap = 4;
        int tx = ab.x;
        int ty = ab.y + ab.h + gap;

        if (tx + tipSize.w > wsz.w)
            tx = wsz.w - tipSize.w - gap;
        if (tx < 0)
            tx = 0;
        if (ty + tipSize.h > wsz.h)
            ty = ab.y - tipSize.h - gap;
        if (ty < 0)
            ty = gap;

        m_tooltip->SetLocalBounds({tx, ty, tipSize.w, tipSize.h});
        ShowPopup(m_tooltip);
    }

    void Window::Update() {
        if (m_tooltipAnchor && !m_tooltipShown) {
            auto elapsed = std::chrono::steady_clock::now() - m_tooltipHoverStart;
            if (elapsed >= std::chrono::milliseconds(500)) {
                m_tooltipShown = true;
                ShowTooltipNow();
                RequestRedraw();
            }
        }

        if (!m_pendingDestroy.empty()) {
            auto& esys = m_application->GetEventSystem();
            for (auto* el : m_pendingDestroy) {
                if (el == m_focused)
                    m_focused = nullptr;
                esys.UnsubscribeAll(el);
                m_popups.erase(std::remove(m_popups.begin(), m_popups.end(), el), m_popups.end());
                m_elements.erase(
                    std::remove_if(
                        m_elements.begin(),
                        m_elements.end(),
                        [el](const std::unique_ptr<Element>& p) { return p.get() == el; }
                    ),
                    m_elements.end()
                );
            }
            m_pendingDestroy.clear();
            RequestRedraw();
        }

        if (m_shouldRedraw) {
            if (m_root)
                m_root->SetLocalBounds(Rectangle(0, 0, m_config.width, m_config.height));
            Redraw();
            m_shouldRedraw = false;
        }
    }

    void Window::ResizeScreenBuffer() {
        GetBackend().CreateRenderBuffer(m_handle, m_config.width, m_config.height);
    }

    void Window::Redraw() {
        m_graphics.SetupDrawing(m_config.width, m_config.height);

        if (m_root) {
            Json windowStyle = GetApp()->GetStyle()["Window"];
            m_graphics.StyledRect(0, 0, m_config.width, m_config.height, windowStyle);
        }

        for (size_t i = 0; i < m_elements.size(); i++) {
            auto* el = m_elements[i].get();
            if (el->GetParent() != nullptr)
                continue;
            if (!el->IsVisible())
                continue;
            if (el->IsDirty()) {
                const auto& lb = el->GetLocalBounds();
                m_graphics.Save();
                m_graphics.Translate(lb.x, lb.y);
                el->OnDraw(m_graphics);
                m_graphics.Restore();
                el->m_dirty = false;
            }
        }
        for (auto* popup : m_popups) {
            if (!popup->IsVisible())
                continue;
            const auto& lb = popup->GetLocalBounds();
            m_graphics.Save();
            m_graphics.Translate(lb.x, lb.y);
            popup->OnDraw(m_graphics);
            m_graphics.Restore();
            popup->m_dirty = false;
        }
        m_graphics.Flush();

        unsigned char* data = cairo_image_surface_get_data(m_graphics.GetCairoSurface());
        int stride = cairo_image_surface_get_stride(m_graphics.GetCairoSurface());

        GetBackend().PresentFrame(m_handle, data, stride, m_config.width, m_config.height);
    }

    void Window::RequestRedrawAll() {
        for (auto&& e : m_elements) {
            e->Invalidate();
        }
    }

    void Window::HookEventsUp(Element* el) {
        auto& esys = m_application->GetEventSystem();
        esys.Subscribe(el, EventType::MouseButton);
        esys.Subscribe(el, EventType::MouseMotion);
        esys.Subscribe(el, EventType::TextInput);
        esys.Subscribe(el, EventType::Focus);
        esys.Subscribe(el, EventType::Blur);
        esys.Subscribe(el, EventType::Key);
        esys.Subscribe(el, EventType::Scroll);
    }

} // namespace gui
