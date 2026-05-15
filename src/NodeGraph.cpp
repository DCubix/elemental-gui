#include "NodeGraph.h"

#include "Layout.h"
#include "Window.h"

#include <atomic>
#include <iostream>

namespace gui {
    NodeId NextNodeId() {
        static std::atomic<NodeId> counter{1};
        return counter.fetch_add(1, std::memory_order_relaxed);
    }

    PinId NextPinId() {
        static std::atomic<PinId> counter{1};
        return counter.fetch_add(1, std::memory_order_relaxed);
    }

    Node::Node(const std::string& name)
        : m_id(NextNodeId()),
          m_name(name) {}

    bool NodeGraph::Connect(PinId from, PinId to) {
        Wire wr{from, to};
        if (!wr.IsValid())
            return false;

        auto* fromPin = FindPin(from);
        auto* toPin = FindPin(to);
        if (!fromPin || !toPin)
            return false;
        if (fromPin->type != toPin->type)
            return false;
        if (fromPin->direction != PinDirection::Output)
            return false;
        if (toPin->direction != PinDirection::Input)
            return false;

        for (auto& wx : m_wires) {
            if (wx.from == from && wx.to == to)
                return false;
        }

        m_wires.push_back(wr);
        Evaluate();
        return true;
    }

    bool NodeGraph::Disconnect(PinId from, PinId to) {
        auto it = std::ranges::find_if(m_wires, [&](const Wire& w) {
            return w.from == from && w.to == to;
        });
        if (it == m_wires.end())
            return false;
        m_wires.erase(it);
        Evaluate();
        return true;
    }

    void NodeGraph::Evaluate() {
        m_context.Clear();
        for (NodeId nid : GetTopologicalOrder()) {
            for (auto& wire : m_wires) {
                m_context.Propagate(wire.from, wire.to);
            }
            if (auto* node = FindNode(nid)) {
                node->Evaluate(m_context);
            }
        }
    }

    Node* NodeGraph::FindNode(NodeId id) const {
        auto it = m_nodes.find(id);
        return it != m_nodes.end() ? it->second.get() : nullptr;
    }

    Wire* NodeGraph::FindWire(PinId pid) {
        for (auto& wire : m_wires) {
            if (wire.from == pid || wire.to == pid)
                return &wire;
        }
        return nullptr;
    }

    const Node::PinDesc* NodeGraph::FindPin(PinId id) const {
        for (auto& [_, node] : m_nodes) {
            auto& pins = node->GetPins();
            if (auto it = pins.find(id); it != pins.end()) {
                return &it->second;
            }
        }
        return nullptr;
    }

    std::vector<NodeId> NodeGraph::GetTopologicalOrder() const {
        std::unordered_map<NodeId, int> inputDegree;
        std::unordered_map<PinId, NodeId> pinOwner;

        for (auto& [id, node] : m_nodes) {
            inputDegree[id] = 0;
            for (auto& [pid, desc] : node->GetPins()) {
                if (desc.direction == PinDirection::Input)
                    pinOwner[pid] = id;
            }
        }

        for (auto& wire : m_wires) {
            if (auto it = pinOwner.find(wire.to); it != pinOwner.end()) {
                inputDegree[it->second]++;
            }
        }

        std::vector<NodeId> queue, order;
        for (auto& [id, deg] : inputDegree) {
            if (deg == 0) {
                queue.push_back(id);
            }
        }

        while (!queue.empty()) {
            NodeId curr = queue.back();
            queue.pop_back();
            order.push_back(curr);

            auto* node = FindNode(curr);
            if (!node)
                continue;

            for (auto& [pid, desc] : node->GetPins()) {
                if (desc.direction != PinDirection::Output)
                    continue;
                for (auto& wire : m_wires) {
                    if (wire.from != pid)
                        continue;
                    if (auto it = pinOwner.find(wire.to); it != pinOwner.end()) {
                        if (--inputDegree[it->second] == 0) {
                            queue.push_back(it->second);
                        }
                    }
                }
            }
        }

        return order;
    }

    // --- Node Editor element ------------------------------------------------

    constexpr int gridSize = 8;
    constexpr int textSideSpacing = 40;
    constexpr int inOutVerticalGap = 20;
    constexpr int nodeTitleHeight = 28;
    constexpr int pinSpacing = 6;
    constexpr int nodePreviewSize = 100;
    static const EdgeInsets nodePadding = EdgeInsets::All(8);

