#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#define SERVER_PORT 15377
#define MAX_PENDING 5
#define MAX_LINE 256

int main()
{
    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        std::cout << "Error at WSAStartup()\n";
        return 1;
    }
    std::cout << "Winsock initialized.\n";

    // Create a socket.
    SOCKET listenSocket;
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cout << "Error at socket(): " << WSAGetLastError();
        WSACleanup();
        return 1;
    }
    std::cout << "Listening socket created.\n";

    // Bind the socket.
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // use local address
    addr.sin_port = htons(SERVER_PORT);
    if (bind(listenSocket, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cout << "bind() failed.";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Socket binded.\n";

    if (listen(listenSocket, MAX_PENDING) == SOCKET_ERROR)
    {
        printf("Error listening on socket.\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Listening on socket...\n";



    SOCKET s;
    std::cout << "Waiting for a client to connect..." << std::endl;
    while (1)
    {
        s = accept(listenSocket, NULL, NULL);
        if (s == SOCKET_ERROR)
        {
            std::cout << "accept() error";
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Client Connected." << std::endl;

        // Send and receive data.
        char buf[MAX_LINE];
        int len = recv(s, buf, MAX_LINE, 0);
        buf[len] = 0;
        send(s, buf, strlen(buf), 0);
        closesocket(s);
        std::cout << "Client Closed." << std::endl;
    }



    closesocket(listenSocket);
    WSACleanup();
    return 0;
}