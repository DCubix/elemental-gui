#pragma once

#include "Element.h"
#include "FlexLayout.h"
#include "ImageView.h"
#include "List.h"
#include "Menu.h"
#include "SplitView.h"
#include "Utils.h"
#include "Window.h"

#include <optional>

namespace gui::declarative {

#define BaseOf(x) x.base.value()

    template <typename T>
    using opt = std::optional<T>;
    using WidgetDesc = std::function<Element*(Window&)>;

    // --- Element base props -------------------------------
    struct ElementProps {
        opt<std::string> tag{std::nullopt};
        opt<bool> enabled{std::nullopt};
        opt<bool> autoSize{std::nullopt};
        opt<float> flexGrow{std::nullopt};
        opt<Rectangle> bounds{std::nullopt};
        opt<Json> style{std::nullopt};

        ElementProps CopyWith(const ElementProps& b) const;
    };

    template <typename T>
    concept HasBaseProps = requires(T t) {
        { t.base } -> std::convertible_to<opt<ElementProps>>;
    };

    void ElementSetup(Element& element, const ElementProps& props);

    // --- Column widget ------------------------------------
    struct ColumnProps {
        opt<ElementProps> base{std::nullopt};
        opt<int> gap{std::nullopt};
        opt<EdgeInsets> padding{std::nullopt};
        opt<FlexAlign> align{std::nullopt};
        opt<FlexJustify> justify{std::nullopt};
        opt<bool> showBackground{std::nullopt};
    };
    ColumnProps Props(const ColumnProps& a, const ColumnProps& b);
    WidgetDesc Column(const ColumnProps& props, const std::vector<WidgetDesc>& children);

    // --- Row widget --------------------------------------
    struct RowProps {
        opt<ElementProps> base{std::nullopt};
        opt<int> gap{std::nullopt};
        opt<EdgeInsets> padding{std::nullopt};
        opt<FlexAlign> align{std::nullopt};
        opt<FlexJustify> justify{std::nullopt};
        opt<bool> showBackground{std::nullopt};
    };
    RowProps Props(const RowProps& a, const RowProps& b);
    WidgetDesc Row(const RowProps& props, const std::vector<WidgetDesc>& children);

    // --- Text widget -------------------------------------
    struct TextProps {
        opt<ElementProps> base{std::nullopt};
        opt<Alignment> align{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};
    };
    TextProps Props(const TextProps& a, const TextProps& b);
    WidgetDesc Text(const std::string& text, const TextProps& props);

    // --- Button widget -----------------------------------
    struct ButtonProps {
        opt<ElementProps> base{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};
        opt<VoidCallback> onClick{std::nullopt};
    };
    ButtonProps Props(const ButtonProps& a, const ButtonProps& b);
    WidgetDesc Button(const std::string& text, const ButtonProps& props);

    // --- TextEdit widget -------------------------------------
    struct TextEditProps {
        opt<ElementProps> base{std::nullopt};
        opt<std::string> text{std::nullopt};
        opt<ValueChanged<std::string>> onChanged{std::nullopt};
        opt<bool> multiLine{std::nullopt};
        opt<bool> masked{std::nullopt};
    };
    TextEditProps Props(const TextEditProps& a, const TextEditProps& b);
    WidgetDesc TextEdit(const TextEditProps& props);

    // --- Image widget -------------------------------------
    struct ImageProps {
        opt<ElementProps> base{std::nullopt};
        opt<gui::Image*> image{std::nullopt};
        opt<ImageScalingMode> scaling{std::nullopt};
    };
    ImageProps Props(const ImageProps& a, const ImageProps& b);
    WidgetDesc Image(const ImageProps& props);

    // --- ScrollView widget ---------------------------------
    struct ScrollViewProps {
        opt<ElementProps> base{std::nullopt};
        opt<Direction> scrollDirection{std::nullopt};
    };
    ScrollViewProps Props(const ScrollViewProps& a, const ScrollViewProps& b);
    WidgetDesc ScrollView(const WidgetDesc& child, const ScrollViewProps& props = {});