    static const Color pinColorPalette[] = {
        Color::FromHex("#FFB347"), // amber
        Color::FromHex("#6EC6F5"), // sky blue
        Color::FromHex("#82E07A"), // green
        Color::FromHex("#C47AE0"), // violet
        Color::FromHex("#4DD0C4"), // teal
        Color::FromHex("#F06C6C"), // red
        Color::FromHex("#F5E642"), // yellow
        Color::FromHex("#A0B8FF"), // lavender
        Color::FromHex("#FF8C94"), // salmon
        Color::FromHex("#FFD580"), // gold
        Color::FromHex("#80E8B0"), // mint
        Color::FromHex("#7AB8F5"), // cornflower
        Color::FromHex("#E07AB8"), // rose
        Color::FromHex("#7AE0D4"), // aqua
        Color::FromHex("#D4E07A"), // lime
        Color::FromHex("#E0A87A"), // peach
        Color::FromHex("#B87AE0"), // purple
        Color::FromHex("#7AE080"), // jade
        Color::FromHex("#F5A442"), // tangerine
        Color::FromHex("#42C8F5"), // cyan
        Color::FromHex("#F542A4"), // magenta
        Color::FromHex("#A4F542"), // chartreuse
        Color::FromHex("#C8A47A"), // sand
        Color::FromHex("#7AA4C8"), // steel
    };
    constexpr int pinPaletteSize = static_cast<int>(std::size(pinColorPalette));

    static Color PinColor(const std::type_index& type) {
        return pinColorPalette[type.hash_code() % pinPaletteSize];
    }

    NodeEditor::NodeEditor()
        : Element() {
        TrackAll(graph);
    }

