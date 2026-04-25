#include <Application.h>
#include <backends/sdl3/SDL3Backend.h>
#include <EventSystem.h>
#include <FlexLayout.h>
#include <Declarative.h>

#include <cmath>

using namespace gui;
namespace decl = gui::declarative;

class InfiniteCanvas : public Element {
public:
	enum class Mode {
		DrawPath,
		ErasePath,
		SegmentEdit
	};

	virtual void OnDraw(Graphics& g) override
	{
		auto b = GetBounds();
		g.Rect(b.x, b.y, b.w, b.h);
		g.Color(0.1f, 0.1f, 0.1f);
		g.Fill();

		g.Save();
		g.Translate(b.x, b.y);
		g.SetLineCap(LineCap::Round);
		g.SetLineJoin(LineJoin::Round);

		g.ClipPushRect(0, 0, b.w, b.h);

		for (const auto& path : m_paths) {
			if (m_hoveredPath == &path) {
				g.Color(0.2f, 1.0f, 0.05f);
				g.BeginPath();
				for (const auto& p : path.points) {
					g.AddPathPoint(p.x, p.y);
				}
				g.EndPath();
				g.LineWidth(7.0f);
				g.Stroke();
			}

			g.Color(path.color.r, path.color.g, path.color.b);
			g.BeginPath();
			for (const auto& p : path.points) {
				g.AddPathPoint(p.x, p.y);
			}
			g.EndPath();
			g.LineWidth(5.0f);
			g.Stroke();
		}

		if (m_selectedPath) {
			for (const auto& p : m_selectedPath->points) {
				bool isSelected = (m_selectedPoint == &p);
				if (isSelected) {
					g.Color(0.0f, 1.0f, 0.0f);
				} else {
					g.Color(1.0f, 0.0f, 0.0f, 0.35f);
				}
				g.Arc(p.x, p.y, isSelected ? 5.0f : 4.0f, 0.0f, 2.0f * M_PI);
				g.Fill();
			}
		}

		if (!m_tempPoints.empty()) {
			g.Color(1.0f, 1.0f, 1.0f);
			g.BeginPath();
			for (const auto& p : m_tempPoints) {
				g.AddPathPoint(p.x, p.y);
			}
			g.EndPath();
			g.LineWidth(5.0f);
			g.Stroke();
		}
		g.ClipPop();

		g.Restore();
	}

	virtual EventStatus OnEvent(Event *event) override
	{
		Rectangle b = GetIntersectedBounds();
		if (event->Type() == EventType::MouseButton) {
			auto* e = static_cast<MouseEvent*>(event);
			if (e->button == MouseButton::Left && e->pressed && b.HasPoint(e->x, e->y)) {
				switch (m_mode) {
					case Mode::DrawPath:    return OnMouseDown_ModeDrawPath(event);
					case Mode::ErasePath:   return OnMouseDown_ModeErasePath(event);
					case Mode::SegmentEdit: return OnMouseDown_ModeSegmentEdit(event);
				}
			} else if (e->button == MouseButton::Left && !e->pressed && b.HasPoint(e->x, e->y)) {
				switch (m_mode) {
					case Mode::DrawPath:    return OnMouseUp_ModeDrawPath(event);
					case Mode::ErasePath:   return OnMouseUp_ModeErasePath(event);
					case Mode::SegmentEdit: return OnMouseUp_ModeSegmentEdit(event);
				}
			}
		} else if (event->Type() == EventType::MouseMotion) {
			switch (m_mode) {
				case Mode::DrawPath:    return OnMouseMove_ModeDrawPath(event);
				case Mode::ErasePath:   return OnMouseMove_ModeErasePath(event);
				case Mode::SegmentEdit: return OnMouseMove_ModeSegmentEdit(event);
			}
		}
		return EventStatus::Active;
	}

	Mode GetMode() const { return m_mode; }
	void SetMode(Mode mode) { m_mode = mode; Invalidate(); }

private:
	bool m_dragging{ false };
	Mode m_mode{ Mode::DrawPath };

	struct Path {
		std::vector<PointI> points;
		Color color{ Color::FromHex("#FFF") };
	};

