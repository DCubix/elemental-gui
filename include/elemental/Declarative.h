#pragma once

#include "Element.h"
#include "FlexLayout.h"
#include "ImageView.h"
#include "List.h"
#include "Menu.h"
#include "NodeGraph.h"
#include "SplitView.h"
#include "Utils.h"
#include "Window.h"

#include <optional>

#define Fields(...)                                                                                \
    auto GetFields() {                                                                             \
        return std::tie(__VA_ARGS__);                                                              \
    }                                                                                              \
    auto GetFields() const {                                                                       \
        return std::tie(__VA_ARGS__);                                                              \
    }

namespace gui::declarative {

    template <typename T>
    using opt = std::optional<T>;
    using WidgetDesc = std::function<Element*(Window&)>;

    // --- Base copiable type
    template <typename T>
    concept TupleLike = requires {
        { std::tuple_size<std::remove_cvref_t<T>>::value } -> std::convertible_to<std::size_t>;
    };

    template <typename P>
    concept HasFields = requires(P& p, const P& pc) {
        requires TupleLike<decltype(p.GetFields())>;
        requires TupleLike<decltype(pc.GetFields())>;
    };

    template <typename Derived>
    struct Copiable {
        Derived CopyWith(const Derived& other) const
            requires HasFields<Derived>
        {
            const Derived& self = static_cast<const Derived&>(*this);
            Derived result = self;

            constexpr std::size_t N =
                std::tuple_size_v<std::remove_cvref_t<decltype(self.GetFields())>>;

            [&]<std::size_t... I>(std::index_sequence<I...>) {
                auto r = result.GetFields();
                auto o = other.GetFields();
                ((std::get<I>(r) = std::get<I>(o).has_value() ? std::get<I>(o) : std::get<I>(r)),
                 ...);
            }(std::make_index_sequence<N>{});

            return result;
        }
    };

    // --- Element base props -------------------------------
    struct ElementProps : Copiable<ElementProps> {
        opt<std::string> tag{std::nullopt};
        opt<bool> enabled{std::nullopt};
        opt<bool> autoSize{std::nullopt};
        opt<float> flexGrow{std::nullopt};
        opt<Rectangle> bounds{std::nullopt};
        opt<Json> style{std::nullopt};
        opt<std::string> tooltip{std::nullopt};

        Fields(tag, enabled, autoSize, flexGrow, bounds, style, tooltip)
    };

    template <typename T>
    concept HasBaseProps = requires(T t) {
        { t.base } -> std::convertible_to<opt<ElementProps>>;
    };

    template <HasBaseProps P>
    ElementProps BaseOf(const P& props) {
        return props.base.value_or(ElementProps{});
    }

    void ElementSetup(Element& element, const ElementProps& props);

    // --- Column widget ------------------------------------
    struct ColumnProps : Copiable<ColumnProps> {
        opt<ElementProps> base{std::nullopt};
        opt<int> gap{std::nullopt};
        opt<EdgeInsets> padding{std::nullopt};
        opt<FlexAlign> align{std::nullopt};
        opt<FlexJustify> justify{std::nullopt};
        opt<FlexWrap> wrap{std::nullopt};
        opt<bool> showBackground{std::nullopt};

        Fields(base, gap, padding, align, justify, wrap, showBackground)
    };
    WidgetDesc Column(const ColumnProps& props, const std::vector<WidgetDesc>& children);

    // --- Row widget --------------------------------------
    struct RowProps : Copiable<RowProps> {
        opt<ElementProps> base{std::nullopt};
        opt<int> gap{std::nullopt};
        opt<EdgeInsets> padding{std::nullopt};
        opt<FlexAlign> align{std::nullopt};
        opt<FlexJustify> justify{std::nullopt};
        opt<FlexWrap> wrap{std::nullopt};
        opt<bool> showBackground{std::nullopt};

        Fields(base, gap, padding, align, justify, wrap, showBackground)
    };
    WidgetDesc Row(const RowProps& props, const std::vector<WidgetDesc>& children);

    // --- Text widget -------------------------------------
    struct TextProps : Copiable<TextProps> {
        opt<ElementProps> base{std::nullopt};
        opt<Alignment> align{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};
        opt<uint> iconSize{std::nullopt};

        Fields(base, align, icon, iconSize)
    };
    WidgetDesc Text(const std::string& text, const TextProps& props);

    // --- Button widget -----------------------------------
    struct ButtonProps : Copiable<ButtonProps> {
        opt<ElementProps> base{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};
        opt<uint> iconSize{std::nullopt};
        opt<VoidCallback> onClick{std::nullopt};

        Fields(base, icon, iconSize, onClick)
    };
    WidgetDesc Button(const std::string& text, const ButtonProps& props);