    void NodeEditor::OnDraw(Graphics& g) {
        Json style = GetStyle();
        Json nodeStyle = style["node"];
        Color connectionColor = Color::FromStyle(style["connectionColor"]);

        auto sz = GetSize();

        g.StyledRect(0, 0, sz.w, sz.h, style);

        // draw grid
        int startX = (m_pan.x % gridSize);
        int startY = (m_pan.y % gridSize);

        g.Color(0.5f, 0.5f, 0.5f, 0.2f);
        for (int sx = startX; sx < sz.w; sx += gridSize) {
            for (int sy = startY; sy < sz.h; sy += gridSize) {
                g.Arc(sx, sy, 1.0f, 0.0f, M_PI * 2.0f);
                g.Fill();
            }
        }

        if (!graph())
            return;

        RebuildLayouts(g);

        float nodeBorderRadius =
            nodeStyle["border"].is_object() ? nodeStyle["border"].value("radius", 0.0f) : 0.0f;

        g.ClipPushRect(0, 0, sz.w, sz.h);

        g.Save();
        g.Translate(m_pan.x, m_pan.y);

        for (const auto& nid : m_selectedNodes) {
            const auto& layout = m_nodeLayouts[nid];
            const auto& rect = layout.rect;
            g.Save();
            g.Translate(rect.x, rect.y);

            g.Color(0.5f, 0.5f, 0.5f, 0.5f);
            g.LineWidth(5.0f);
            g.RoundRect(0, 0, rect.w, rect.h, nodeBorderRadius);
            g.Stroke();

            g.Restore();
        }

        for (auto& wire : graph()->GetWires()) {
            PointI from = m_nodeLayouts[m_pinNode[wire.from]].pins[wire.from];
            PointI to = m_nodeLayouts[m_pinNode[wire.to]].pins[wire.to];

            float tension = std::max(80.0f, std::abs(to.x - from.x) * 0.5f);
            PointI cp1{static_cast<int>(from.x + tension), from.y};
            PointI cp2{static_cast<int>(to.x - tension), to.y};

            auto* fromNode = graph()->FindNode(m_pinNode[wire.from]);
            auto pin = fromNode->GetPins().at(wire.from);
            auto wireColor = PinColor(pin.type);

            g.Color(wireColor.r, wireColor.g, wireColor.b);
            g.MoveTo(from.x, from.y);
            g.CurveTo(cp1.x, cp1.y, cp2.x, cp2.y, to.x, to.y);
            g.LineWidth(3.0f);
            g.Stroke();
        }

        for (auto& [nid, node] : graph()->GetNodes()) {
            auto& attrs = m_nodeAttributes[nid];

            if (attrs.showPreview) {
                if (!attrs.preview.IsValid()) {
                    attrs.preview = Image(nodePreviewSize, nodePreviewSize);
                }

                // draw preview
                auto g = Graphics::CreateGraphics(attrs.preview);
                node->OnDrawPreview(g, graph()->GetContext());
            }

            const auto& layout = m_nodeLayouts[nid];
            const auto& rect = layout.rect;

            g.Save();
            g.Translate(rect.x, rect.y);

            g.StyledRect(0, 0, rect.w, rect.h, nodeStyle);

            g.ClipPushRoundRect(0, 0, rect.w, rect.h, nodeBorderRadius);
            g.Color(0.0f, 0.0f, 0.0f, 0.35f);
            g.Rect(0, 0, rect.w, nodeTitleHeight);
            g.Fill();

            g.StyledTextBegin(nodeStyle);
            auto titleTM = g.MeasureText(node->GetName());
            g.StyledTextEnd(
                node->GetName(),
                nodePadding.left,
                (nodeTitleHeight / 2 - titleTM.size.h / 2) - titleTM.yBearing
            );

            if (attrs.showPreview) {
                g.DrawImage(&attrs.preview, 0, nodeTitleHeight, nodePreviewSize, nodePreviewSize);
            }

            for (auto& [pid, desc] : node->GetPins()) {
                if (desc.direction != PinDirection::Output)
                    continue;

                const int localCY = layout.pins.at(pid).y - rect.y;

                g.StyledTextBegin(nodeStyle);
                auto tm = g.MeasureText(desc.label);
                g.StyledTextEnd(
                    desc.label,
                    rect.w - tm.size.w - nodePadding.right,
                    localCY - tm.size.h / 2 - tm.yBearing
                );

                g.ClipPop();
                auto outColor = m_selectedPin == pid ? connectionColor : PinColor(desc.type);
                g.Color(outColor.r, outColor.g, outColor.b, outColor.a);
                g.Arc(rect.w, localCY, 5.0f, 0.0f, M_PI * 2.0f);
                g.ClosePath();
                g.Fill();
                g.ClipPushRoundRect(0, 0, rect.w, rect.h, nodeBorderRadius);
            }

            for (auto& [pid, desc] : node->GetPins()) {
                if (desc.direction != PinDirection::Input)
                    continue;

                const int localCY = layout.pins.at(pid).y - rect.y;

                g.StyledTextBegin(nodeStyle);
                auto tm = g.MeasureText(desc.label);
                g.StyledTextEnd(
                    desc.label,
                    nodePadding.left,
                    localCY - tm.size.h / 2 - tm.yBearing
                );

                g.ClipPop();
                auto inColor = m_selectedPin == pid ? connectionColor : PinColor(desc.type);
                g.Color(inColor.r, inColor.g, inColor.b, inColor.a);
                g.Arc(0, localCY, 5.0f, 0.0f, M_PI * 2.0f);
                g.ClosePath();
                g.Fill();
                g.ClipPushRoundRect(0, 0, rect.w, rect.h, nodeBorderRadius);
            }

            g.ClipPop();

            g.Restore();
        }

        if (m_editorState == EdConnecting) {
            PointI from = m_nodeLayouts[m_pinNode[m_selectedPin]].pins[m_selectedPin];
            PointI to = m_prevMouse - m_pan;

            if (to.x < from.x)
                std::swap(from, to);

            float tension = std::max(80.0f, std::abs(to.x - from.x) * 0.5f);
            PointI cp1{static_cast<int>(from.x + tension), from.y};
            PointI cp2{static_cast<int>(to.x - tension), to.y};

            g.Color(connectionColor.r, connectionColor.g, connectionColor.b);
            g.MoveTo(from.x, from.y);
            g.CurveTo(cp1.x, cp1.y, cp2.x, cp2.y, to.x, to.y);
            g.LineWidth(3.0f);
            g.Stroke();
        }

        if (m_editorState == EdSelecting) {
            auto rect = GetSelectionRect();
            g.Color(0.5f, 0.5f, 0.5f, 0.2f);
            g.Rect(rect.x, rect.y, rect.w, rect.h);
            g.Fill(true);
            g.Color(0.5f, 0.5f, 0.5f, 0.6f);
            g.LineWidth(1.0f);
            g.Stroke();
        }

        g.Restore();

        g.ClipPop();
    }

