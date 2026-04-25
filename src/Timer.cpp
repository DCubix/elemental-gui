#include "Timer.h"

namespace gui {
    void Timer::Start(int intervalMs, std::function<void()> callback) {
        Stop(); // Stop any existing timer

        m_running = true;
        m_thread = std::thread([this, intervalMs, callback]() {
            while (m_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
                if (m_running) {
                    callback();
                }
            }
        });
    }

    void Timer::Stop() {
        m_running = false;
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
}