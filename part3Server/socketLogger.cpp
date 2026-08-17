#include "../include/logger.h"
#include <iostream>

int main()
{
    SocketLogger logger("127.0.0.1", 5001, LogLevel::Info);

    logger.log("This is a debug message", LogLevel::Debug);
    logger.log("This is an info message", LogLevel::Info);
    logger.log("This is a warning message", LogLevel::Warning);
    logger.log("This is a critical message", LogLevel::Critical);

    std::cin.get();
}

//g++ part3Server/socketLogger.cpp -Iinclude src/logger.cpp src/log_level.cpp -o sender