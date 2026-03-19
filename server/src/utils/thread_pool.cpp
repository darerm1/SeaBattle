#include "utils/thread_pool.hpp"

ThreadPool::ThreadPool(size_t num_threads) 
                    : work_guard_(boost::asio::make_work_guard(io_context_)) {
    for (size_t i = 0; i < num_threads; ++i) {
        threads_.emplace_back([this] {
            io_context_.run();
        });
    }
}

ThreadPool::~ThreadPool() {
    work_guard_.reset();
    io_context_.stop();
    for (auto& thread : threads_) {
        if (thread.joinable())
            thread.join();
    }
}

boost::asio::io_context::executor_type ThreadPool::get_executor() {
    return io_context_.get_executor();
}