	std::vector<Path> m_paths;
	Path* m_hoveredPath{ nullptr }, *m_selectedPath{ nullptr };
	PointI* m_hoveredPoint{ nullptr }, *m_selectedPoint{ nullptr };
	std::vector<PointI> m_tempPoints;

	PointI* GetHoveredPoint(Path* path, const PointI& mp)
	{
		for (auto& point : path->points) {
			if (point.DistanceTo(mp) < 10) {
				return &point;
			}
		}
		return nullptr;
	}

	std::vector<PointI*> GetNeighdorPoints(Path* path, PointI* pt)
	{
		std::vector<PointI*> neighbors;
		for (size_t i = 0; i < path->points.size(); i++) {
			PointI* curr = &path->points[i];
			if (curr == pt) {
				if (i > 0) neighbors.push_back(&path->points[i - 1]);
				if (i < path->points.size() - 1) neighbors.push_back(&path->points[i + 1]);
				break;
			}
		}
		return neighbors;
	}

	EventStatus OnMouseDown_ModeSegmentEdit(Event* event)
	{
		auto b = GetBounds();
		auto* e = static_cast<MouseEvent*>(event);
		auto mp = PointI{ e->x - b.x, e->y - b.y };

		auto* path = GetHoveredPath(mp);
		if (!m_selectedPath) {
			m_selectedPath = path;
			Invalidate();
		} else {
			if (m_selectedPath == path) {
				auto* point = GetHoveredPoint(path, mp);
				if (point) {
					m_selectedPoint = point;
					Invalidate();
				}
			} else {
				m_selectedPath = nullptr;
				m_selectedPoint = nullptr;
				Invalidate();
			}
		}
		m_dragging = true;
		return EventStatus::Consumed;
	}

	EventStatus OnMouseUp_ModeSegmentEdit(Event* event)
	{
		m_dragging = false;
		m_selectedPoint = nullptr;
		return EventStatus::Consumed;
	}

	EventStatus OnMouseMove_ModeSegmentEdit(Event* event)
	{
		auto b = GetBounds();
		auto* e = static_cast<MouseEvent*>(event);
		auto mp = PointI{ e->x - b.x, e->y - b.y };
		if (m_dragging) {
			if (m_selectedPoint) {
				m_selectedPoint->x = mp.x;
				m_selectedPoint->y = mp.y;
				Invalidate();
			}
		} else {
			m_hoveredPath = GetHoveredPath(mp);
			if (m_selectedPath) {
				m_hoveredPoint = GetHoveredPoint(m_selectedPath, mp);
			}
			Invalidate();
		}
		return EventStatus::Consumed;
	}

	Path* GetHoveredPath(const PointI& mp)
	{
		for (auto& path : m_paths) {
			for (const auto& point : path.points) {
				if (point.DistanceTo(mp) <= 8) return &path;
			}
		}
		return nullptr;
	}

	void RemovePath(Path* path)
	{
		m_hoveredPath = nullptr;
		m_paths.erase(std::remove_if(m_paths.begin(), m_paths.end(), [path](const Path& p) {
			return &p == path;
		}), m_paths.end());
		Invalidate();
	}

	EventStatus OnMouseDown_ModeErasePath(Event* event)
	{
		auto b = GetBounds();
		auto* e = static_cast<MouseEvent*>(event);
		auto mp = PointI{ e->x - b.x, e->y - b.y };
		auto* path = GetHoveredPath(mp);
		if (path) RemovePath(path);
		m_dragging = true;
		return EventStatus::Consumed;
	}

	EventStatus OnMouseMove_ModeErasePath(Event* event)
	{
		auto b = GetBounds();
		auto* e = static_cast<MouseEvent*>(event);
		auto mp = PointI{ e->x - b.x, e->y - b.y };
		if (m_dragging) {
			auto* path = GetHoveredPath(mp);
			if (path) RemovePath(path);
		} else {
			m_hoveredPath = GetHoveredPath(mp);
			Invalidate();
		}
		return EventStatus::Consumed;
	}

	EventStatus OnMouseUp_ModeErasePath(Event* event)
	{
		m_dragging = false;
		return EventStatus::Consumed;
	}

