#ifndef TUI_ELEMENT_H
#define TUI_ELEMENT_H

#include <optional>
#include <array>
#include <regex>
#include <sstream>

#include "EventSystem.h"
#include "Graphics.h"

namespace tui {
	namespace utils {
		inline std::string formatHelper(
			const std::string &string_to_update,
			const size_t)
		{
		  return string_to_update;
		}

		template <typename T, typename... Args>
		inline std::string formatHelper(
			const std::string &string_to_update,
			const size_t index_to_replace,
			T &&val,
			Args &&...args)
		{
			std::regex pattern{"{" + std::to_string(index_to_replace) + "}"};
			std::string replacement_string{(std::ostringstream{} << val).str()};
			return formatHelper(
						std::regex_replace(string_to_update, pattern, replacement_string),
						index_to_replace + 1,
						std::forward<Args>(args)...
			);
		}

		template <typename... Args>
		inline std::string Format(const std::string &fmt, Args &&...args) {
			return formatHelper(fmt, 1, std::forward<Args>(args)...);
		}

		inline std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter) {
			std::vector<std::string> strings;

			std::string::size_type pos = 0;
			std::string::size_type prev = 0;
			while ((pos = str.find(delimiter, prev)) != std::string::npos) {
				strings.push_back(str.substr(prev, pos - prev));
				prev = pos + 1;
			}

			// To get the last substring (or only, if delimiter is not found)
			strings.push_back(str.substr(prev));

			return strings;
		}
	}

	struct Range {
		float minimum, maximum;

		Range() = default;
		Range(float minv, float maxv)
			: minimum(minv), maximum(maxv)
		{}

		float Normalized(float value);
		float Remap(Range other, float value);
		float Constrain(float value);
	};

	class Application;
	class Element : public Subscriber {
		friend class Application;
		friend class Panel;
		friend class ScrollView;
	public:
		Element();
		~Element() = default;

		virtual void OnDraw(Graphics& g);
		virtual EventStatus OnEvent(Event *event) override;
		virtual Size GetPreferredSize();

		Rectangle GetBounds() const;
		Rectangle GetIntersectedBounds();

		Rectangle& GetLocalBounds() { return m_bounds; }
		void SetLocalBounds(Rectangle b) { m_bounds = b; Invalidate(); }

		Element* GetParent() { return m_parent; }

		void Invalidate();

		bool IsVisible() const { return m_visible; }
		void SetVisible(bool v) { m_visible = v; Invalidate(); }

		bool IsFocused() const { return m_focused; }
		void RequestFocus();

		bool IsAutoSize() const { return m_autoSize; }
		void SetAutoSize(bool as) { m_autoSize = as; Invalidate(); }

		float GetFlexGrow() const { return m_flexGrow; }
		void SetFlexGrow(float fg) { m_flexGrow = fg; Invalidate(); }

		Application* GetApp() { return m_application; }

	private:
		Application *m_application;
		Element *m_parent;
		Rectangle m_bounds;
		bool m_dirty, m_visible, m_focused, m_autoSize;

		float m_flexGrow{0.0f};

		virtual bool IsDirty() { return m_dirty; }
	};
}

#endif // TUI_ELEMENT_H
