#pragma once
#include <iostream>
#include <mutex>

class Logger {
public:
    static void set_verbose(bool verbose) { verbose_ = verbose; }

    template<typename... Args>
    static void log(Args&&... args) {
        if (verbose_) {
            std::lock_guard<std::mutex> lock(mutex_);
            (std::cout << ... << std::forward<Args>(args)) << std::endl;
        }
    }

private:
    static bool verbose_;
    static std::mutex mutex_;
};