    // --- CheckBox widget ----------------------------------
    struct CheckBoxProps {
        opt<ElementProps> base{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<ValueChanged<bool>> onChanged{std::nullopt};
    };
    CheckBoxProps Props(const CheckBoxProps& a, const CheckBoxProps& b);
    WidgetDesc CheckBox(const std::string& text, const CheckBoxProps& props);

    // --- RadioButton widget --------------------------------
    struct RadioButtonProps {
        opt<ElementProps> base{std::nullopt};
        opt<std::string> group{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<ValueChanged<bool>> onChanged{std::nullopt};
    };
    RadioButtonProps Props(const RadioButtonProps& a, const RadioButtonProps& b);
    WidgetDesc RadioButton(const std::string& text, const RadioButtonProps& props);

    // --- Switch widget ------------------------------------
    struct SwitchProps {
        opt<ElementProps> base{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<ValueChanged<bool>> onChanged{std::nullopt};
    };
    SwitchProps Props(const SwitchProps& a, const SwitchProps& b);
    WidgetDesc Switch(const SwitchProps& props);

    // --- Slider widget -------------------------------------
    struct SliderProps {
        opt<ElementProps> base{std::nullopt};
        opt<Direction> direction{std::nullopt};
        opt<Range> range{std::nullopt};
        opt<float> value{std::nullopt};
        opt<float> step{std::nullopt};
        opt<ValueChanged<float>> onValueChange{std::nullopt};
    };
    SliderProps Props(const SliderProps& a, const SliderProps& b);
    WidgetDesc Slider(const SliderProps& props);

    // --- MenuItem widget ----------------------------------
    using MenuDesc = std::function<gui::Menu*(Window&)>;

    struct MenuItemProps {
        opt<ElementProps> base{std::nullopt};
        opt<std::string> text{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<VoidCallback> onClick{std::nullopt};
        opt<MenuDesc> subMenu{std::nullopt};
    };
    MenuItemProps Props(const MenuItemProps& a, const MenuItemProps& b);
    WidgetDesc MenuItem(const MenuItemProps& props);
    WidgetDesc MenuSeparator();

    // --- Menu widget -------------------------------------
    struct MenuProps {
        opt<ElementProps> base{std::nullopt};
        opt<VoidCallback> onDismiss{std::nullopt};
    };
    MenuProps Props(const MenuProps& a, const MenuProps& b);
    MenuDesc Menu(const MenuProps& props, const std::vector<WidgetDesc>& items);

    // --- Custom widget ------------------------------------
    template <DerivedFromElement Elem, HasBaseProps P>
    WidgetDesc Custom(const P& props, std::function<void(Elem&, const P&)> setup = nullptr) {
        return [props, setup](Window& window) -> Element* {
            auto& element = window.Create<Elem>();
            ElementSetup(element, props.base.value_or(ElementProps{}));
            if (setup) {
                setup(element, props);
            }
            return &element;
        };
    }

    // --- SplitView widget -------------------------------------
    struct SplitViewProps {
        opt<ElementProps> base{std::nullopt};
        opt<Direction> direction{std::nullopt};
        opt<int> splitPosition{std::nullopt}; // px
        opt<gui::SplitViewAlign> align{std::nullopt};
        opt<WidgetDesc> first{std::nullopt};
        opt<WidgetDesc> second{std::nullopt};
    };
    SplitViewProps Props(const SplitViewProps& a, const SplitViewProps& b);
    WidgetDesc SplitView(const SplitViewProps& props);

    // --- List widget -------------------------------------
    template <typename T>
    struct ListProps {
        opt<ElementProps> base{std::nullopt};
        opt<std::vector<ListItem<T>>> items{std::nullopt};
        opt<int> selectedIndex{std::nullopt};
        opt<ValueChanged<int>> onSelectionChanged{std::nullopt};
    };

