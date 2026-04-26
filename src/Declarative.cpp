#include "Declarative.h"

#include "Button.h"
#include "CheckBox.h"
#include "Edit.h"
#include "FlexLayout.h"
#include "Menu.h"
#include "Panel.h"
#include "ProgressBar.h"
#include "RadioButton.h"
#include "ScrollView.h"
#include "Slider.h"
#include "Spinner.h"
#include "SplitView.h"
#include "Switch.h"
#include "TextArea.h"
#include "ToolButton.h"

namespace gui::declarative {

    void ElementSetup(Element& element, const ElementProps& props) {
        if (props.bounds.w > 0)
            element.SetSize({props.bounds.w, element.GetSize().h});
        if (props.bounds.h > 0)
            element.SetSize({element.GetSize().w, props.bounds.h});
        element.SetPosition({props.bounds.x, props.bounds.y});
        element.SetTag(props.tag);
        element.SetAutoSize(props.autoSize);
        element.SetFlexGrow(props.flexGrow);
        element.SetStyle(props.style);
        element.SetEnabled(props.enabled);
    }

    WidgetDesc Column(const ColumnProps& props, const std::vector<WidgetDesc>& children) {
        return [props, children](Window& window) -> Element* {
            auto& panel = window.Create<Panel>();
            ElementSetup(panel, props.base);

            panel.SetBackgroundVisible(props.showBackground);

            auto* flex = panel.GetLayout<FlexLayout>();
            flex->SetDirection(FlexDirection::Column);
            flex->SetAlignItems(props.align);
            flex->SetJustifyContent(props.justify);
            flex->SetGap(props.gap);
            flex->SetPadding(props.padding);

            for (auto&& childDesc : children) {
                Element* child = childDesc(window);
                panel.Add(child);
            }

            return &panel;
        };
    }

    WidgetDesc Row(const RowProps& props, const std::vector<WidgetDesc>& children) {
        return [props, children](Window& window) -> Element* {
            auto& panel = window.Create<Panel>();
            ElementSetup(panel, props.base);

            panel.SetBackgroundVisible(props.showBackground);

            auto* flex = panel.GetLayout<FlexLayout>();
            flex->SetDirection(FlexDirection::Row);
            flex->SetAlignItems(props.align);
            flex->SetJustifyContent(props.justify);
            flex->SetGap(props.gap);
            flex->SetPadding(props.padding);

            for (auto&& childDesc : children) {
                Element* child = childDesc(window);
                panel.Add(child);
            }

            return &panel;
        };
    }

    WidgetDesc Text(const std::string& text, const TextProps& props) {
        return [text, props](Window& window) -> Element* {
            auto& label = window.Create<Label>();
            ElementSetup(label, props.base);
            label.SetText(text);
            label.SetAlignment(props.align);
            label.SetIcon(props.icon);
            return &label;
        };
    }

    WidgetDesc Button(const std::string& text, const ButtonProps& props) {
        return [text, props](Window& window) -> Element* {
            auto& button = window.Create<gui::Button>();
            ElementSetup(button, props.base);
            button.SetText(text);
            button.SetIcon(props.icon);
            button.SetOnClick(props.onClick);
            return &button;
        };
    }

    WidgetDesc TextEdit(const TextEditProps& props) {
        return [props](Window& window) -> Element* {
            if (props.multiLine) {
                auto& edit = window.Create<TextArea>();
                ElementSetup(edit, props.base);
                edit.SetText(props.text);
                edit.SetOnChange(props.onChanged);
                return &edit;
            } else {
                auto& edit = window.Create<Edit>();
                ElementSetup(edit, props.base);
                edit.SetText(props.text);
                edit.SetMasked(props.masked);
                edit.SetOnChange(props.onChanged);
                return &edit;
            }
        };
    }

    WidgetDesc Image(const ImageProps& props) {
        return [props](Window& window) -> Element* {
            auto& imageView = window.Create<ImageView>();
            ElementSetup(imageView, props.base);

            imageView.SetImage(props.image);
            imageView.SetScalingMode(props.scaling);

            return &imageView;
        };
    }

    WidgetDesc ScrollView(const WidgetDesc& child, const ScrollViewProps& props) {
        return [child, props](Window& window) -> Element* {
            auto& sv = window.Create<gui::ScrollView>();
            ElementSetup(sv, props.base);
            sv.SetScrollDirection(props.scrollDirection);
            Element* content = child(window);
            sv.SetElement(content);
            return &sv;
        };
    }

    WidgetDesc CheckBox(const std::string& text, const CheckBoxProps& props) {
        return [props, text](Window& window) -> Element* {
            auto& cb = window.Create<gui::CheckBox>();
            ElementSetup(cb, props.base);
            cb.SetText(text);
            cb.SetChecked(props.checked);
            cb.SetOnChanged(props.onChanged);
            return &cb;
        };
    }

    WidgetDesc RadioButton(const std::string& text, const RadioButtonProps& props) {
        return [props, text](Window& window) -> Element* {
            auto& rb = window.Create<gui::RadioButton>();
            ElementSetup(rb, props.base);
            rb.SetText(text);
            rb.SetGroup(props.group);
            rb.SetChecked(props.checked);
            rb.SetOnChanged(props.onChanged);
            return &rb;
        };
    }

    WidgetDesc Switch(const SwitchProps& props) {
        return [props](Window& window) -> Element* {
            auto& sw = window.Create<gui::Switch>();
            ElementSetup(sw, props.base);
            sw.SetChecked(props.checked);
            sw.SetOnChanged(props.onChanged);
            return &sw;
        };
    }

