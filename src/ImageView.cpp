#include "ImageView.h"

namespace gui {

    ImageView::ImageView()
        : m_image(nullptr) {
        scalingMode.SetOnUpdate([this]{ Invalidate(); });
    }

    void ImageView::OnDraw(Graphics& g) {
        if (!m_image)
            return;

        Size size = GetSize();
        Rectangle clip = GetLocalIntersectedBounds();
        Size pref = GetPreferredSize();

        Rectangle imgRect{0, 0, pref.w, pref.h};
        switch (scalingMode()) {
            case ImageScalingMode::Stretch:
                break;
            case ImageScalingMode::Contain: {
                float imgAspect = static_cast<float>(m_image->GetWidth()) /
                                  static_cast<float>(m_image->GetHeight());
                float boxAspect = static_cast<float>(size.w) / static_cast<float>(size.h);
                if (imgAspect > boxAspect) {
                    imgRect.w = size.w;
                    imgRect.h = static_cast<int>(size.w / imgAspect);
                    imgRect.y = (size.h - imgRect.h) / 2;
                } else {
                    imgRect.h = size.h;
                    imgRect.w = static_cast<int>(size.h * imgAspect);
                    imgRect.x = (size.w - imgRect.w) / 2;
                }
                break;
            }
            case ImageScalingMode::Cover: {
                float imgAspect = static_cast<float>(m_image->GetWidth()) /
                                  static_cast<float>(m_image->GetHeight());
                float boxAspect = static_cast<float>(size.w) / static_cast<float>(size.h);
                if (imgAspect < boxAspect) {
                    imgRect.w = size.w;
                    imgRect.h = static_cast<int>(size.w / imgAspect);
                    imgRect.y = (size.h - imgRect.h) / 2;
                } else {
                    imgRect.h = size.h;
                    imgRect.w = static_cast<int>(size.h * imgAspect);
                    imgRect.x = (size.w - imgRect.w) / 2;
                }
                break;
            }
        }

        g.ClipPushRect(clip.x, clip.y, clip.w, clip.h);
        g.DrawImage(m_image, imgRect.x, imgRect.y, imgRect.w, imgRect.h);
        g.ClipPop();
    }

    Size ImageView::GetPreferredSize() const {
        if (m_image && IsAutoSize()) {
            return {m_image->GetWidth(), m_image->GetHeight()};
        }
        return {GetLocalBounds().w, GetLocalBounds().h};
    }

} // namespace gui