    void NodeEditor::OnMouseDown(MouseEvent e) {
        m_selectionMin = {e.x, e.y};
        m_selectionMax = {e.x, e.y};
        m_prevMouse = {e.x, e.y};

        if (e.button == MouseButton::Left && m_editorState != EdMoving) {
            PinId hitPin = HitTestPin(e.x, e.y, 6);
            if (hitPin) {
                auto* node = graph()->FindNode(m_pinNode[hitPin]);
                const auto& pin = node->GetPins().at(hitPin);
                if (pin.direction == PinDirection::Input) {
                    auto* wire = graph()->FindWire(hitPin);
                    if (wire) {
                        hitPin = wire->from;
                        graph()->Disconnect(wire->from, wire->to);
                    }
                }
            }

            if (hitPin) {
                m_editorState = EdConnecting;
                m_selectedPin = hitPin;
            } else {
                m_editorState = EdMoving;
            }
        } else if (e.button == MouseButton::Right) {
            NodeId hitNode = HitTestNode(e.x, e.y);
            if (!hitNode) {
                m_selectedNodes.clear();
                m_editorState = EdSelecting;
            } else {
                if (!m_shiftPressed) {
                    m_selectedNodes.clear();
                }

                if (m_selectedNodes.contains(hitNode)) {
                    m_selectedNodes.erase(hitNode);
                } else {
                    m_selectedNodes.insert(hitNode);
                }
            }
        } else if (e.button == MouseButton::Middle) {
            m_editorState = EdPanning;
        }

        Invalidate();
    }

    void NodeEditor::OnMouseUp(MouseEvent e) {
        if (m_editorState == EdConnecting) {
            PinId hitPin = HitTestPin(e.x, e.y, 6);
            if (hitPin) {
                auto nodeFrom = graph()->FindNode(m_pinNode[m_selectedPin]);
                auto nodeTo = graph()->FindNode(m_pinNode[hitPin]);
                const auto& fromPin = nodeFrom->GetPins().at(m_selectedPin);
                const auto& toPin = nodeTo->GetPins().at(hitPin);

                if (fromPin.direction == PinDirection::Input &&
                    toPin.direction == PinDirection::Output) {
                    std::swap(m_selectedPin, hitPin);
                }

                graph()->Connect(m_selectedPin, hitPin);
            }
        }

        m_editorState = EdIdling;
        m_selectedPin = 0;
        Invalidate();
    }

    void NodeEditor::OnMouseMove(MotionEvent e) {
        if (m_editorState == EdSelecting) {
            m_selectionMax = {e.x, e.y};

            m_selectedNodes.clear();
            auto rect = GetSelectionRect();
            for (auto& [nid, _] : graph()->GetNodes()) {
                const auto& layout = m_nodeLayouts[nid];
                if (rect.Intersects(layout.rect)) {
                    m_selectedNodes.insert(nid);
                }
            }

            Invalidate();
        } else if (m_editorState == EdMoving) {
            for (const auto& nid : m_selectedNodes) {
                auto& attr = m_nodeAttributes[nid];
                attr.rawPosition.x += e.x - m_prevMouse.x;
                attr.rawPosition.y += e.y - m_prevMouse.y;

                if (m_snapping) {
                    attr.position.x = std::floor(float(attr.rawPosition.x) / gridSize) * gridSize;
                    attr.position.y = std::floor(float(attr.rawPosition.y) / gridSize) * gridSize;
                } else {
                    attr.position = attr.rawPosition;
                }

                m_nodeLayouts[nid] = ComputeNodeLayout(nid, GetWindow()->GetGraphics());
            }

            m_prevMouse = {e.x, e.y};
            Invalidate();
        } else if (m_editorState == EdConnecting) {
            m_prevMouse = {e.x, e.y};
            Invalidate();
        } else if (m_editorState == EdPanning) {
            m_pan.x += e.x - m_prevMouse.x;
            m_pan.y += e.y - m_prevMouse.y;

            m_prevMouse = {e.x, e.y};
            Invalidate();
        }
    }

    void NodeEditor::OnKeyDown(KeyEvent e) {
        if (e.key == Key::LeftShift || e.key == Key::RightShift) {
            m_shiftPressed = true;
        }
    }

    void NodeEditor::OnKeyUp(KeyEvent e) {
        if (e.key == Key::LeftShift || e.key == Key::RightShift) {
            m_shiftPressed = false;
        }
    }

    void NodeEditor::SetPosition(NodeId nid, const PointI& position) {
        auto& attr = m_nodeAttributes[nid];
        attr.rawPosition = position;

        if (m_snapping) {
            attr.position.x = std::floor(float(attr.rawPosition.x) / gridSize) * gridSize;
            attr.position.y = std::floor(float(attr.rawPosition.y) / gridSize) * gridSize;
        } else {
            attr.position = attr.rawPosition;
        }

        m_layoutDirty = true;
        Invalidate();
    }

