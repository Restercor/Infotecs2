#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "logger.h"

class AsyncLogger
{
public:
	explicit AsyncLogger(ILogger& logger);
	~AsyncLogger();

	AsyncLogger(const AsyncLogger&) = delete;
	AsyncLogger& operator=(const AsyncLogger&) = delete;

	void enqueue(const std::string& message, LogLevel level);

private:
	void worker_function();

private:
	ILogger& m_logger;

	std::queue<LogMessage> m_queue;

	std::mutex m_mutex;
	std::condition_variable m_cv;

	bool m_stop = false;

	std::thread m_worker;
};