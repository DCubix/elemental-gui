#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include <vector>

#include "EventSystem.h"
#include "Graphics.h"

namespace tui {
	class Application;
	class ApplicationAdapter {
	public:
		virtual ~ApplicationAdapter() = default;
		virtual void OnCreate(Application& app) = 0;
		virtual void OnDestroy() = 0;
	};
	
	class Window;
	struct WindowConfig;
	using WindowId = uint64_t;
	
	class Application {
	public:
		Application() = default;
		~Application() = default;

		int Start(ApplicationAdapter *adapter);

		Window* CreateWindow(const WindowConfig& config);

		void SetClipboard(const std::string& str);
		std::string GetClipboard();

		EventSystem& GetEventSystem() { return m_eventSystem; }

		static Json DefaultStyle;
	private:
		EventSystem m_eventSystem;
		std::vector<std::unique_ptr<Window>> m_windows;

		Window* FindWindow(WindowId id);
	};
}