	EventStatus OnMouseDown_ModeDrawPath(Event* event)
	{
		auto b = GetBounds();
		m_dragging = true;
		auto* e = static_cast<MouseEvent*>(event);
		m_tempPoints.reserve(100);
		auto pt = PointI{ e->x - b.x, e->y - b.y };
		m_tempPoints.push_back(pt);
		Invalidate();
		return EventStatus::Consumed;
	}

	EventStatus OnMouseMove_ModeDrawPath(Event* event)
	{
		auto b = GetBounds();
		auto* e = static_cast<MouseEvent*>(event);
		if (m_dragging) {
			auto pt = PointI{ e->x - b.x, e->y - b.y };
			m_tempPoints.push_back(pt);
			Invalidate();
		}
		return EventStatus::Consumed;
	}

	EventStatus OnMouseUp_ModeDrawPath(Event* event)
	{
		m_dragging = false;
		if (m_tempPoints.size() > 1) {
			SimplifyTempPath();
			m_paths.push_back({ m_tempPoints, Color::FromHex("#FFF") });
		}
		m_tempPoints.clear();
		Invalidate();
		return EventStatus::Consumed;
	}

	void SimplifyTempPath()
	{
		const int minDistanceBetweenPoints = 6;
		if (m_tempPoints.size() < 2) return;

		std::vector<PointI> simplified;
		simplified.reserve(m_tempPoints.size());
		simplified.push_back(m_tempPoints[0]);

		for (size_t i = 1; i < m_tempPoints.size(); i++) {
			const auto& last = simplified.back();
			const auto& current = m_tempPoints[i];
			if (last.DistanceTo(current) >= minDistanceBetweenPoints) {
				simplified.push_back(current);
			}
		}

		m_tempPoints = std::move(simplified);
	}
};

struct InfiniteCanvasProps {
	decl::ElementProps base;
};

class App : public ApplicationAdapter {
public:
	gui::Image icPen, icEraser, icSegment;
	gui::Window* winMain{ nullptr };

	void OnCreate(Application& app) {
		icPen     = gui::Image("pen.svg");
		icEraser  = gui::Image("eraser.svg");
		icSegment = gui::Image("segment.svg");

		winMain = app.CreateWindow({
			.title = "Drawing Pad",
			.width = 800,
			.height = 600
		});

		auto toolSize = decl::ElementProps{ .bounds = Rectangle::FromHeight(34) };
		auto toolBox = decl::Column({
			.base = { .bounds = Rectangle::FromWidth(45) },
			.gap = 4,
			.padding = EdgeInsets::All(4),
			.align = FlexAlign::Stretch,
			.justify = FlexJustify::Start,
			.showBackground = true
		}, {
			decl::ToolRadioButton("", { .base = toolSize, .icon = &icPen, .toggled = true, .onClick = [this]() {
				auto canvas = winMain->FindByTag<InfiniteCanvas>("canvas");
				if (canvas) canvas->SetMode(InfiniteCanvas::Mode::DrawPath);
			}}),
			decl::ToolRadioButton("", { .base = toolSize, .icon = &icEraser, .onClick = [this]() {
				auto canvas = winMain->FindByTag<InfiniteCanvas>("canvas");
				if (canvas) canvas->SetMode(InfiniteCanvas::Mode::ErasePath);
			}}),
			decl::ToolRadioButton("", { .base = toolSize, .icon = &icSegment, .onClick = [this]() {
				auto canvas = winMain->FindByTag<InfiniteCanvas>("canvas");
				if (canvas) canvas->SetMode(InfiniteCanvas::Mode::SegmentEdit);
			}}),
		});

		auto gui = decl::Row({
			.gap = 8,
			.align = FlexAlign::Stretch,
			.justify = FlexJustify::Start
		}, {
			toolBox,
			decl::Custom<InfiniteCanvas, InfiniteCanvasProps>({
				.base = { .tag = "canvas", .flexGrow = 1.0f }
			}, [](InfiniteCanvas&, const InfiniteCanvasProps&) {})
		});

		winMain->SetRoot(gui(*winMain));
		winMain->Show();
	}

	void OnDestroy() {}
};

int main(int argc, char** argv) {
	gui::Application app{};
	app.SetBackend(std::make_unique<gui::SDL3Backend>());
	return app.Start(new App());
}
