#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "clientManager.hpp"
#include "cmdparser.hpp"

#define SERVER_PORT 15377
#define MAX_PENDING 5
#define MAX_LINE 256
#define MAX_CLIENTS 3


bool handleCmd(std::istringstream& cmd, SOCKET s);

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

    ClientManager *clientManager = ClientManager::getInstance();
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

        clientManager->addClient(s);
        clientManager->createUser(s, "user" + std::to_string(clientManager->getClients().size()), "pass" + std::to_string(clientManager->getClients().size()));
        std::cout << "Client Connected." << std::endl;
        clientManager->printClients();

        // Send and receive data.
        char buf[MAX_LINE];

        // another loop!
        while (1)
        {
            int len = recv(s, buf, MAX_LINE, 0);
            if (len == 0)
            {
                std::cout << "Client disconnected." << std::endl;
                clientManager->removeClient(s);
                closesocket(s);
                break;
            }
            else if (len == SOCKET_ERROR)
            {
                std::cout << "recv failed: " << WSAGetLastError() << std::endl;
                break;
            }
            buf[len] = 0;
            //send(s, buf, strlen(buf), 0);
            std::cout << "Received from client: " << buf << std::endl;
            std::istringstream iss(buf);
            handleCmd(iss, s);
            memset(buf, 0, MAX_LINE); // Clear the buffer for the next input
            clientManager->printClients();
        }
        // closesocket(s);

        // clientManager->removeClient(s);
        // std::cout << "Client Closed." << std::endl;
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}


bool handleCmd(std::istringstream &cmd, SOCKET s)
{
    std::string firstToken;
    cmd >> firstToken;
    std::cout << "First token: " << firstToken << std::endl;
    if (firstToken == "login")
    {
        std::string username, password;
        cmd >> username >> password;
        std::cout << "Login command received. Username: " << username << ", Password: " << password << std::endl;
        ClientManager *clientManager = ClientManager::getInstance();
        
        return clientManager->clientLogin(s, username, password); // Command handled
    }
    return false; // Command not handled
}
