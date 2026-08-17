#include <iostream>

#include "async_logger.h"
#include "logger.h"

//
// Parses input string into LogMessage compatible pair
//
std::pair<std::string, LogLevel> parseInput(const std::string& input, LogLevel defaultLevel)
{
    auto pos = input.find(' ');

    if (pos == std::string::npos)
    {
        return { input, defaultLevel };
    }

    std::string first = input.substr(0, pos);
    LogLevel level;
    if(!fromString(first, level))
    {
        return { input, defaultLevel };
    }

    return { input.substr(pos + 1), level };
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <log_file> <log_level>\n";

        return EXIT_FAILURE;
    }

    LogLevel defaultLevel;
    if(!fromString(argv[2], defaultLevel))
    {
        std::cerr << "Invalid log level: " << argv[2] << "\n";
        return EXIT_FAILURE;
    }

    try
    {
        // Logger init
        FileLogger logger(argv[1], defaultLevel);
        AsyncLogger asyncLogger(logger);

        std::cout << "Enter log messages.\n";
        std::cout << "Examples:\n";
        std::cout << "INFO Server started\n";
        std::cout << "WARNING Low memory\n";
        std::cout << "Just a message\n";
        std::cout << "Type EXIT to quit.\n\n";

        std::string input;

        while (std::getline(std::cin, input))
        {
            if (input == "EXIT")
            {
                break;
            }
            if (input.empty())
            {
                continue;
            }
               
            auto msg = parseInput(input, defaultLevel);

            asyncLogger.enqueue(msg.first,msg.second);
        } // main loop
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    } // Logger and AsyncLogger destroyed, file closed.
}