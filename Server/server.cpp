#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <thread>
#include <ws2tcpip.h>
#include "clientManager.hpp"

// COMPILE COMMAND g++ -g *.cpp -o server.exe -lws2_32

// Name: XXXXXXXXXXXXXX
// Date: 2/26/2026
// Description: This is the server for a simple chat room application. It listens for incoming connections from clients and handles commands sent by the clients.
// The server maintains a list of connected clients and their login status.
// It processes commands such as login, newuser, send, and logout, and sends appropriate responses back to the clients.
// It uses a ClientManager class to manage client connections and user accounts. The server runs indefinitely, accepting new messages and processing their commands.
// Accounts are stored in a text file called "accounts.txt" and are loaded into memory when the server starts. The server also handles client disconnections gracefully.

#define SERVER_PORT 15377
#define MAX_PENDING 5
#define MAX_LINE 256
#define MAX_CLIENTS 3



void listenForClients(SOCKET listenSocket);
void listenForMessages(SOCKET s);

void spam(SOCKET s)
{
    ClientManager *clientManager = ClientManager::getInstance();
    std::string message = "This is a spam message from the server.";
    while (true)
    {
        Sleep(5000); // Sleep for 5 seconds before sending the next message
        std::cout << message << std::endl;
        clientManager->sendToClient(s, message);
    }
}

bool handleCmd(std::istringstream &cmd, SOCKET s);

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

    // Create a socket.
    SOCKET listenSocket;
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cout << "Error at socket(): " << WSAGetLastError();
        WSACleanup();
        return 1;
    }

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

    // listening
    if (listen(listenSocket, MAX_PENDING) == SOCKET_ERROR)
    {
        printf("Error listening on socket.\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    ClientManager *clientManager = ClientManager::getInstance();
    SOCKET s;
    std::cout << "My chat room server. Version One." << std::endl;
    std::thread clientListenerThread(listenForClients, listenSocket);
    clientListenerThread.detach();

    while(1){
        Sleep(1000); // Sleep for a while to reduce CPU usage
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}

// This function takes a command from the client and processes it. It returns true if the command was handled, and false if the command was not recognized.
bool handleCmd(std::istringstream &cmd, SOCKET s)
{
    std::string firstToken;
    cmd >> firstToken;
    ClientManager *clientManager = ClientManager::getInstance();
    if (firstToken == "login")
    {
        std::string username, password;
        cmd >> username >> password;
        // std::cout << "Login command received. Username: " << username << ", Password: " << password << std::endl;

        return clientManager->clientLogin(s, username, password); // Command handled
    }
    else if (firstToken == "newuser")
    {
        std::string username, password;
        cmd >> username >> password;
        // std::cout << "New user command received. Username: " << username << ", Password: " << password << std::endl;

        return clientManager->createUser(s, username, password); // Command handled
    }
    else if (firstToken == "send")
    {
        std::string message;
        std::getline(cmd, message);
        // std::cout << "Send command received. Message: " << message << std::endl;
        return clientManager->sendTextMessage(s, message); // Command handled
    }
    else if (firstToken == "logout")
    {
        // std::cout << "Logout command received." << std::endl;
        return clientManager->userLogout(s); // Command handled
    }
    return false; // Command not handled
}

void listenForClients(SOCKET listenSocket)
{
    while (true)
    {
        SOCKET s = accept(listenSocket, NULL, NULL);
        if (s == SOCKET_ERROR)
        {
            std::cout << "accept() error";
            closesocket(listenSocket);
            WSACleanup();
            exit(1);    
        }
        ClientManager::getInstance()->addClient(s);
        std::thread messageListenerThread(listenForMessages, s);
        messageListenerThread.detach();
    }
}

void listenForMessages(SOCKET s)
{
    char buf[MAX_LINE];
    while (true)
    {
        int len = recv(s, buf, MAX_LINE, 0);
        if (len == 0)
        {
            ClientManager::getInstance()->removeClient(s);
            closesocket(s);
            break;
        }
        else if (len == SOCKET_ERROR)
        {
            std::cout << "recv failed: " << WSAGetLastError() << std::endl;
            ClientManager::getInstance()->removeClient(s);
            closesocket(s);
            break;
        }
        buf[len] = 0;
        std::istringstream iss(buf);
        handleCmd(iss, s);
        memset(buf, 0, MAX_LINE); // Clear the buffer for the next input
    }
}