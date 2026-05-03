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
        opt<std::string> tooltip{std::nullopt};

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
        opt<FlexWrap> wrap{std::nullopt};
        opt<bool> showBackground{std::nullopt};

        ColumnProps CopyWith(const ColumnProps& b) const;
    };
    WidgetDesc Column(const ColumnProps& props, const std::vector<WidgetDesc>& children);

    // --- Row widget --------------------------------------
    struct RowProps {
        opt<ElementProps> base{std::nullopt};
        opt<int> gap{std::nullopt};
        opt<EdgeInsets> padding{std::nullopt};
        opt<FlexAlign> align{std::nullopt};
        opt<FlexJustify> justify{std::nullopt};
        opt<FlexWrap> wrap{std::nullopt};
        opt<bool> showBackground{std::nullopt};

        RowProps CopyWith(const RowProps& b) const;
    };
    WidgetDesc Row(const RowProps& props, const std::vector<WidgetDesc>& children);

    // --- Text widget -------------------------------------
    struct TextProps {
        opt<ElementProps> base{std::nullopt};
        opt<Alignment> align{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};

        TextProps CopyWith(const TextProps& b) const;
    };
    WidgetDesc Text(const std::string& text, const TextProps& props);

    // --- Button widget -----------------------------------
    struct ButtonProps {
        opt<ElementProps> base{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};
        opt<uint> iconSize{std::nullopt};
        opt<VoidCallback> onClick{std::nullopt};

        ButtonProps CopyWith(const ButtonProps& b) const;
    };
    WidgetDesc Button(const std::string& text, const ButtonProps& props);

    // --- TextEdit widget -------------------------------------
    struct TextEditProps {
        opt<ElementProps> base{std::nullopt};
        opt<std::string> text{std::nullopt};
        opt<ValueChanged<std::string>> onChanged{std::nullopt};
        opt<bool> multiLine{std::nullopt};
        opt<bool> masked{std::nullopt};

        TextEditProps CopyWith(const TextEditProps& b) const;
    };
    WidgetDesc TextEdit(const TextEditProps& props);

    // --- Image widget -------------------------------------
    struct ImageProps {
        opt<ElementProps> base{std::nullopt};
        opt<gui::Image*> image{std::nullopt};
        opt<ImageScalingMode> scaling{std::nullopt};

        ImageProps CopyWith(const ImageProps& b) const;
    };
    WidgetDesc Image(const ImageProps& props);

    // --- ScrollView widget ---------------------------------
    struct ScrollViewProps {
        opt<ElementProps> base{std::nullopt};
        opt<Direction> scrollDirection{std::nullopt};

        ScrollViewProps CopyWith(const ScrollViewProps& b) const;
    };
    WidgetDesc ScrollView(const WidgetDesc& child, const ScrollViewProps& props = {});

    // --- CheckBox widget ----------------------------------
    struct CheckBoxProps {
        opt<ElementProps> base{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<ValueChanged<bool>> onChanged{std::nullopt};

        CheckBoxProps CopyWith(const CheckBoxProps& b) const;
    };
    WidgetDesc CheckBox(const std::string& text, const CheckBoxProps& props);

    // --- RadioButton widget --------------------------------
    struct RadioButtonProps {
        opt<ElementProps> base{std::nullopt};
        opt<std::string> group{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<ValueChanged<bool>> onChanged{std::nullopt};

        RadioButtonProps CopyWith(const RadioButtonProps& b) const;
    };
    WidgetDesc RadioButton(const std::string& text, const RadioButtonProps& props);

    // --- Switch widget ------------------------------------
    struct SwitchProps {
        opt<ElementProps> base{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<ValueChanged<bool>> onChanged{std::nullopt};

        SwitchProps CopyWith(const SwitchProps& b) const;
    };
    WidgetDesc Switch(const SwitchProps& props);

    // --- Slider widget -------------------------------------
    struct SliderProps {
        opt<ElementProps> base{std::nullopt};
        opt<Direction> direction{std::nullopt};
        opt<Range> range{std::nullopt};
        opt<float> value{std::nullopt};
        opt<float> step{std::nullopt};
        opt<ValueChanged<float>> onValueChange{std::nullopt};

        SliderProps CopyWith(const SliderProps& b) const;
    };
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

        MenuItemProps CopyWith(const MenuItemProps& b) const;
    };
    WidgetDesc MenuItem(const MenuItemProps& props);
    WidgetDesc MenuSeparator();

    // --- Menu widget -------------------------------------
    struct MenuProps {
        opt<ElementProps> base{std::nullopt};
        opt<VoidCallback> onDismiss{std::nullopt};

        MenuProps CopyWith(const MenuProps& b) const;
    };
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

        SplitViewProps CopyWith(const SplitViewProps& b) const;
    };
    WidgetDesc SplitView(const SplitViewProps& props);

    // --- List widget -------------------------------------
    template <typename T>
    struct ListProps {
        opt<ElementProps> base{std::nullopt};
        opt<std::vector<ListItem<T>>> items{std::nullopt};
        opt<int> selectedIndex{std::nullopt};
        opt<ValueChanged<int>> onSelectionChanged{std::nullopt};

        ListProps<T> CopyWith(const ListProps<T>& b) const {
            return ListProps<T>{
                .base = b.base.has_value() ? b.base : base,
                .items = b.items.has_value() ? b.items : items,
                .selectedIndex = b.selectedIndex.has_value() ? b.selectedIndex : selectedIndex,
                .onSelectionChanged =
                    b.onSelectionChanged.has_value() ? b.onSelectionChanged : onSelectionChanged,
            };
        }
    };

    template <typename T>
    WidgetDesc List(const ListProps<T>& props) {
        return [props](Window& window) -> Element* {
            auto& list = window.Create<gui::List<T>>();
            ElementSetup(list, props.base.value_or(ElementProps{}));
            auto items = props.items.value_or(std::vector<ListItem<T>>{});
            for (const auto& item : items) {
                list.AddItem(item.value, item.label);
            }
            list.selectedIndex = props.selectedIndex.value_or(-1);
            if (props.onSelectionChanged.has_value() && *props.onSelectionChanged)
                list.selectedIndex.Bind(*props.onSelectionChanged);
            return &list;
        };
    }

    struct BasicListProps {
        opt<ElementProps> base{std::nullopt};
        opt<std::vector<std::string>> items{std::nullopt};
        opt<int> selectedIndex{std::nullopt};
        opt<ValueChanged<int>> onSelectionChanged{std::nullopt};

        BasicListProps CopyWith(const BasicListProps& b) const;
    };
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

        ProgressBarProps CopyWith(const ProgressBarProps& b) const;
    };
    WidgetDesc ProgressBar(const ProgressBarProps& props);

    // --- Spinner widget -------------------------------------
    struct SpinnerProps {
        opt<ElementProps> base{std::nullopt};
        opt<Range> range{std::nullopt};
        opt<float> value{std::nullopt};
        opt<float> step{std::nullopt};
        opt<int> decimals{std::nullopt};
        opt<ValueChanged<float>> onValueChange{std::nullopt};

        SpinnerProps CopyWith(const SpinnerProps& b) const;
    };
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
        opt<ValueChanged<Color>> onChange{std::nullopt};

        ColorPickerProps CopyWith(const ColorPickerProps& b) const;
    };
    WidgetDesc ColorPicker(const ColorPickerProps& props);
} // namespace gui::declarative
