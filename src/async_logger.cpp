#include "async_logger.h"

AsyncLogger::AsyncLogger(ILogger& logger)
	: m_logger(logger)
{
	m_worker = std::thread(&AsyncLogger::worker_function, this);
}

AsyncLogger::~AsyncLogger()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stop = true;
	}

	m_cv.notify_one();

	if (m_worker.joinable())
	{
		m_worker.join();
	}
}

//
// Places new message in queue
//
void AsyncLogger::enqueue(const std::string& msg, LogLevel level)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		m_queue.push(createLogMessage(msg, level));
	}

	m_cv.notify_one();
}

//
//  Main thread function
//
void AsyncLogger::worker_function()
{
	while (true)
	{
		LogMessage msg;

		{
			std::unique_lock<std::mutex> lock(m_mutex);

			m_cv.wait(lock,
				[this]
				{
					return m_stop || !m_queue.empty();
				});

			if (m_stop && m_queue.empty())
			{
				break;
			}

			msg = std::move(m_queue.front());
			m_queue.pop();
		} // lock lifted

		m_logger.log(msg.message, msg.level);
	}
}