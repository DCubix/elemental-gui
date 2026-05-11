#include "ImageView.h"

namespace gui {

    ImageView::ImageView()
        : Element() {
        TrackAll(image, scalingMode);
    }

    void ImageView::OnDraw(Graphics& g) {
        if (!image())
            return;

        Size size = GetSize();
        Rectangle clip = GetLocalIntersectedBounds();
        Size pref = GetPreferredSize();

        Rectangle imgRect{0, 0, pref.w, pref.h};
        switch (scalingMode()) {
            case ImageScalingMode::Stretch:
                break;
            case ImageScalingMode::Contain: {
                float imgAspect = static_cast<float>(image()->GetWidth()) /
                                  static_cast<float>(image()->GetHeight());
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
                float imgAspect = static_cast<float>(image()->GetWidth()) /
                                  static_cast<float>(image()->GetHeight());
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
        g.DrawImage(image(), imgRect.x, imgRect.y, imgRect.w, imgRect.h);
        g.ClipPop();
    }

    Size ImageView::GetPreferredSize() const {
        if (image() && IsAutoSize()) {
            return {image()->GetWidth(), image()->GetHeight()};
        }
        return {GetLocalBounds().w, GetLocalBounds().h};
    }

} // namespace gui
