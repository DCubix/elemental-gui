#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

namespace gui {
    class Timer {
    public:
        Timer()
            : m_running(false) {}
        ~Timer() { Stop(); }

        void Start(int intervalMs, std::function<void()> callback);
        void Stop();

    private:
        std::thread m_thread;
        std::atomic<bool> m_running;
    };
} // namespace gui