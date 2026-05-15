#include <iostream>
#include <memory>
#include <vector>

#include <elemental/Application.h>
#include <elemental/Backend.h>
#include <elemental/Declarative.h>
#include <elemental/backends/sdl3/SDL3Backend.h>

using namespace gui;
namespace dc = gui::declarative;

class ValueNode : public Node {
public:
    float value{0.0f};
    Pin<float> pinValue{"Value", gui::PinDirection::Output, 0.0f};

    ValueNode(float val)
        : Node("Value"),
          value(val) {
        RegisterPin(pinValue);
    }

    void Evaluate(ExecutionContext& ctx) override { ctx.Set(pinValue, value); }
};

class ResultNode : public Node {
public:
    Pin<float> pinResult{"Value", gui::PinDirection::Input, 0.0f};

    ResultNode()
        : Node("Result") {
        RegisterPin(pinResult);
    }

    void Evaluate(ExecutionContext& ctx) override {}
    void OnDrawPreview(Graphics& g, const ExecutionContext& ctx) override {
        g.Clear(0.0f, 0.0f, 0.0f, 1.0f);
        g.Font(FontStyle::Bold, "Sans", 20.0);
        g.Color(1.0f, 1.0f, 1.0f);
        g.StyledTextEnd(std::to_string(ctx.GetOr(pinResult, 0.0f)), 8, 18);
    }
};

class AddNode : public Node {
public:
    Pin<float> pinA{"A", gui::PinDirection::Input, 0.0f};
    Pin<float> pinB{"B", gui::PinDirection::Input, 0.0f};
    Pin<float> pinResult{"Result", gui::PinDirection::Output, 0.0f};

    AddNode()
        : Node("Add") {
        RegisterPins(pinA, pinB, pinResult);
    }

    void Evaluate(ExecutionContext& ctx) override {
        auto a = ctx.GetOr(pinA, 0.0f);
        auto b = ctx.GetOr(pinB, 0.0f);
        ctx.Set(pinResult, a + b);
    }
};

class App : public gui::Window {
public:
    App()
        : gui::Window(WindowConfig{.title = "Node Editor", .width = 800, .height = 600}) {}

    NodeGraph graph{};

    dc::WidgetDesc OnBuild() override {
        Show();

        auto& n1 = graph.AddNode<AddNode>();
        auto& n2 = graph.AddNode<ValueNode>(2.0f);
        auto& n3 = graph.AddNode<ValueNode>(6.2f);
        auto& n4 = graph.AddNode<ResultNode>();

        // graph.Connect(n2.pinValue, n1.pinA);
        // graph.Connect(n3.pinValue, n1.pinB);

        // clang-format off
        auto ui = dc::NodeEditor({
            .graph = &graph,
            .nodeAttributes = {{
                {n1.GetId(), dc::NodeAttrs{ .position = PointI{300, 60} }},
                {n2.GetId(), dc::NodeAttrs{ .position = PointI{60, 60} }},
                {n3.GetId(), dc::NodeAttrs{ .position = PointI{60, 160} }},
                {n4.GetId(), dc::NodeAttrs{ .position = PointI{430, 60}, .showPreview = true }},
            }},
        });
        // clang-format on
        return ui;
    }
};

int main() {
    Application app{new SDL3Backend()};
    app.CreateWindow<App>();
    return app.Start();
}
