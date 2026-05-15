#pragma once

#include "Element.h"

#include <any>
#include <concepts>
#include <cstdint>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace gui {
    using NodeId = uint64_t;
    using PinId = uint64_t;

    NodeId NextNodeId();
    PinId NextPinId();

    enum class PinDirection { Input, Output };

    template <typename T>
    concept Pinnable = std::copyable<T> && std::movable<T>;

    class ExecutionContext;
    class Node;
    class NodeGraph;

    template <Pinnable T>
    struct Pin {
        friend class ExecutionContext;
        friend class Node;
        friend class NodeGraph;

        std::string label;
        PinDirection direction{PinDirection::Input};
        std::optional<T> defaultValue;

        std::type_index GetTypeIndex() const { return std::type_index(typeid(T)); }

        Pin(const std::string& label,
            PinDirection direction,
            std::optional<T> defaultValue = std::nullopt)
            : label(label),
              direction(direction),
              defaultValue(defaultValue) {}

    private:
        PinId id{NextPinId()};
    };

    struct Wire {
        PinId from, to;
        bool IsValid() const { return from != to && from != 0 && to != 0; }
    };

    class ExecutionContext {
    public:
        template <Pinnable T>
        void Set(const Pin<T>& pin, T value) {
            m_values[pin.id] = std::move(value);
        }

        template <Pinnable T>
        T Get(const Pin<T>& pin) const {
            auto it = m_values.find(pin.id);
            if (it == m_values.end()) {
                throw std::runtime_error("No value for pin " + std::to_string(pin.id));
            }
            return std::any_cast<T>(it->second);
        }

        template <Pinnable T>
        T GetOr(const Pin<T>& pin, T defaultValue) const {
            auto it = m_values.find(pin.id);
            if (it == m_values.end() || !it->second.has_value()) {
                return defaultValue;
            }
            return std::any_cast<T>(it->second);
        }

        void Propagate(PinId from, PinId to) { m_values[to] = m_values[from]; }
        void Clear() { m_values.clear(); }

    private:
        std::unordered_map<PinId, std::any> m_values;
    };

    class Node {
    public:
        virtual ~Node() = default;
        explicit Node(const std::string& name);

        virtual void Evaluate(ExecutionContext& ctx) = 0;
        virtual void OnDrawPreview(Graphics& g, const ExecutionContext& ctx) {}

        [[nodiscard]] NodeId GetId() const { return m_id; }
        [[nodiscard]] const std::string& GetName() const { return m_name; }
        [[nodiscard]] const auto& GetPins() const { return m_pins; }

        struct PinDesc {
            std::string label;
            PinDirection direction;
            std::type_index type;
            std::any defaultValue;
        };

    protected:
        template <Pinnable T>
        void RegisterPin(const Pin<T>& pin) {
            m_pins.emplace(
                pin.id,
                PinDesc{
                    pin.label,
                    pin.direction,
                    pin.GetTypeIndex(),
                    pin.defaultValue ? std::any(*pin.defaultValue) : std::any{}
                }
            );
        }

        template <Pinnable... Ts>
        void RegisterPins(const Pin<Ts>&... pins) {
            (RegisterPin(pins), ...);
        }

    private:
        NodeId m_id;
        std::string m_name;
        std::map<PinId, PinDesc> m_pins;
    };

    class NodeGraph {
    public:
        template <std::derived_from<Node> T, typename... Args>
        T& AddNode(Args&&... args) {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *node;
            m_nodes[node->GetId()] = std::move(node);
            return ref;
        }

        bool Connect(PinId from, PinId to);

        template <Pinnable P1, Pinnable P2>
        bool Connect(const Pin<P1>& from, const Pin<P2>& to) {
            return Connect(from.id, to.id);
        }

        bool Disconnect(PinId from, PinId to);

        template <Pinnable P1, Pinnable P2>
        bool Disconnect(const Pin<P1>& from, const Pin<P2>& to) {
            return Disconnect(from.id, to.id);
        }

        void Evaluate();

        [[nodiscard]] Node* FindNode(NodeId id) const;
        [[nodiscard]] Wire* FindWire(PinId pid);
        [[nodiscard]] const auto& GetNodes() const { return m_nodes; }
        [[nodiscard]] const auto& GetWires() const { return m_wires; }
        [[nodiscard]] const ExecutionContext& GetContext() const { return m_context; }

    private:
        std::unordered_map<NodeId, std::unique_ptr<Node>> m_nodes;
        std::vector<Wire> m_wires;
        ExecutionContext m_context;

        const Node::PinDesc* FindPin(PinId id) const;
        std::vector<NodeId> GetTopologicalOrder() const;
    };

    class NodeEditor : public Element {
    public:
        NodeEditor();

        std::string StyleKey() const override { return "NodeEditor"; }

        void OnDraw(Graphics& g) override;
        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseMove(MotionEvent e) override;
        void OnKeyDown(KeyEvent e) override;
        void OnKeyUp(KeyEvent e) override;

        void SetPosition(NodeId nid, const PointI& position);
        void SetPreviewVisible(NodeId nid, bool visible);

        Property<NodeGraph*> graph;

    private:
        struct NodeLayout {
            Rectangle rect;
            std::unordered_map<PinId, PointI> pins;
        };

        struct NodeAttributes {
            PointI position;
            bool showPreview{false};
            Image preview;
        };

        std::unordered_map<NodeId, NodeAttributes> m_nodeAttributes;
        mutable std::unordered_map<NodeId, NodeLayout> m_nodeLayouts;
        mutable std::unordered_map<PinId, NodeId> m_pinNode;
        bool m_layoutDirty{true};

        enum { EdIdling = 0, EdSelecting, EdMoving, EdConnecting } m_editorState{EdIdling};

        bool m_shiftPressed{false};
        std::set<NodeId> m_selectedNodes;
        PinId m_selectedPin;

        PointI m_selectionMin, m_selectionMax;
        PointI m_prevMouse;

        const NodeLayout* GetNodeLayout(NodeId nid) const;
        PinId HitTestPin(int x, int y, int radius = 8) const;
        NodeId HitTestNode(int x, int y) const;

        Rectangle GetSelectionRect() const;

        NodeLayout ComputeNodeLayout(NodeId nid, Graphics& g) const;
        void RebuildLayouts(Graphics& g);
    };
} // namespace gui