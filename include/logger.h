#pragma once
#include "log_level.h"
#include <fstream>

class ILogger
{
public:
	virtual ~ILogger() = default;
	virtual void log(const std::string& msg, LogLevel level) = 0;
	virtual void setLogLevel(LogLevel level) = 0;
};

class FileLogger : public ILogger
{
public:
	FileLogger(const std::string& filename, LogLevel defaultLevel);

	~FileLogger() = default;

	void log(const std::string& msg, LogLevel level) override;

	void setLogLevel(LogLevel level) { m_defaultLevel = level; };

private:
	std::ofstream m_file;
	LogLevel m_defaultLevel;

	//static std::string getCurrentTime();
};

class SocketLogger : public ILogger
{
public:
	SocketLogger(const std::string& address, int port, LogLevel defaultLevel);

	~SocketLogger();
	
	void log(const std::string& msg, LogLevel level) override;
	void setLogLevel(LogLevel level) { m_defaultLevel = level; };

private:
	void sendAll(const std::string& data);
	std::string serialize(const LogMessage& msg);

private:
	int m_socket;
	LogLevel m_defaultLevel;
};

std::string formatLogMessage(const LogMessage& msg);

std::string formatTime(const std::chrono::system_clock::time_point& time);