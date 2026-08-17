#include "Statistics.h"

#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>


bool parseUnsigned(const char* value, std::size_t& result)
{
    if(value == nullptr || *value == '\0')
    {
        return false; // Null or empty string
    }

    char* endPtr = nullptr;

    errno = 0; // Reset errno before the call
    const unsigned long parsedValue = std::strtoul(value, &endPtr, 10);

    if(errno == ERANGE || *endPtr != '\0')
    {
        return false;
    }

    result = static_cast<std::size_t>(parsedValue);
    return true;
}
int main(int argc, char* argv[])
{

    if (argc != 5)
    {
        std::cout << "Usage: " << argv[0] << " <IP_ADDRESS> <PORT> <N> <T>\n";
        return -1;
    }

    const std::string ipAddress = argv[1];

    std::size_t port;
    std::size_t N;
    std::size_t T;

    if(!parseUnsigned(argv[2], port) || !parseUnsigned(argv[3], N) || !parseUnsigned(argv[4], T))
    {
        std::cerr << "Invalid argument(s). PORT, N, and T must be positive integers.\n";
        return -1;
    }

    if (port == 0 || port > 65535)
    {
        std::cerr << "Invalid port number. Must be in the range 1-65535.\n";
        return -1;
    }

    if (N == 0)
    {
        std::cerr << "N must be greater than 0.\n";
        return -1;
    }

    if(T == 0)
    {
        std::cerr << "T must be greater than 0.\n";
        return -1;
    }

    const int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1)
    {
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    }

    int option = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
    {
        close(serverSocket);
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(static_cast<uint16_t>(port));

    if(inet_pton(AF_INET, ipAddress.c_str(), &serverAddress.sin_addr) != 1)
    {
        close(serverSocket);
        throw std::runtime_error("Invalid IPv4 address: " + ipAddress);
    }

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        close(serverSocket);
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
    }

    if(listen(serverSocket, 1) == -1)
    {
        close(serverSocket);
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
    }

    std::cout << "Statistics server listening on port " << port << '\n';

    const int clientSocket = accept(serverSocket, nullptr, nullptr);

    if (clientSocket == -1)
    {
        close(serverSocket);
        throw std::runtime_error("Failed to accept connection: " + std::string(strerror(errno)));
    }

    Statistics stats;

    std::string buffer;
    char receiveBuffer[1024];

    auto lastStatisticsPrintTime = std::chrono::steady_clock::now();
    const auto statsTimeout = std::chrono::seconds(T);

    while(true)
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);
        
        timeval timeout{};
        timeout.tv_sec = 1; // 1 second timeout
        timeout.tv_usec = 0;

        const int selectResult = select(clientSocket + 1, &readSet, nullptr, nullptr, &timeout);

        if (selectResult == -1)
        {
            if (errno == EINTR)
            {
                continue; // Interrupted by signal, retry
            }

            throw std::runtime_error("Select error: " + std::string(strerror(errno)));
        }
        if (selectResult == 0)
        {
            //Nothing to read, check if we need to print statistics
        }
        else if (FD_ISSET(clientSocket, &readSet))
        {
            const ssize_t received = recv(clientSocket, receiveBuffer, sizeof(receiveBuffer), 0);
            if (received == 0)
            {
                std::cout << "Client disconnected.\n";
                break;
            }

            if (received == -1)
            {
                if(errno == EINTR)
                {
                    continue; // Interrupted by signal, retry
                }
                else if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue; // No data available, retry
                }
                std::cout << "Failed to receive data: " << std::string(strerror(errno)) << '\n';
                break;
            }

            buffer.append(receiveBuffer, static_cast<std::size_t>(received));

            std::size_t pos;
            while ((pos = buffer.find('\n')) != std::string::npos)
            {   
                std::string message = buffer.substr(0, pos);
                buffer.erase(0, pos + 1);

                std::cout << "Received message: " << message << '\n';

                LogMessage logMsg;

                if(!parseLogMessage(message, logMsg))
                {
                    std::cerr << "Error parsing log message: " << message << '\n';
                    continue;
                }

                stats.add(logMsg.level, logMsg.message.length());

                if(stats.getMessageCount() >= N)
                {
                    stats.print();
                    stats.resetChanged();
                    lastStatisticsPrintTime = std::chrono::steady_clock::now();
                }
            }
        }

        const auto now = std::chrono::steady_clock::now();

        if (now - lastStatisticsPrintTime >= statsTimeout && stats.changed())
        {
            stats.print();
            stats.resetChanged();
            lastStatisticsPrintTime = now;
        }
    }

        
    close(clientSocket);
    close(serverSocket);

    stats.print();

    return 0;
}