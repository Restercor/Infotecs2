#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#include "logger.h"

FileLogger::FileLogger(const std::string& filename, LogLevel defaultLevel)
{
	try
	{
		m_file.open(filename, std::ios::app);
	}
	catch (const std::exception& e)
	{
		throw e.what();
	}

	m_defaultLevel = defaultLevel;
}

void FileLogger::log(const std::string& msg, LogLevel level)
{
	if (level < m_defaultLevel)
	{
		return;
	}
	
	if (!m_file.is_open())
	{
		throw std::runtime_error("Log file is not open");
	}

	//auto timestamp = getCurrentTime();

	const LogMessage message = createLogMessage(msg, level);

	m_file << formatLogMessage(message) << '\n';
	m_file.flush(); // in case of emergency

	if (!m_file)
	{
		throw std::runtime_error("Failed to write to log file!");
	}
}

SocketLogger::SocketLogger(const std::string& address, int port, LogLevel defaultLevel) : m_socket(-1), m_defaultLevel(defaultLevel)
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_socket == -1)
	{
		throw std::runtime_error("Failed to create socket");
	}

	sockaddr_in serverAddress{};

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);

	const int result = inet_pton(AF_INET, address.c_str(), &serverAddress.sin_addr);

	if (result == 0)
	{
		close(m_socket);
		m_socket = -1;
		throw std::invalid_argument("Invalid IP address format: " + address);
	}

	if (result == -1)
	{
		close(m_socket);
		m_socket = -1;
		throw std::runtime_error("Failed to convert IP address: " + std::string(strerror(errno)));
	}

	if (connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1)
	{
		const std::string error = strerror(errno);

		close(m_socket);
		m_socket = -1;
		throw std::runtime_error("Failed to connect to server: " + address + ":" + std::to_string(port) + ":" + error);
	}

	if (m_socket < 0)
	{
		throw std::invalid_argument("Invalid socket file descriptor");
	}
}

SocketLogger::~SocketLogger()
{
	if (m_socket != -1)
	{
		close(m_socket);
	}
}

void SocketLogger::log(const std::string& msg, LogLevel level)
{
	if (level < m_defaultLevel)
	{
		return;
	}

	const LogMessage message = createLogMessage(msg, level);

	const std::string data = serialize(message);
	sendAll(data);
}

std::string SocketLogger::serialize(const LogMessage& msg)
{
	return formatTime(msg.time) + '|' + toString(msg.level) + '|' + msg.message + '\n';
}

void SocketLogger::sendAll(const std::string& data)
{
	std::size_t totalSent = 0;

	while (totalSent < data.size())
	{
		const ssize_t sent = send(m_socket, data.data() + totalSent, data.size() - totalSent, 0);

		if (sent == -1)
		{
			throw std::runtime_error("Failed to send data: " + std::string(strerror(errno)));
		}

		if(sent == 0)
		{
			throw std::runtime_error("Connection closed by the server");
		}

		totalSent += static_cast<std::size_t>(sent);
	}
}

std::string formatLogMessage(const LogMessage& msg)
{
	std::ostringstream stream;
	
	stream << formatTime(msg.time) << " [" << toString(msg.level) << "] " << msg.message;
	
	return stream.str();
}

std::string formatTime(const std::chrono::system_clock::time_point& time)
{
	const std::time_t timeT = std::chrono::system_clock::to_time_t(time);

	std::tm tm{};

	localtime_r(&timeT, &tm);

	std::ostringstream stream;

	stream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

	return stream.str();
}
