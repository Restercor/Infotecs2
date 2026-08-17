#include <cassert>
#include <filesystem>
//#include <chrono>
#include <fstream>
#include <iostream>
//#include <thread>
#include <sstream>

#include "logger.h"
#include "async_logger.h"

namespace fs = std::filesystem;

constexpr const char* TEST_FILE = "test.log"; // using string literal

void removeTestFile()
{
    if(fs::exists(TEST_FILE))
    {
        fs::remove(TEST_FILE);
    }
} // cleaner function


void testLoggerCreation()
{
    removeTestFile();

    FileLogger logger(TEST_FILE, LogLevel::Info);

    assert(fs::exists(TEST_FILE));

    std::cout << "Logger creation successful\n";
}

void testLogFilter()
{
    removeTestFile();

    {
        FileLogger logger(TEST_FILE, LogLevel::Warning);
        logger.log("Info message", LogLevel::Info);
        logger.log("Warning message", LogLevel::Warning);

    } // Logger destroyed, file closed
    
    std::ifstream file(TEST_FILE);

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    assert(content.find("Warning message") != std::string::npos);
    assert(content.find("Info message") == std::string::npos);

    std::cout << "Log filtering successful\n";
}

void testSetLogLevel()
{
    removeTestFile();

    {
        FileLogger logger(TEST_FILE, LogLevel::Critical);

        logger.setLogLevel(LogLevel::Info);

        logger.log("Info message", LogLevel::Info);
    }  // Logger destroyed, file closed
    
    std::ifstream file(TEST_FILE);

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    assert(content.find("Info message") != std::string::npos);

    std::cout << "Logger setLogLevel successful\n";
}

void testAsyncLogger()
{
    removeTestFile();

    {
        FileLogger logger(TEST_FILE, LogLevel::Info);

        AsyncLogger asyncLogger(logger);

        for (int i = 0; i < 100; i++)
        {
            asyncLogger.enqueue("Message " + std::to_string(i), LogLevel::Info);
        }

    } // AsyncLogger and Logger destroyed, file closed

    std::ifstream file(TEST_FILE);

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    for(int i=0; i < 100; ++i)
    {
        assert(content.find("Message " + std::to_string(i)) != std::string::npos);
    }

    std::cout << "AsyncLogger successful\n";
}

int main()
{
    testLoggerCreation();

    testLogFilter();

    testSetLogLevel();

    testAsyncLogger();

    std::cout << "\nAll tests PASSED\n";

    removeTestFile();
}