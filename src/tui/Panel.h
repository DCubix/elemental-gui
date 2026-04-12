#pragma once

#include "Element.h"
#include "Layout.h"

#include <memory>
#include <concepts>

namespace tui {
	template <typename L>
	concept DerivedFromLayout = std::derived_from<L, Layout>;

	class Panel : public Element {
	public:
		Panel();

		void Add(Element *element);

		void OnDraw(Graphics& g) override;
		Size GetPreferredSize() override;

		void SetLayout(std::unique_ptr<Layout> layout);

		template <DerivedFromLayout L>
		L* GetLayout() { return dynamic_cast<L*>(m_layout.get()); }

		std::vector<Element*> GetChildren() { return m_children; }

		bool IsBackgroundVisible() const { return m_backgroundVisible; }
		void SetBackgroundVisible(bool bv) { m_backgroundVisible = bv; Invalidate(); }

	private:
		std::unique_ptr<Layout> m_layout;
		std::vector<Element*> m_children;
		bool m_backgroundVisible;

		bool IsDirty() override;
	};
}
