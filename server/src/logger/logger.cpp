#include "logger/logger.hpp"

bool Logger::verbose_ = false;
std::mutex Logger::mutex_;
