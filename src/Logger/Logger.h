#pragma once
#include <string_view>

class Logger{
public:
	static void Confirm(const std::string_view& message);
	static void Log(const std::string_view& message);
	static void Err(const std::string_view& message);
};
