#pragma once

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/packaged_task.hpp>
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

    template<typename F>
    auto enqueue(F&& f) -> boost::future<decltype(f())> {
        using return_type = decltype(f());
        
        auto task = std::make_shared<boost::packaged_task<return_type>>(std::forward<F>(f));
        boost::future<return_type> result = task->get_future();
        
        boost::asio::post(io_context_, [task]() { (*task)(); });
        return result;
    }

    boost::asio::io_context::executor_type get_executor();

private:
    std::vector<std::thread> threads_;
    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
};