    WidgetDesc Slider(const SliderProps& props) {
        return [props](Window& window) -> Element* {
            auto& slider = window.Create<gui::Slider>();
            ElementSetup(slider, props.base);
            slider.SetDirection(props.direction);
            slider.SetRange(props.range.minimum, props.range.maximum);
            slider.SetValue(props.value);
            slider.SetStep(props.step);
            slider.SetOnValueChange(props.onValueChange);
            return &slider;
        };
    }

    WidgetDesc MenuItem(const MenuItemProps& props) {
        return [props](Window& window) -> Element* {
            auto& item = window.Create<gui::MenuItem>();
            ElementSetup(item, props.base);
            item.SetText(props.text);
            item.SetIcon(props.icon);
            item.SetChecked(props.checked);
            item.SetOnClick(props.onClick);
            if (props.subMenu) {
                gui::Menu* subMenu = props.subMenu(window);
                item.SetSubMenu(subMenu);
            }
            return &item;
        };
    }

    WidgetDesc MenuSeparator() {
        return [](Window& window) -> Element* {
            auto& item = window.Create<gui::MenuItem>();
            item.SetSeparator(true);
            item.SetAutoSize(true);
            return &item;
        };
    }

    MenuDesc Menu(const MenuProps& props, const std::vector<WidgetDesc>& items) {
        return [props, items](Window& window) -> gui::Menu* {
            auto& menu = window.Create<gui::Menu>();
            ElementSetup(menu, props.base);
            menu.SetAutoSize(true);
            menu.SetOnDismiss(props.onDismiss);

            for (auto&& itemDesc : items) {
                Element* el = itemDesc(window);
                if (auto* item = dynamic_cast<gui::MenuItem*>(el)) {
                    item->SetAutoSize(true);
                    menu.Add(item);
                }
            }

            return &menu;
        };
    }

    WidgetDesc SplitView(const SplitViewProps& props) {
        return [props](Window& window) -> Element* {
            auto& splitView = window.Create<gui::SplitView>();
            ElementSetup(splitView, props.base);
            splitView.SetDirection(props.direction);
            splitView.SetSplitPosition(props.splitPosition);
            if (props.first) {
                Element* first = props.first(window);
                splitView.Add(first);
            }
            if (props.second) {
                Element* second = props.second(window);
                splitView.Add(second);
            }
            return &splitView;
        };
    }

    WidgetDesc BasicList(const BasicListProps& props) {
        return [props](Window& window) -> Element* {
            auto& list = window.Create<gui::List<std::string>>();
            ElementSetup(list, props.base);
            for (const auto& item : props.items) {
                list.AddItem(item, item);
            }
            list.SetSelectedIndex(props.selectedIndex);
            list.SetOnSelectionChanged(props.onSelectionChanged);
            return &list;
        };
    }

    WidgetDesc ToolButton(const std::string& text, const ToolButtonProps& props) {
        return [text, props](Window& window) -> Element* {
            auto& button = window.Create<gui::ToolButton>();
            ElementSetup(button, props.base);
            button.SetText(text);
            button.SetIcon(props.icon);
            button.SetIconSize(props.iconSize);
            button.SetMode(gui::ToolButton::Mode::Normal);
            button.SetGroup(props.group);
            button.SetOnClick(props.onClick);
            return &button;
        };
    }

    WidgetDesc ToolRadioButton(const std::string& text, const ToolButtonProps& props) {
        return [text, props](Window& window) -> Element* {
            auto& button = window.Create<gui::ToolButton>();
            ElementSetup(button, props.base);
            button.SetText(text);
            button.SetIcon(props.icon);
            button.SetIconSize(props.iconSize);
            button.SetMode(gui::ToolButton::Mode::Radio);
            button.SetGroup(props.group);
            button.SetToggled(props.toggled);
            button.SetOnClick(props.onClick);
            return &button;
        };
    }

    WidgetDesc ToolToggleButton(const std::string& text, const ToolButtonProps& props) {
        return [text, props](Window& window) -> Element* {
            auto& button = window.Create<gui::ToolButton>();
            ElementSetup(button, props.base);
            button.SetText(text);
            button.SetIcon(props.icon);
            button.SetIconSize(props.iconSize);
            button.SetMode(gui::ToolButton::Mode::Toggle);
            button.SetToggled(props.toggled);
            button.SetOnClick(props.onClick);
            return &button;
        };
    }

    WidgetDesc ProgressBar(const ProgressBarProps& props) {
        return [props](Window& window) -> Element* {
            auto& progressBar = window.Create<gui::ProgressBar>();
            ElementSetup(progressBar, props.base);
            progressBar.SetRange(props.range.minimum, props.range.maximum);
            progressBar.SetValue(props.value);
            progressBar.SetIndeterminate(props.indeterminate);
            progressBar.SetDirection(props.direction);
            return &progressBar;
        };
    }

    WidgetDesc Spinner(const SpinnerProps& props) {
        return [props](Window& window) -> Element* {
            auto& spinner = window.Create<gui::Spinner>();
            ElementSetup(spinner, props.base);
            spinner.SetRange(props.range.minimum, props.range.maximum);
            spinner.SetValue(props.value);
            spinner.SetStep(props.step);
            spinner.SetDecimals(props.decimals);
            if (props.onValueChange)
                spinner.SetOnValueChange(props.onValueChange);
            return &spinner;
        };
    }

    WidgetDesc Spacer() {
        return [](Window& window) -> Element* {
            auto& el = window.Create<gui::Element>();
            el.SetFlexGrow(1.0f);
            return &el;
        };
    }
} // namespace gui::declarative