#include "Declarative.h"

#include "Panel.h"
#include "FlexLayout.h"
#include "Button.h"
#include "Edit.h"
#include "ScrollView.h"
#include "Slider.h"
#include "Switch.h"
#include "CheckBox.h"
#include "Menu.h"
#include "RadioButton.h"
#include "SplitView.h"

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
    
    WidgetDesc Button(const std::string &text, const ButtonProps &props)
    {
        return [text, props](Application& app) -> Element* {
            auto& button = app.Create<tui::Button>();
            ElementSetup(button, props.base);
            button.SetText(text);
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
    
    WidgetDesc ScrollView(const WidgetDesc &child, const ScrollViewProps& props)
    {
        return [child, props](Application& app) -> Element* {
            auto& sv = app.Create<tui::ScrollView>();
            ElementSetup(sv, props.base);
            sv.SetScrollDirection(props.scrollDirection);
            Element* content = child(app);
            sv.SetElement(content);
            return &sv;
        };
    }
    
    WidgetDesc CheckBox(const std::string &text, const CheckBoxProps &props)
    {
        return [props, text](Application& app) -> Element* {
            auto& cb = app.Create<tui::CheckBox>();
            ElementSetup(cb, props.base);
            cb.SetText(text);
            cb.SetChecked(props.checked);
            cb.SetOnChanged(props.onChanged);
            return &cb;
        };
    }

    WidgetDesc RadioButton(const std::string &text, const RadioButtonProps &props)
    {
        return [props, text](Application& app) -> Element* {
            auto& rb = app.Create<tui::RadioButton>();
            ElementSetup(rb, props.base);
            rb.SetText(text);
            rb.SetGroup(props.group);
            rb.SetChecked(props.checked);
            rb.SetOnChanged(props.onChanged);
            return &rb;
        };
    }

    WidgetDesc Switch(const SwitchProps &props)
    {
        return [props](Application& app) -> Element* {
            auto& sw = app.Create<tui::Switch>();
            ElementSetup(sw, props.base);
            sw.SetChecked(props.checked);
            sw.SetOnChanged(props.onChanged);
            return &sw;
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

    WidgetDesc MenuItem(const MenuItemProps &props)
    {
        return [props](Application& app) -> Element* {
            auto& item = app.Create<tui::MenuItem>();
            ElementSetup(item, props.base);
            item.SetText(props.text);
            item.SetIcon(props.icon);
            item.SetChecked(props.checked);
            item.SetOnClick(props.onClick);
            if (props.subMenu) {
                tui::Menu* subMenu = props.subMenu(app);
                item.SetSubMenu(subMenu);
            }
            return &item;
        };
    }

    WidgetDesc MenuSeparator()
    {
        return [](Application& app) -> Element* {
            auto& item = app.Create<tui::MenuItem>();
            item.SetSeparator(true);
            item.SetAutoSize(true);
            return &item;
        };
    }

    MenuDesc Menu(const MenuProps &props, const std::vector<WidgetDesc> &items)
    {
        return [props, items](Application& app) -> tui::Menu* {
            auto& menu = app.Create<tui::Menu>();
            ElementSetup(menu, props.base);
            menu.SetAutoSize(true);
            menu.SetOnDismiss(props.onDismiss);

            for (auto&& itemDesc : items) {
                Element* el = itemDesc(app);
                if (auto* item = dynamic_cast<tui::MenuItem*>(el)) {
                    item->SetAutoSize(true);
                    menu.Add(item);
                }
            }

            return &menu;
        };
    }
    
    WidgetDesc SplitView(const SplitViewProps &props)
    {
        return [props](Application& app) -> Element* {
            auto& splitView = app.Create<tui::SplitView>();
            ElementSetup(splitView, props.base);
            splitView.SetDirection(props.direction);
            splitView.SetSplitPosition(props.splitPosition);
            if (props.first) {
                Element* first = props.first(app);
                splitView.Add(first);
            }
            if (props.second) {
                Element* second = props.second(app);
                splitView.Add(second);
            }
            return &splitView;
        };
    }
    
    WidgetDesc BasicList(const BasicListProps &props)
    {
        return [props](Application& app) -> Element* {
            auto& list = app.Create<tui::List<std::string>>();
            ElementSetup(list, props.base);
            for (const auto& item : props.items) {
                list.AddItem(item, item);
            }
            list.SetSelectedIndex(props.selectedIndex);
            list.SetOnSelectionChanged(props.onSelectionChanged);
            return &list;
        };
    }
}