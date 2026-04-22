#pragma once

#include "Utils.h"
#include "Element.h"
#include "Application.h"
#include "FlexLayout.h"
#include "ImageView.h"
#include "Menu.h"
#include "List.h"

namespace tui::declarative {
    using WidgetDesc = std::function<Element*(Application&)>;

    // --- Element base props -------------------------------
    struct ElementProps {
        std::string tag{""};
        bool enabled{true};
        bool autoSize{false};
        float flexGrow{0.0f};
        Rectangle bounds{0, 0, 0, 0};
        Json style;
    };

    template <typename T>
    concept HasBaseProps = requires(T t) {
        { t.base } -> std::convertible_to<ElementProps>;
    };

    void ElementSetup(Element& element, const ElementProps& props);

    // --- Column widget ------------------------------------
    struct ColumnProps {
        ElementProps base{};
        int gap{0};
        EdgeInsets padding{0, 0, 0, 0};
        FlexAlign align{FlexAlign::Start};
        FlexJustify justify{FlexJustify::Start};
        bool showBackground{false};
    };
    WidgetDesc Column(const ColumnProps& props, const std::vector<WidgetDesc>& children);

    // --- Row widget --------------------------------------
    struct RowProps {
        ElementProps base{};
        int gap{0};
        EdgeInsets padding{0, 0, 0, 0};
        FlexAlign align{FlexAlign::Start};
        FlexJustify justify{FlexJustify::Start};
        bool showBackground{false};
    };
    WidgetDesc Row(const RowProps& props, const std::vector<WidgetDesc>& children);

    // --- Text widget -------------------------------------
    struct TextProps {
        ElementProps base{};
        Alignment align{Alignment::TopLeft};
        tui::Image* icon{nullptr};
    };
    WidgetDesc Text(const std::string& text, const TextProps& props);

    // --- Button widget -----------------------------------
    struct ButtonProps {
        ElementProps base{};
        tui::Image* icon{nullptr};
        VoidCallback onClick;
    };
    WidgetDesc Button(const std::string& text, const ButtonProps& props);

    // --- TextEdit widget -------------------------------------
    struct TextEditProps {
        ElementProps base{};
        std::string text{""};
        ValueChanged<std::string> onChanged;
        bool multiLine{false};
        bool masked{false};
    };
    WidgetDesc TextEdit(const TextEditProps& props);

    // --- Image widget -------------------------------------
    struct ImageProps {
        ElementProps base{};
        std::string fileName{""}; // TODO: support from memory, etc.
        ImageScalingMode scaling{ImageScalingMode::Stretch};
    };
    WidgetDesc Image(const ImageProps& props);

    // --- ScrollView widget ---------------------------------
    struct ScrollViewProps {
        ElementProps base{};
        Direction scrollDirection{Direction::Vertical};
    };
    WidgetDesc ScrollView(const WidgetDesc& child, const ScrollViewProps& props = {});

    // --- CheckBox widget ----------------------------------
    struct CheckBoxProps {
        ElementProps base{};
        bool checked{false};
        ValueChanged<bool> onChanged;
    };
    WidgetDesc CheckBox(const std::string& text, const CheckBoxProps& props);

    // --- RadioButton widget --------------------------------
    struct RadioButtonProps {
        ElementProps base{};
        std::string group{""};
        bool checked{false};
        ValueChanged<bool> onChanged;
    };
    WidgetDesc RadioButton(const std::string& text, const RadioButtonProps& props);

    // --- Switch widget ------------------------------------
    struct SwitchProps {
        ElementProps base{};
        bool checked{false};
        ValueChanged<bool> onChanged;
    };
    WidgetDesc Switch(const SwitchProps& props);

    // --- Slider widget -------------------------------------
    struct SliderProps {
        ElementProps base{};
        Direction direction{Direction::Horizontal};
        Range range{0.0f, 1.0f};
        float value{0.0f};
        float step{0.01f};
        ValueChanged<float> onValueChange;
    };
    WidgetDesc Slider(const SliderProps& props);

    // --- MenuItem widget ----------------------------------
    using MenuDesc = std::function<tui::Menu*(Application&)>;

    struct MenuItemProps {
        ElementProps base{};
        std::string text{""};
        tui::Image* icon{nullptr};
        bool checked{false};
        VoidCallback onClick;
        MenuDesc subMenu{nullptr};
    };
    WidgetDesc MenuItem(const MenuItemProps& props);
    WidgetDesc MenuSeparator();

    // --- Menu widget -------------------------------------
    struct MenuProps {
        ElementProps base{};
        VoidCallback onDismiss;
    };
    MenuDesc Menu(const MenuProps& props, const std::vector<WidgetDesc>& items);

    // --- Custom widget ------------------------------------
    template <DerivedFromElement Elem, HasBaseProps Props>
    WidgetDesc Custom(const Props& props, std::function<void(Elem&, const Props&)> setup = nullptr) {
        return [props, setup](Application& app) -> Element* {
            auto& element = app.Create<Elem>();
            ElementSetup(element, props.base);
            if (setup) {
                setup(element, props);
            }
            return &element;
        };
    }

    // --- SplitView widget -------------------------------------
    struct SplitViewProps {
        ElementProps base{};
        Direction direction{Direction::Horizontal};
        int splitPosition{100}; // px
        WidgetDesc first{nullptr};
        WidgetDesc second{nullptr};
    };
    WidgetDesc SplitView(const SplitViewProps& props);

    // --- List widget -------------------------------------
    template <typename T>
    struct ListProps {
        ElementProps base{};
        std::vector<ListItem<T>> items;
        int selectedIndex{-1};
        ValueChanged<int> onSelectionChanged;
    };
    
    template <typename T>
    WidgetDesc List(const ListProps<T>& props) {
        return [props](Application& app) -> Element* {
            auto& list = app.Create<tui::List<T>>();
            ElementSetup(list, props.base);
            for (const auto& item : props.items) {
                list.AddItem(item.data, item.label);
            }
            list.SetSelectedIndex(props.selectedIndex);
            list.SetOnSelectionChanged(props.onSelectionChanged);
            return &list;
        };
    }

    struct BasicListProps {
        ElementProps base{};
        std::vector<std::string> items;
        int selectedIndex{-1};
        ValueChanged<int> onSelectionChanged;
    };
    WidgetDesc BasicList(const BasicListProps& props);

    // --- ToolButton widget -------------------------------------
    struct ToolButtonProps {
        ElementProps base{};
        tui::Image* icon{nullptr};
        uint iconSize{16};
        bool toggled{false};
        VoidCallback onClick;
        std::string group{""}; // For radio behavior
    };
    WidgetDesc ToolButton(const std::string& text, const ToolButtonProps& props);
    WidgetDesc ToolRadioButton(const std::string& text, const ToolButtonProps& props);
    WidgetDesc ToolToggleButton(const std::string& text, const ToolButtonProps& props);

    // --- ProgressBar widget -------------------------------------
    struct ProgressBarProps {
        ElementProps base{};
        Range range{0.0f, 1.0f};
        float value{0.0f};
        bool indeterminate{false};
        Direction direction{Direction::Horizontal};
    };
    WidgetDesc ProgressBar(const ProgressBarProps& props);
}