    // --- TextEdit widget -------------------------------------
    struct TextEditProps : Copiable<TextEditProps> {
        opt<ElementProps> base{std::nullopt};
        opt<std::string> text{std::nullopt};
        opt<ValueChanged<std::string>> onChanged{std::nullopt};
        opt<bool> multiLine{std::nullopt};
        opt<bool> masked{std::nullopt};

        Fields(base, text, onChanged, multiLine, masked)
    };
    WidgetDesc TextEdit(const TextEditProps& props);

    // --- Image widget -------------------------------------
    struct ImageProps : Copiable<ImageProps> {
        opt<ElementProps> base{std::nullopt};
        opt<gui::Image*> image{std::nullopt};
        opt<ImageScalingMode> scaling{std::nullopt};

        Fields(base, image, scaling)
    };
    WidgetDesc Image(const ImageProps& props);

    // --- ScrollView widget ---------------------------------
    struct ScrollViewProps : Copiable<ScrollViewProps> {
        opt<ElementProps> base{std::nullopt};
        opt<Direction> scrollDirection{std::nullopt};

        Fields(base, scrollDirection)
    };
    WidgetDesc ScrollView(const WidgetDesc& child, const ScrollViewProps& props = {});

    // --- CheckBox widget ----------------------------------
    struct CheckBoxProps : Copiable<CheckBoxProps> {
        opt<ElementProps> base{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<ValueChanged<bool>> onChanged{std::nullopt};

        Fields(base, checked, onChanged)
    };
    WidgetDesc CheckBox(const std::string& text, const CheckBoxProps& props);

    // --- RadioButton widget --------------------------------
    struct RadioButtonProps : Copiable<RadioButtonProps> {
        opt<ElementProps> base{std::nullopt};
        opt<std::string> group{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<ValueChanged<bool>> onChanged{std::nullopt};

        Fields(base, group, checked, onChanged)
    };
    WidgetDesc RadioButton(const std::string& text, const RadioButtonProps& props);

    // --- Switch widget ------------------------------------
    struct SwitchProps : Copiable<SwitchProps> {
        opt<ElementProps> base{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<ValueChanged<bool>> onChanged{std::nullopt};

        Fields(base, checked, onChanged)
    };
    WidgetDesc Switch(const SwitchProps& props);

    // --- Slider widget -------------------------------------
    struct SliderProps : Copiable<SliderProps> {
        opt<ElementProps> base{std::nullopt};
        opt<Direction> direction{std::nullopt};
        opt<Range> range{std::nullopt};
        opt<float> value{std::nullopt};
        opt<float> step{std::nullopt};
        opt<ValueChanged<float>> onValueChange{std::nullopt};

        Fields(base, direction, range, value, step, onValueChange)
    };
    WidgetDesc Slider(const SliderProps& props);

    // --- MenuItem widget ----------------------------------
    using MenuDesc = std::function<gui::Menu*(Window&)>;

    struct MenuItemProps : Copiable<MenuItemProps> {
        opt<ElementProps> base{std::nullopt};
        opt<std::string> text{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};
        opt<bool> checked{std::nullopt};
        opt<VoidCallback> onClick{std::nullopt};
        opt<MenuDesc> subMenu{std::nullopt};

        Fields(base, text, icon, checked, onClick, subMenu)
    };
    WidgetDesc MenuItem(const MenuItemProps& props);
    WidgetDesc MenuSeparator();

    // --- Menu widget -------------------------------------
    struct MenuProps : Copiable<MenuProps> {
        opt<ElementProps> base{std::nullopt};
        opt<VoidCallback> onDismiss{std::nullopt};

        Fields(base, onDismiss)
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
    struct SplitViewProps : Copiable<SplitViewProps> {
        opt<ElementProps> base{std::nullopt};
        opt<Direction> direction{std::nullopt};
        opt<int> splitPosition{std::nullopt}; // px
        opt<gui::SplitViewAlign> align{std::nullopt};
        opt<WidgetDesc> first{std::nullopt};
        opt<WidgetDesc> second{std::nullopt};

        Fields(base, direction, splitPosition, align, first, second)
    };
    WidgetDesc SplitView(const SplitViewProps& props);

    // --- List widget -------------------------------------
    template <typename T>
    struct ListProps : Copiable<ListProps<T>> {
        opt<ElementProps> base{std::nullopt};
        opt<std::vector<T>> items{std::nullopt};
        opt<int> selectedIndex{std::nullopt};
        opt<typename gui::List<T>::template Builder<std::string>> labelBuilder{std::nullopt};
        opt<typename gui::List<T>::template Builder<gui::Image*>> iconBuilder{std::nullopt};
        opt<ValueChanged<int>> onSelectionChanged{std::nullopt};

