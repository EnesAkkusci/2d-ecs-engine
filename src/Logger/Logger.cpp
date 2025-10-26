#include <cassert>
#include <iostream>
#include "./Logger.h"

void Logger::Confirm(const std::string_view& message) {
    std::cout << "\x1B[32m" << message << "\033[0m" << std::endl;
}

void Logger::Err(const std::string_view& message) {
    std::cerr << "\x1B[91m"<< message << "\033[0m" << std::endl;
}

void Logger::Log(const std::string_view& message) {
    std::cout << "\033[0m" << message << "\033[0m" << std::endl;
}
