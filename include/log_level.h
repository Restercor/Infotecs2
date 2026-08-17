#pragma once

#include <chrono>
#include <string>

enum class LogLevel
{
	Debug = 0,
	Info = 1,
	Warning = 2,
	Critical = 3
};

struct LogMessage
{
	std::string message;
	LogLevel level;
	std::chrono::system_clock::time_point time;
};

std::string toString(LogLevel level);

bool fromString(const std::string& str, LogLevel& result);

LogMessage createLogMessage(const std::string& msg, LogLevel level);