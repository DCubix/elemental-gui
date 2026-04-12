#pragma once

namespace tui {
	class Layout {
	public:
		virtual ~Layout() = default;
		virtual void Apply(int x, int y, int w, int h) = 0;

		int GetPadding() const { return m_padding; }
		void SetPadding(int p) { m_padding = p; }

		int GetGap() const { return m_gap; }
		void SetGap(int g) { m_gap = g; }

	protected:
		int m_padding{}, m_gap{};
	};
}
