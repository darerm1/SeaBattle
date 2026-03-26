#pragma once

#include <boost/asio.hpp>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    template<typename F, typename Callback>
    void enqueue(F&& f, Callback&& cb) {
        boost::asio::post(io_context_, [f = std::forward<F>(f), cb = std::forward<Callback>(cb)]() {
            cb(f());
        });
    }

    template<typename F>
    void enqueue(F&& f) {
        boost::asio::post(io_context_, std::forward<F>(f));
    }

    boost::asio::io_context::executor_type get_executor();

private:
    std::vector<std::thread> threads_;
    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
};
