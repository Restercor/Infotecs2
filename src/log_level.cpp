#include <stdexcept>

#include "log_level.h"


std::string toString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Debug: return "DEBUG";
	case LogLevel::Info: return "INFO";
	case LogLevel::Warning: return "WARNING";
	case LogLevel::Critical: return "CRITICAL";
	}

	return "UNKNOWN";
}

bool fromString(const std::string& str, LogLevel& result)
{
	if (str == "DEBUG")
	{
		result = LogLevel::Debug;
		return true;
	}
	if (str == "INFO")
	{
		result =  LogLevel::Info;
		return true;
	}
	if (str == "WARNING")
	{
		result = LogLevel::Warning;
		return true;
	}
	if (str == "CRITICAL")
	{
		result = LogLevel::Critical;
		return true;
	}

	return false;
}

LogMessage createLogMessage(const std::string& msg, LogLevel level)
{
	return LogMessage{msg, level, std::chrono::system_clock::now()};
}