#pragma once

#include "Container.h"
#include "Layout.h"

#include <memory>
#include <concepts>

namespace tui {
	template <typename L>
	concept DerivedFromLayout = std::derived_from<L, Layout>;

	class Panel : public Container {
	public:
		Panel();

		void Add(Element *element) override;
		void Remove(Element *element) override;

		void OnDraw(Graphics& g) override;
		Size GetPreferredSize() const override;

		void SetLayout(std::unique_ptr<Layout> layout);

		template <DerivedFromLayout L>
		L* GetLayout() { return dynamic_cast<L*>(m_layout.get()); }

		bool IsBackgroundVisible() const { return m_backgroundVisible; }
		void SetBackgroundVisible(bool bv) { m_backgroundVisible = bv; Invalidate(); }

	private:
		std::unique_ptr<Layout> m_layout;
		bool m_backgroundVisible;
	};
}
