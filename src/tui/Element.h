#pragma once

#include <optional>
#include <array>
#include <regex>
#include <sstream>
#include <concepts>

#include "EventSystem.h"
#include "Graphics.h"

#include "Utils.h"
using namespace tui::utils;

namespace tui {
	class Application;
	class Menu;
	class Element : public Subscriber {
		friend class Application;
		friend class Container;
		friend class ScrollView;
		friend class Menu;
	public:
		Element();
		~Element() = default;

		virtual void OnDraw(Graphics& g);
		virtual EventStatus OnEvent(Event *event) override;

		// Used for Auto-Size
		virtual Size GetPreferredSize() const { return { m_bounds.w, m_bounds.h }; }

		Rectangle GetBounds() const;
		Rectangle GetIntersectedBounds() const;

		const Rectangle& GetLocalBounds() const { return m_bounds; }
		void SetLocalBounds(Rectangle b) { m_bounds = b; Invalidate(); }

		PointI GetPosition() const { return { m_bounds.x, m_bounds.y }; }
		void SetPosition(PointI p) { m_bounds.x = p.x; m_bounds.y = p.y; Invalidate(); }

		Size GetSize() const { return { m_bounds.w, m_bounds.h }; }
		void SetSize(Size s) { m_bounds.w = s.w; m_bounds.h = s.h; Invalidate(); }

		Element* GetParent() const { return m_parent; }

		void Invalidate();

		bool IsVisible() const { return m_visible; }
		void SetVisible(bool v) { m_visible = v; Invalidate(); }

		bool IsFocused() const { return m_focused; }
		void RequestFocus();

		bool IsAutoSize() const { return m_autoSize; }
		void SetAutoSize(bool as) { m_autoSize = as; Invalidate(); }

		float GetFlexGrow() const { return m_flexGrow; }
		void SetFlexGrow(float fg) { m_flexGrow = fg; Invalidate(); }

		Json GetStyle() const;
		void SetStyle(Json style) { m_style = style; Invalidate(); }

		std::string GetTag() const { return m_tag; }
		void SetTag(const std::string& tag) { m_tag = tag; }

		Application* GetApp() const { return m_application; }

	protected:
		Application *m_application;
		Element *m_parent;
		Rectangle m_bounds;

		bool m_dirty, m_visible, m_focused, m_autoSize;
		float m_flexGrow{0.0f};

		Json m_style;

		std::string m_tag{""};

		virtual bool IsDirty() { return m_dirty; }
	};
	
	template <typename E>
	concept DerivedFromElement = std::derived_from<E, Element>;
}
