#include <iostream>
#include "winsock2.h"
#include <ws2tcpip.h>

#define SERVER_PORT 15377
#define MAX_LINE 256

int main(int argc, char **argv)
{
    std::cout << "argc = " << argc << std::endl;
    if (argc < 2)
    {
        std::cout << "\nUseage: client serverName\n";
        return 1;
    }

    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        std::cout << "Error at WSAStartup()";
        return 1;
    }
    std::cout << "Winsock initialized." << std::endl;

    // translate the server name or IP address (128.90.54.1) to resolved IP address
    unsigned int ipaddr;
    // If the user input is an alpha name for the host, use gethostbyname()
    // If not, get host by addr (assume IPv4)
    std::cout << "argv[1] = " << argv[1] << std::endl;
    if (isalpha(argv[1][0]))
    { // host address is a name
        hostent *remoteHost = gethostbyname(argv[1]);
        if (remoteHost == NULL)
        {
            std::cout << "Host not found";
            WSACleanup();
            return 1;
        }
        ipaddr = *((unsigned long *)remoteHost->h_addr);
    }
    else //"128.90.54.1"
        {ipaddr = inet_addr(argv[1]);}

    // Create a socket.
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Connect to a server.
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ipaddr;
    addr.sin_port = htons(SERVER_PORT);
    if (connect(s, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        printf("Failed to connect.\n");
        WSACleanup();
        return 1;
    }
    std::cout << "Connected to server." << std::endl;
    ipaddr = inet_addr(argv[1]);
    




    return 0;
}