        Fields(base, items, selectedIndex, labelBuilder, iconBuilder, onSelectionChanged)
    };

    template <typename T>
    WidgetDesc List(const ListProps<T>& props) {
        return [props](Window& window) -> Element* {
            auto& list = window.Create<gui::List<T>>();
            ElementSetup(list, props.base.value_or(ElementProps{}));
            list.items = props.items.value_or(std::vector<T>{});
            list.selectedIndex = props.selectedIndex.value_or(-1);
            if (props.onSelectionChanged.has_value() && *props.onSelectionChanged)
                list.selectedIndex.Bind(*props.onSelectionChanged);
            list.SetLabelBuilder(props.labelBuilder.value_or(nullptr));
            list.SetIconBuilder(props.iconBuilder.value_or(nullptr));
            return &list;
        };
    }

    struct BasicListProps : Copiable<BasicListProps> {
        opt<ElementProps> base{std::nullopt};
        opt<std::vector<std::string>> items{std::nullopt};
        opt<int> selectedIndex{std::nullopt};
        opt<ValueChanged<int>> onSelectionChanged{std::nullopt};

        Fields(base, items, selectedIndex, onSelectionChanged)
    };
    WidgetDesc BasicList(const BasicListProps& props);

    // --- ToolButton widget -------------------------------------
    struct ToolButtonProps : Copiable<ToolButtonProps> {
        opt<ElementProps> base{std::nullopt};
        opt<gui::Image*> icon{std::nullopt};
        opt<uint> iconSize{std::nullopt};
        opt<bool> toggled{std::nullopt};
        opt<VoidCallback> onClick{std::nullopt};
        opt<std::string> group{std::nullopt}; // For radio behavior

        Fields(base, icon, iconSize, toggled, onClick, group)
    };
    WidgetDesc ToolButton(const std::string& text, const ToolButtonProps& props);
    WidgetDesc ToolRadioButton(const std::string& text, const ToolButtonProps& props);
    WidgetDesc ToolToggleButton(const std::string& text, const ToolButtonProps& props);

    // --- ProgressBar widget -------------------------------------
    struct ProgressBarProps : Copiable<ProgressBarProps> {
        opt<ElementProps> base{std::nullopt};
        opt<Range> range{std::nullopt};
        opt<float> value{std::nullopt};
        opt<bool> indeterminate{std::nullopt};
        opt<Direction> direction{std::nullopt};

        Fields(base, range, value, indeterminate, direction)
    };
    WidgetDesc ProgressBar(const ProgressBarProps& props);

    // --- Spinner widget -------------------------------------
    struct SpinnerProps : Copiable<SpinnerProps> {
        opt<ElementProps> base{std::nullopt};
        opt<Range> range{std::nullopt};
        opt<float> value{std::nullopt};
        opt<float> step{std::nullopt};
        opt<std::string> formatString{std::nullopt};
        opt<ValueChanged<float>> onValueChange{std::nullopt};

        Fields(base, range, value, step, formatString, onValueChange)
    };
    WidgetDesc Spinner(const SpinnerProps& props = {});

    WidgetDesc Spacer();

    // --- Panel widget -------------------------------------
    struct PanelProps : Copiable<PanelProps> {
        opt<ElementProps> base{std::nullopt};
        opt<EdgeInsets> padding{std::nullopt};
        opt<bool> showBackground{std::nullopt};

        Fields(base, padding, showBackground)
    };
    WidgetDesc Panel(const PanelProps& props, const std::vector<WidgetDesc>& children);

    // --- Color Picker widget -------------------------------------
    struct ColorPickerProps : Copiable<ColorPickerProps> {
        opt<ElementProps> base{std::nullopt};
        opt<Color> selectedColor{std::nullopt};
        opt<ValueChanged<Color>> onChange{std::nullopt};

        Fields(base, selectedColor, onChange)
    };
    WidgetDesc ColorPicker(const ColorPickerProps& props);

    // --- Node Editor widget -------------------------------------
    struct NodeAttrs {
        opt<PointI> position{std::nullopt};
        opt<bool> showPreview{std::nullopt};
    };
    struct NodeEditorProps : Copiable<NodeEditorProps> {
        opt<ElementProps> base{std::nullopt};
        opt<NodeGraph*> graph{std::nullopt};
        opt<std::unordered_map<NodeId, NodeAttrs>> nodeAttributes{std::nullopt};

        Fields(base, graph, nodeAttributes)
    };
    WidgetDesc NodeEditor(const NodeEditorProps& props);
} // namespace gui::declarative
