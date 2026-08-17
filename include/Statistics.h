#include "log_level.h"
#include <iostream>
#include <iomanip>

struct Statistics {

    void add(LogLevel level, std::size_t messageLength = 0) {
        switch (level) {
            case LogLevel::Debug:
                ++debug;
                break;
            case LogLevel::Info:
                ++info;
                break;
            case LogLevel::Warning:
                ++warning;
                break;
            case LogLevel::Critical:
                ++critical;
                break;
        }

        ++m_messageCount;

        if(m_messageCount == 1)
        {
            m_minMessageLength = messageLength;
            m_maxMessageLength = messageLength;
        }
        else
        {
            m_minMessageLength = std::min(m_minMessageLength, messageLength);
            m_maxMessageLength = std::max(m_maxMessageLength, messageLength);
        }
        
        m_totalMessageLength += messageLength;

        m_changed = true;
    }

    void print() const {
        std::cout << "\n-------------Statistics-------------\n"
                  << "Debug: " << debug << "\n"
                  << "Info: " << info << "\n"
                  << "Warning: " << warning << "\n"
                  << "Critical: " << critical << "\n"
                  << "Total messages: " << m_messageCount << "\n\n"
                  << "Message length statistics:\n"
                  << "Minimum message length: " << m_minMessageLength << "\n"
                  << "Maximum message length: " << m_maxMessageLength << "\n"
                  << "Average message length: " << averageMessageLength() << "\n"
                  << "------------------------------------\n";
    }

    double averageMessageLength() const
    {
        if (m_messageCount == 0)
        {
            return 0.0;
        }
        return static_cast<double>(m_totalMessageLength) / m_messageCount;
    }

    bool changed() const
    {
        return m_changed;
    }

    void resetChanged()
    {
        m_changed = false;
    }

    std::size_t getMessageCount() const
    {
        return m_messageCount;
    }

    private:
        std::size_t m_messageCount = 0;

        std::size_t debug = 0;
        std::size_t info = 0;
        std::size_t warning = 0;
        std::size_t critical = 0;

        std::size_t m_minMessageLength = 0;
        std::size_t m_maxMessageLength = 0;
        std::size_t m_totalMessageLength = 0;

        bool m_changed = false;
};


bool parseLogMessage(const std::string& msg, LogMessage& result)
{
    const auto firstSeparator = msg.find('|');

    if (firstSeparator == std::string::npos)
    {
        return false; // Invalid format
    }

    const auto secondSeparator = msg.find('|', firstSeparator + 1);

    if (secondSeparator == std::string::npos)
    {
        return false; // Invalid format
    }

    const std::string time = msg.substr(0, firstSeparator);
    const std::string level = msg.substr(firstSeparator + 1, secondSeparator - firstSeparator - 1);
    const std::string text = msg.substr(secondSeparator + 1);

    LogLevel logLevel;

    if (level == "DEBUG")
    {
        logLevel = LogLevel::Debug;
    }
    else if (level == "INFO")
    {
        logLevel = LogLevel::Info;
    }
    else if (level == "WARNING")
    {
        logLevel = LogLevel::Warning;
    }
    else if (level == "CRITICAL")
    {
        logLevel = LogLevel::Critical;
    }
    else
    {
        return false; // Invalid log level
    }

    std::tm tm{};
    std::istringstream stream(time);
    stream >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (stream.fail())
    {
        return false; // Invalid time format
    }

    const std::time_t timeT = std::mktime(&tm);

    result = LogMessage{text, logLevel, std::chrono::system_clock::from_time_t(timeT)};
    return true;
}