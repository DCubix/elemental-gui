#include "Declarative.h"

#include "Button.h"
#include "CheckBox.h"
#include "ColorPicker.h"
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

#define MergeField(name) .name = b.name.has_value() ? b.name : name

    ElementProps ElementProps::CopyWith(const ElementProps& b) const {
        return ElementProps{
            MergeField(tag),
            MergeField(enabled),
            MergeField(autoSize),
            MergeField(flexGrow),
            MergeField(bounds),
            MergeField(style),
        };
    }

    void ElementSetup(Element& element, const ElementProps& props) {
        auto tag = props.tag.value_or("");
        auto enabled = props.enabled.value_or(true);
        auto autoSize = props.autoSize.value_or(false);
        auto flexGrow = props.flexGrow.value_or(0.0f);
        auto bounds = props.bounds.value_or(Rectangle{0, 0, 0, 0});
        auto style = props.style.value_or(Json());

        if (bounds.w > 0)
            element.SetSize({bounds.w, element.GetSize().h});
        if (bounds.h > 0)
            element.SetSize({element.GetSize().w, bounds.h});
        element.SetPosition({bounds.x, bounds.y});
        element.SetTag(tag);
        element.SetAutoSize(autoSize);
        element.SetFlexGrow(flexGrow);
        element.SetStyle(style);
        element.SetEnabled(enabled);
    }

    WidgetDesc Column(const ColumnProps& props, const std::vector<WidgetDesc>& children) {
        return [props, children](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto gap = props.gap.value_or(0);
            auto padding = props.padding.value_or(EdgeInsets::All(0));
            auto align = props.align.value_or(FlexAlign::Start);
            auto justify = props.justify.value_or(FlexJustify::Start);
            auto wrap = props.wrap.value_or(FlexWrap::NoWrap);
            auto showBackground = props.showBackground.value_or(false);

            auto& panel = window.Create<gui::Panel>();
            ElementSetup(panel, base);

            panel.SetBackgroundVisible(showBackground);

            auto* flex = panel.GetLayout<FlexLayout>();
            flex->SetDirection(FlexDirection::Column);
            flex->SetAlignItems(align);
            flex->SetJustifyContent(justify);
            flex->SetWrap(wrap);
            flex->SetGap(gap);
            flex->SetPadding(padding);

            for (auto&& childDesc : children) {
                Element* child = childDesc(window);
                panel.Add(child);
            }

            return &panel;
        };
    }

    WidgetDesc Row(const RowProps& props, const std::vector<WidgetDesc>& children) {
        return [props, children](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto gap = props.gap.value_or(0);
            auto padding = props.padding.value_or(EdgeInsets::All(0));
            auto align = props.align.value_or(FlexAlign::Start);
            auto justify = props.justify.value_or(FlexJustify::Start);
            auto wrap = props.wrap.value_or(FlexWrap::NoWrap);
            auto showBackground = props.showBackground.value_or(false);

            auto& panel = window.Create<gui::Panel>();
            ElementSetup(panel, base);

            panel.SetBackgroundVisible(showBackground);

            auto* flex = panel.GetLayout<FlexLayout>();
            flex->SetDirection(FlexDirection::Row);
            flex->SetAlignItems(align);
            flex->SetJustifyContent(justify);
            flex->SetWrap(wrap);
            flex->SetGap(gap);
            flex->SetPadding(padding);

            for (auto&& childDesc : children) {
                Element* child = childDesc(window);
                panel.Add(child);
            }

            return &panel;
        };
    }

    WidgetDesc Text(const std::string& text, const TextProps& props) {
        return [text, props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto align = props.align.value_or(Alignment::TopLeft);
            auto icon = props.icon.value_or(nullptr);

            auto& label = window.Create<Label>();
            ElementSetup(label, base);
            label.text = text;
            label.alignment = align;
            label.SetIcon(icon);
            return &label;
        };
    }

    WidgetDesc Button(const std::string& text, const ButtonProps& props) {
        return [text, props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto icon = props.icon.value_or(nullptr);
            auto iconSize = props.iconSize.value_or(16);
            auto onClick = props.onClick.value_or(nullptr);

            auto& button = window.Create<gui::Button>();
            ElementSetup(button, base);
            button.text = text;
            button.SetIcon(icon);
            button.iconSize = iconSize;
            button.SetOnClick(onClick);
            return &button;
        };
    }

    WidgetDesc TextEdit(const TextEditProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto text = props.text.value_or("");
            auto onChanged = props.onChanged.value_or(nullptr);
            auto multiLine = props.multiLine.value_or(false);
            auto masked = props.masked.value_or(false);

            if (multiLine) {
                auto& edit = window.Create<TextArea>();
                ElementSetup(edit, base);
                edit.text = text;
                if (onChanged)
                    edit.text.Bind(onChanged);
                return &edit;
            } else {
                auto& edit = window.Create<Edit>();
                ElementSetup(edit, base);
                edit.text = text;
                edit.SetMasked(masked);
                if (onChanged)
                    edit.text.Bind(onChanged);
                return &edit;
            }
        };
    }

    WidgetDesc Image(const ImageProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto image = props.image.value_or(nullptr);
            auto scaling = props.scaling.value_or(ImageScalingMode::Stretch);

            auto& imageView = window.Create<ImageView>();
            ElementSetup(imageView, base);
            imageView.SetImage(image);
            imageView.scalingMode = scaling;
            return &imageView;
        };
    }

    WidgetDesc ScrollView(const WidgetDesc& child, const ScrollViewProps& props) {
        return [child, props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto scrollDirection = props.scrollDirection.value_or(Direction::Vertical);

            auto& sv = window.Create<gui::ScrollView>();
            ElementSetup(sv, base);
            sv.SetScrollDirection(scrollDirection);
            Element* content = child(window);
            sv.SetElement(content);
            return &sv;
        };
    }

    WidgetDesc CheckBox(const std::string& text, const CheckBoxProps& props) {
        return [props, text](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto checked = props.checked.value_or(false);
            auto onChanged = props.onChanged.value_or(nullptr);

            auto& cb = window.Create<gui::CheckBox>();
            ElementSetup(cb, base);
            cb.text = text;
            cb.checked = checked;
            if (onChanged)
                cb.checked.Bind(onChanged);
            return &cb;
        };
    }

    WidgetDesc RadioButton(const std::string& text, const RadioButtonProps& props) {
        return [props, text](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto group = props.group.value_or("");
            auto checked = props.checked.value_or(false);
            auto onChanged = props.onChanged.value_or(nullptr);

            auto& rb = window.Create<gui::RadioButton>();
            ElementSetup(rb, base);
            rb.text = text;
            rb.group = group;
            rb.checked = checked;
            if (onChanged)
                rb.checked.Bind(onChanged);
            return &rb;
        };
    }

    WidgetDesc Switch(const SwitchProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto checked = props.checked.value_or(false);
            auto onChanged = props.onChanged.value_or(nullptr);

            auto& sw = window.Create<gui::Switch>();
            ElementSetup(sw, base);
            sw.checked = checked;
            if (onChanged)
                sw.checked.Bind(onChanged);
            return &sw;
        };
    }

    WidgetDesc Slider(const SliderProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto direction = props.direction.value_or(Direction::Horizontal);
            auto range = props.range.value_or(Range{0.0f, 1.0f});
            auto value = props.value.value_or(0.0f);
            auto step = props.step.value_or(0.01f);
            auto onValueChange = props.onValueChange.value_or(nullptr);

            auto& slider = window.Create<gui::Slider>();
            ElementSetup(slider, base);
            slider.direction = direction;
            slider.SetRange(range.minimum, range.maximum);
            slider.value = value;
            slider.step = step;
            if (onValueChange)
                slider.value.Bind(onValueChange);
            return &slider;
        };
    }

    WidgetDesc MenuItem(const MenuItemProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto text = props.text.value_or("");
            auto icon = props.icon.value_or(nullptr);
            auto checked = props.checked.value_or(false);
            auto onClick = props.onClick.value_or(nullptr);
            auto subMenu = props.subMenu.value_or(nullptr);

            auto& item = window.Create<gui::MenuItem>();
            ElementSetup(item, base);
            item.text = text;
            item.SetIcon(icon);
            item.checked = checked;
            item.SetOnClick(onClick);
            if (subMenu) {
                gui::Menu* sm = subMenu(window);
                item.SetSubMenu(sm);
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
            auto base = props.base.value_or(ElementProps{});
            auto onDismiss = props.onDismiss.value_or(nullptr);

            auto& menu = window.Create<gui::Menu>();
            ElementSetup(menu, base);
            menu.SetAutoSize(true);
            menu.SetOnDismiss(onDismiss);

            for (auto&& itemDesc : items) {
                Element* el = itemDesc(window);
                el->SetAutoSize(true);
                menu.Add(el);
            }

            return &menu;
        };
    }

    WidgetDesc SplitView(const SplitViewProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto direction = props.direction.value_or(Direction::Horizontal);
            auto align = props.align.value_or(SplitViewAlign::Start);
            auto splitPosition = props.splitPosition.value_or(100);
            auto first = props.first.value_or(nullptr);
            auto second = props.second.value_or(nullptr);

            auto& splitView = window.Create<gui::SplitView>();
            ElementSetup(splitView, base);
            splitView.direction = direction;
            splitView.SetAlign(align);
            splitView.splitPosition = splitPosition;
            if (first) {
                Element* firstEl = first(window);
                splitView.Add(firstEl);
            }
            if (second) {
                Element* secondEl = second(window);
                splitView.Add(secondEl);
            }
            return &splitView;
        };
    }

    WidgetDesc BasicList(const BasicListProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto items = props.items.value_or(std::vector<std::string>{});
            auto selectedIndex = props.selectedIndex.value_or(-1);
            auto onSelectionChanged = props.onSelectionChanged.value_or(nullptr);

            auto& list = window.Create<gui::List<std::string>>();
            ElementSetup(list, base);
            for (const auto& item : items)
                list.AddItem(item, item);
            list.selectedIndex = selectedIndex;
            if (onSelectionChanged)
                list.selectedIndex.Bind(onSelectionChanged);
            return &list;
        };
    }

    WidgetDesc ToolButton(const std::string& text, const ToolButtonProps& props) {
        return [text, props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto icon = props.icon.value_or(nullptr);
            auto iconSize = props.iconSize.value_or(16u);
            auto group = props.group.value_or("");
            auto onClick = props.onClick.value_or(nullptr);

            auto& button = window.Create<gui::ToolButton>();
            ElementSetup(button, base);
            button.text = text;
            button.SetIcon(icon);
            button.iconSize = iconSize;
            button.SetMode(gui::ToolButton::Mode::Normal);
            button.SetGroup(group);
            button.SetOnClick(onClick);
            return &button;
        };
    }

    WidgetDesc ToolRadioButton(const std::string& text, const ToolButtonProps& props) {
        return [text, props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto icon = props.icon.value_or(nullptr);
            auto iconSize = props.iconSize.value_or(16u);
            auto toggled = props.toggled.value_or(false);
            auto group = props.group.value_or("");
            auto onClick = props.onClick.value_or(nullptr);

            auto& button = window.Create<gui::ToolButton>();
            ElementSetup(button, base);
            button.text = text;
            button.SetIcon(icon);
            button.iconSize = iconSize;
            button.SetMode(gui::ToolButton::Mode::Radio);
            button.SetGroup(group);
            button.toggled = toggled;
            button.SetOnClick(onClick);
            return &button;
        };
    }

    WidgetDesc ToolToggleButton(const std::string& text, const ToolButtonProps& props) {
        return [text, props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto icon = props.icon.value_or(nullptr);
            auto iconSize = props.iconSize.value_or(16u);
            auto toggled = props.toggled.value_or(false);
            auto onClick = props.onClick.value_or(nullptr);

            auto& button = window.Create<gui::ToolButton>();
            ElementSetup(button, base);
            button.text = text;
            button.SetIcon(icon);
            button.iconSize = iconSize;
            button.SetMode(gui::ToolButton::Mode::Toggle);
            button.toggled = toggled;
            button.SetOnClick(onClick);
            return &button;
        };
    }

    WidgetDesc ProgressBar(const ProgressBarProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto range = props.range.value_or(Range{0.0f, 1.0f});
            auto value = props.value.value_or(0.0f);
            auto indeterminate = props.indeterminate.value_or(false);
            auto direction = props.direction.value_or(Direction::Horizontal);

            auto& progressBar = window.Create<gui::ProgressBar>();
            ElementSetup(progressBar, base);
            progressBar.SetRange(range.minimum, range.maximum);
            progressBar.value = value;
            progressBar.SetIndeterminate(indeterminate);
            progressBar.direction = direction;
            return &progressBar;
        };
    }

    WidgetDesc Spinner(const SpinnerProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto range = props.range.value_or(Range{0.0f, 100.0f});
            auto value = props.value.value_or(0.0f);
            auto step = props.step.value_or(1.0f);
            auto decimals = props.decimals.value_or(0);
            auto onValueChange = props.onValueChange.value_or(nullptr);

            auto& spinner = window.Create<gui::Spinner>();
            ElementSetup(spinner, base);
            spinner.SetRange(range.minimum, range.maximum);
            spinner.value = value;
            spinner.step = step;
            spinner.decimals = decimals;
            if (onValueChange)
                spinner.value.Bind(onValueChange);
            return &spinner;
        };
    }

    WidgetDesc Spacer() {
        return [](Window& window) -> Element* {
            auto& el = window.Create<gui::Element>();
            el.SetLocalBounds(Rectangle{0, 0, 1, 1});
            el.SetFlexGrow(1.0f);
            return &el;
        };
    }

    WidgetDesc Panel(const PanelProps& props, const std::vector<WidgetDesc>& children) {
        return [props, children](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto padding = props.padding.value_or(EdgeInsets::All(0));
            auto showBackground = props.showBackground.value_or(false);

            auto& panel = window.Create<gui::Panel>();
            ElementSetup(panel, base);

            panel.SetBackgroundVisible(showBackground);
            panel.SetLayout(nullptr);

            for (auto&& childDesc : children) {
                Element* child = childDesc(window);
                panel.Add(child);
            }

            return &panel;
        };
    }

    WidgetDesc ColorPicker(const ColorPickerProps& props) {
        return [props](Window& window) -> Element* {
            auto base = props.base.value_or(ElementProps{});
            auto onChange = props.onChange.value_or(nullptr);

            auto& picker = window.Create<gui::ColorPicker>();
            ElementSetup(picker, base);

            if (props.selectedColor.has_value())
                picker.selected = props.selectedColor.value();
            picker.SetOnChange(onChange);

            return &picker;
        };
    }

    ColumnProps ColumnProps::CopyWith(const ColumnProps& b) const {
        return ColumnProps{
            MergeField(base),
            MergeField(gap),
            MergeField(padding),
            MergeField(align),
            MergeField(justify),
            MergeField(wrap),
            MergeField(showBackground),
        };
    }

    RowProps RowProps::CopyWith(const RowProps& b) const {
        return RowProps{
            MergeField(base),
            MergeField(gap),
            MergeField(padding),
            MergeField(align),
            MergeField(justify),
            MergeField(wrap),
            MergeField(showBackground),
        };
    }

    TextProps TextProps::CopyWith(const TextProps& b) const {
        return TextProps{
            MergeField(base),
            MergeField(align),
            MergeField(icon),
        };
    }

    ButtonProps ButtonProps::CopyWith(const ButtonProps& b) const {
        return ButtonProps{
            MergeField(base),
            MergeField(icon),
            MergeField(onClick),
        };
    }

    TextEditProps TextEditProps::CopyWith(const TextEditProps& b) const {
        return TextEditProps{
            MergeField(base),
            MergeField(text),
            MergeField(onChanged),
            MergeField(multiLine),
            MergeField(masked),
        };
    }

    ImageProps ImageProps::CopyWith(const ImageProps& b) const {
        return ImageProps{
            MergeField(base),
            MergeField(image),
            MergeField(scaling),
        };
    }

    ScrollViewProps ScrollViewProps::CopyWith(const ScrollViewProps& b) const {
        return ScrollViewProps{
            MergeField(base),
            MergeField(scrollDirection),
        };
    }

    CheckBoxProps CheckBoxProps::CopyWith(const CheckBoxProps& b) const {
        return CheckBoxProps{
            MergeField(base),
            MergeField(checked),
            MergeField(onChanged),
        };
    }

    RadioButtonProps RadioButtonProps::CopyWith(const RadioButtonProps& b) const {
        return RadioButtonProps{
            MergeField(base),
            MergeField(group),
            MergeField(checked),
            MergeField(onChanged),
        };
    }

    SwitchProps SwitchProps::CopyWith(const SwitchProps& b) const {
        return SwitchProps{
            MergeField(base),
            MergeField(checked),
            MergeField(onChanged),
        };
    }

    SliderProps SliderProps::CopyWith(const SliderProps& b) const {
        return SliderProps{
            MergeField(base),
            MergeField(direction),
            MergeField(range),
            MergeField(value),
            MergeField(step),
            MergeField(onValueChange),
        };
    }

    MenuItemProps MenuItemProps::CopyWith(const MenuItemProps& b) const {
        return MenuItemProps{
            MergeField(base),
            MergeField(text),
            MergeField(icon),
            MergeField(checked),
            MergeField(onClick),
            MergeField(subMenu),
        };
    }

    MenuProps MenuProps::CopyWith(const MenuProps& b) const {
        return MenuProps{
            MergeField(base),
            MergeField(onDismiss),
        };
    }

    SplitViewProps SplitViewProps::CopyWith(const SplitViewProps& b) const {
        return SplitViewProps{
            MergeField(base),
            MergeField(direction),
            MergeField(splitPosition),
            MergeField(align),
            MergeField(first),
            MergeField(second),
        };
    }

    BasicListProps BasicListProps::CopyWith(const BasicListProps& b) const {
        return BasicListProps{
            MergeField(base),
            MergeField(items),
            MergeField(selectedIndex),
            MergeField(onSelectionChanged),
        };
    }

    ToolButtonProps ToolButtonProps::CopyWith(const ToolButtonProps& b) const {
        return ToolButtonProps{
            MergeField(base),
            MergeField(icon),
            MergeField(iconSize),
            MergeField(toggled),
            MergeField(onClick),
            MergeField(group),
        };
    }

    ProgressBarProps ProgressBarProps::CopyWith(const ProgressBarProps& b) const {
        return ProgressBarProps{
            MergeField(base),
            MergeField(range),
            MergeField(value),
            MergeField(indeterminate),
            MergeField(direction),
        };
    }

    SpinnerProps SpinnerProps::CopyWith(const SpinnerProps& b) const {
        return SpinnerProps{
            MergeField(base),
            MergeField(range),
            MergeField(value),
            MergeField(step),
            MergeField(decimals),
            MergeField(onValueChange),
        };
    }

    PanelProps PanelProps::CopyWith(const PanelProps& b) const {
        return PanelProps{
            MergeField(base),
            MergeField(padding),
            MergeField(showBackground),
        };
    }

    ColorPickerProps ColorPickerProps::CopyWith(const ColorPickerProps& b) const {
        return ColorPickerProps{
            MergeField(base),
            MergeField(selectedColor),
            MergeField(onChange),
        };
    }

} // namespace gui::declarative
