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
		inline std::string format(const std::string &fmt, Args &&...args) {
			return formatHelper(fmt, 1, std::forward<Args>(args)...);
		}

		inline std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
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

		float normalized(float value);
		float remap(Range other, float value);
		float constrain(float value);
	};

	class Application;
	class Element;
	class Layout {
	public:
		Layout();

		enum LayoutDirection {
			Center = 0,
			Top,
			Bottom,
			Left,
			Right,
			LayoutDirectionCount,
			None
		};

		void set(Element *element, LayoutDirection dir);
		void perform(int x, int y, int w, int h);

		int padding() const { return m_padding; }
		void padding(int p) { m_padding = p; }

		int gap() const { return m_gap; }
		void gap(int p) { m_gap = p; }

		std::array<Element*, LayoutDirectionCount>& elements() { return m_elements; }

	private:
		std::array<Element*, LayoutDirectionCount> m_elements;
		int m_padding{}, m_gap{};

		Size calcElementSize(Element *element);
	};

	class Element : public Subscriber {
		friend class Application;
		friend class Panel;
	public:
		Element();
		~Element() = default;

		virtual void onDraw(Graphics& g);
		virtual EventStatus onEvent(Event *event) override;

		Rect bounds() const;
		Rect intersectedBounds();

		Rect& localBounds() { return m_bounds; }
		void localBounds(Rect b) { m_bounds = b; invalidate(); }

		Element* parent() { return m_parent; }

		void invalidate();

		bool visible() const { return m_visible; }
		void visible(bool v) { m_visible = v; }

		bool focused() const { return m_focused; }
		void requestFocus();

		Application* app() { return m_application; }

	private:
		Application *m_application;
		Element *m_parent;
		Rect m_bounds;
		bool m_dirty, m_visible, m_focused;

		virtual bool dirty() { return m_dirty; }
	};
}

#endif // TUI_ELEMENT_H