    template <typename T>
    ListProps<T> Props(const ListProps<T>& a, const ListProps<T>& b) {
        return ListProps<T>{
            .base = b.base.has_value() ? b.base : a.base,
            .items = b.items.has_value() ? b.items : a.items,
            .selectedIndex = b.selectedIndex.has_value() ? b.selectedIndex : a.selectedIndex,
            .onSelectionChanged =
                b.onSelectionChanged.has_value() ? b.onSelectionChanged : a.onSelectionChanged,
        };
    }

    template <typename T>
    WidgetDesc List(const ListProps<T>& props) {
        return [props](Window& window) -> Element* {
            auto& list = window.Create<gui::List<T>>();
            ElementSetup(list, props.base.value_or(ElementProps{}));
            auto items = props.items.value_or(std::vector<ListItem<T>>{});
            for (const auto& item : items) {
                list.AddItem(item.data, item.label);
            }
            list.SetSelectedIndex(props.selectedIndex.value_or(-1));
            list.SetOnSelectionChanged(props.onSelectionChanged.value_or(nullptr));
            return &list;
        };
    }

    struct BasicListProps {
        opt<ElementProps> base{std::nullopt};
        opt<std::vector<std::string>> items{std::nullopt};
        opt<int> selectedIndex{std::nullopt};
        opt<ValueChanged<int>> onSelectionChanged{std::nullopt};
    };
    BasicListProps Props(const BasicListProps& a, const BasicListProps& b);
    WidgetDesc BasicList(const BasicListProps& props);

    // --- ToolButton widget -------------------------------------
    struct ToolButtonProps {
        opt<ElementProps> base{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};
        opt<uint> iconSize{std::nullopt};
        opt<bool> toggled{std::nullopt};
        opt<VoidCallback> onClick{std::nullopt};
        opt<std::string> group{std::nullopt}; // For radio behavior

        ToolButtonProps CopyWith(const ToolButtonProps& b) const;
    };
    ToolButtonProps Props(const ToolButtonProps& a, const ToolButtonProps& b);
    WidgetDesc ToolButton(const std::string& text, const ToolButtonProps& props);
    WidgetDesc ToolRadioButton(const std::string& text, const ToolButtonProps& props);
    WidgetDesc ToolToggleButton(const std::string& text, const ToolButtonProps& props);

    // --- ProgressBar widget -------------------------------------
    struct ProgressBarProps {
        opt<ElementProps> base{std::nullopt};
        opt<Range> range{std::nullopt};
        opt<float> value{std::nullopt};
        opt<bool> indeterminate{std::nullopt};
        opt<Direction> direction{std::nullopt};
    };
    ProgressBarProps Props(const ProgressBarProps& a, const ProgressBarProps& b);
    WidgetDesc ProgressBar(const ProgressBarProps& props);

    // --- Spinner widget -------------------------------------
    struct SpinnerProps {
        opt<ElementProps> base{std::nullopt};
        opt<Range> range{std::nullopt};
        opt<float> value{std::nullopt};
        opt<float> step{std::nullopt};
        opt<int> decimals{std::nullopt};
        opt<ValueChanged<float>> onValueChange{std::nullopt};
    };
    SpinnerProps Props(const SpinnerProps& a, const SpinnerProps& b);
    WidgetDesc Spinner(const SpinnerProps& props = {});

    WidgetDesc Spacer();

    struct PanelProps {
        opt<ElementProps> base{std::nullopt};
        opt<EdgeInsets> padding{std::nullopt};
        opt<bool> showBackground{std::nullopt};

        PanelProps CopyWith(const PanelProps& b) const;
    };
    WidgetDesc Panel(const PanelProps& props, const std::vector<WidgetDesc>& children);

    struct ColorPickerProps {
        opt<ElementProps> base{std::nullopt};
        opt<Color> selectedColor{std::nullopt};
    };
    WidgetDesc ColorPicker(const ColorPickerProps& props);
} // namespace gui::declarative