    void NodeEditor::SetPreviewVisible(NodeId nid, bool visible) {
        m_nodeAttributes[nid].showPreview = visible;
        Invalidate();
    }

    Rectangle NodeEditor::GetSelectionRect() const {
        PointI tl = {
            std::min(m_selectionMin.x, m_selectionMax.x),
            std::min(m_selectionMin.y, m_selectionMax.y)
        };
        PointI br = {
            std::max(m_selectionMin.x, m_selectionMax.x),
            std::max(m_selectionMin.y, m_selectionMax.y)
        };
        tl = tl - m_pan;
        br = br - m_pan;
        return Rectangle{tl.x, tl.y, br.x - tl.x, br.y - tl.y};
    }

    NodeEditor::NodeLayout NodeEditor::ComputeNodeLayout(NodeId nid, Graphics& g) const {
        auto* node = graph()->FindNode(nid);
        NodeAttributes attrs{};
        if (auto it = m_nodeAttributes.find(nid); it != m_nodeAttributes.end())
            attrs = it->second;

        PointI pos = attrs.position;

        const auto nodeStyle = GetStyle()["node"];

        g.Save();
        g.StyledTextBegin(nodeStyle);

        std::unordered_map<PinId, TextExtents> pinTM;
        int maxLabelW = 0;
        for (auto& [pid, desc] : node->GetPins()) {
            auto tm = g.MeasureText(desc.label);
            pinTM[pid] = tm;
            maxLabelW = std::max(maxLabelW, tm.size.w);
            m_pinNode[pid] = nid;
        }

        g.Restore();

        const int computedW =
            static_cast<int>(maxLabelW + textSideSpacing + nodePadding.GetHorizontal());
        const int nodeW = attrs.showPreview ? std::max(computedW, nodePreviewSize) : computedW;

        NodeLayout layout;

        int pinY = nodeTitleHeight;
        if (attrs.showPreview)
            pinY += nodePreviewSize;
        pinY += pinSpacing;

        int outCount = 0;

        for (auto& [pid, desc] : node->GetPins()) {
            if (desc.direction != PinDirection::Output)
                continue;
            auto& tm = pinTM[pid];
            layout.pins[pid] = PointI{pos.x + nodeW, pos.y + pinY + tm.size.h / 2};
            pinY += pinSpacing + tm.size.h;
            outCount++;
        }

        if (outCount > 0)
            pinY -= pinSpacing;

        int inCount = 0;
        for (auto& [pid, desc] : node->GetPins()) {
            if (desc.direction != PinDirection::Input)
                continue;
            if (inCount == 0)
                pinY += inOutVerticalGap;
            auto& tm = pinTM[pid];
            layout.pins[pid] = PointI{pos.x, pos.y + pinY + tm.size.h / 2};
            pinY += pinSpacing + tm.size.h;
            inCount++;
        }

        int lastContentBottom;
        if (inCount > 0)
            lastContentBottom = pinY - pinSpacing;
        else if (outCount > 0)
            lastContentBottom = pinY;
        else
            lastContentBottom = nodeTitleHeight;

        const int nodeH = lastContentBottom + nodePadding.bottom;

        layout.rect = {pos.x, pos.y, nodeW, nodeH};
        return layout;
    }

    void NodeEditor::RebuildLayouts(Graphics& g) {
        if (!m_layoutDirty)
            return;
        m_nodeLayouts.clear();
        for (auto& [nid, _] : graph()->GetNodes())
            m_nodeLayouts[nid] = ComputeNodeLayout(nid, g);
        m_layoutDirty = false;
    }

    const NodeEditor::NodeLayout* NodeEditor::GetNodeLayout(NodeId nid) const {
        auto it = m_nodeLayouts.find(nid);
        return it != m_nodeLayouts.end() ? &it->second : nullptr;
    }

    PinId NodeEditor::HitTestPin(int x, int y, int radius) const {
        x -= m_pan.x;
        y -= m_pan.y;
        for (auto& [nid, layout] : m_nodeLayouts) {
            for (auto& [pid, pin] : layout.pins) {
                int dx = x - pin.x;
                int dy = y - pin.y;
                if (dx * dx + dy * dy <= radius * radius)
                    return pid;
            }
        }
        return 0;
    }

    NodeId NodeEditor::HitTestNode(int x, int y) const {
        for (auto& [nid, layout] : m_nodeLayouts) {
            if (layout.rect.HasPoint(x - m_pan.x, y - m_pan.y))
                return nid;
        }
        return 0;
    }
} // namespace gui