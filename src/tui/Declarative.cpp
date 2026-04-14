#include "Declarative.h"

#include "Panel.h"
#include "FlexLayout.h"
#include "Button.h"
#include "Edit.h"
#include "ScrollView.h"
#include "Slider.h"

namespace tui::declarative {

    void ElementSetup(Element& element, const ElementProps& props) {
        if (props.bounds.w > 0) element.SetSize({ props.bounds.w, element.GetSize().h });
        if (props.bounds.h > 0) element.SetSize({ element.GetSize().w, props.bounds.h });
        element.SetPosition({ props.bounds.x, props.bounds.y });
        element.SetTag(props.tag);
        element.SetAutoSize(props.autoSize);
        element.SetFlexGrow(props.flexGrow);
        element.SetStyle(props.style);
        element.SetEnabled(props.enabled);
    }

    WidgetDesc Column(const ColumnProps& props, const std::vector<WidgetDesc>& children) {
        return [props, children](Application& app) -> Element* {
            auto& panel = app.Create<Panel>();
            ElementSetup(panel, props.base);

            panel.SetBackgroundVisible(props.showBackground);

            auto* flex = panel.GetLayout<FlexLayout>();
            flex->SetDirection(FlexDirection::Column);
            flex->SetAlignItems(props.align);
            flex->SetJustifyContent(props.justify);
            flex->SetGap(props.gap);
            flex->SetPadding(props.padding);

            for (auto&& childDesc : children) {
                Element* child = childDesc(app);
                panel.Add(child);
            }

            return &panel;
        };
    }

    WidgetDesc Row(const RowProps &props, const std::vector<WidgetDesc> &children)
    {
        return [props, children](Application& app) -> Element* {
            auto& panel = app.Create<Panel>();
            ElementSetup(panel, props.base);

            panel.SetBackgroundVisible(props.showBackground);

            auto* flex = panel.GetLayout<FlexLayout>();
            flex->SetDirection(FlexDirection::Row);
            flex->SetAlignItems(props.align);
            flex->SetJustifyContent(props.justify);
            flex->SetGap(props.gap);
            flex->SetPadding(props.padding);

            for (auto&& childDesc : children) {
                Element* child = childDesc(app);
                panel.Add(child);
            }

            return &panel;
        };
    }
    
    WidgetDesc Text(const std::string &text, const TextProps &props)
    {
        return [text, props](Application& app) -> Element* {
            auto& label = app.Create<Label>();
            ElementSetup(label, props.base);
            label.SetText(text);
            label.SetAlignment(props.align);
            label.SetIcon(props.icon);
            return &label;
        };
    }
    
    WidgetDesc Button(const ButtonProps &props)
    {
        return [props](Application& app) -> Element* {
            auto& button = app.Create<tui::Button>();
            ElementSetup(button, props.base);
            button.SetText(props.text);
            button.SetIcon(props.icon);
            button.SetOnClick(props.onClick);
            return &button;
        };
    }

    WidgetDesc TextEdit(const TextEditProps &props)
    {
        return [props](Application& app) -> Element* {
            auto& edit = app.Create<Edit>();
            ElementSetup(edit, props.base);
            edit.SetText(props.text);
            edit.SetMultiLine(props.multiLine);
            edit.SetMasked(props.masked);
            edit.SetOnChange(props.onChanged);
            return &edit;
        };
    }

    WidgetDesc Image(const ImageProps &props)
    {
        return [props](Application& app) -> Element* {
            auto& imageView = app.Create<ImageView>();
            ElementSetup(imageView, props.base);
            
            imageView.SetImage(new tui::Image(props.fileName));
            imageView.SetScalingMode(props.scaling);

            return &imageView;
        };
    }
    
    WidgetDesc ScrollView(const WidgetDesc &child)
    {
        return [child](Application& app) -> Element* {
            auto& sv = app.Create<tui::ScrollView>();
            Element* content = child(app);
            sv.SetElement(content);
            return &sv;
        };
    }
    
    WidgetDesc Slider(const SliderProps &props)
    {
        return [props](Application& app) -> Element* {
            auto& slider = app.Create<tui::Slider>();
            ElementSetup(slider, props.base);
            slider.SetDirection(props.direction);
            slider.SetRange(props.range.minimum, props.range.maximum);
            slider.SetValue(props.value);
            slider.SetStep(props.step);
            slider.SetOnValueChange(props.onValueChange);
            return &slider;
        };
    }
}