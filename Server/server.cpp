#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <thread>
#include <atomic>
#include <ws2tcpip.h>
#include "clientManager.hpp"

// COMPILE COMMAND g++ -g *.cpp -o server.exe -lws2_32

// Name: XXXXXXXXXXXXXX
// Date: 2/26/2026
// Description: This is the server for a simple chat room application. It listens for incoming connections from clients and handles commands sent by the clients.
// The server maintains a list of connected clients and their login status.
// It processes commands such as login, newuser, send, and logout, and sends appropriate responses back to the clients.
// It uses a ClientManager class to manage client connections and user accounts. The server runs indefinitely, accepting new client connections and messages while processing the commands.
// Accounts are stored in a text file called "accounts.txt" and are loaded into memory when the server starts. The server also handles client disconnections gracefully.

#define SERVER_PORT 15377
#define MAX_PENDING 5
#define MAX_LINE 256
#define MAX_CLIENTS 3

std::atomic<int> currentConnections(0);
MessageQueue messageQueue;

void listenForClients(SOCKET listenSocket);
void listenForMessages(SOCKET s);
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
    std::cout << "My chat room server. Version One." << std::endl;

    // Start a thread to listen for incoming client connections
    std::thread clientListenerThread(listenForClients, listenSocket);
    clientListenerThread.detach();

    // Main thread will continuously check the message queue for new messages from clients and send them to the handleCmd function for processing.
    // This allows us to process client commands in the main thread while the client listener thread handles incoming connections and message listener threads handle incoming messages from clients.
    while(1){
        Sleep(100); // Sleep for a while to reduce CPU usage
        Message msg = Message(0, ""); // Initialize with default values
        if(messageQueue.pop(msg)){
            std::istringstream iss(msg.text);
            handleCmd(iss, msg.sender);
        }
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
        return clientManager->clientLogin(s, username, password); // Command handled
    }
    else if (firstToken == "newuser")
    {
        std::string username, password;
        cmd >> username >> password;
        return clientManager->createUser(s, username, password); // Command handled
    }
    else if (firstToken == "send")
    {
        std::string message, recipient;
        cmd >> recipient;
        std::getline(cmd, message);
        return clientManager->sendTextMessage(s, message, recipient); // Command handled
    }
    else if (firstToken == "logout")
    {
        return clientManager->userLogout(s); // Command handled
    }
    else if(firstToken == "who")
    {
        clientManager->listOnlineUsers(s);
        return true; // Command handled
    }
    return false; // Command not handled
}

// Function to listen for incoming client connections and start a new thread to listen for messages from each connected client
// If the server is at max capacity, it sends a message to the client and closes the connection.
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
        if(currentConnections.load() >= MAX_CLIENTS){
            std::string message = "Server is full. Try again later.";
            send(s, message.c_str(), message.size(), 0);
            closesocket(s);
            continue;
        }
        ClientManager::getInstance()->addClient(s);
        currentConnections++;

        // Start a thread to listen for messages from this newly connected client
        std::thread messageListenerThread(listenForMessages, s);
        messageListenerThread.detach();
    }
}


// This function listens for messages from a specific client socket.
// When a message is received, it is added to the message queue for processing by the main thread. 
//If the client disconnects, the function removes the client from the ClientManager and closes the socket.
void listenForMessages(SOCKET s)
{
    char buf[MAX_LINE];
    while (true)
    {
        int len = recv(s, buf, MAX_LINE, 0);
        if (len == 0)
        {
            break; // Connection closed by client
        }
        else if (len == SOCKET_ERROR)
        {
            std::cout << "recv failed: " << WSAGetLastError() << std::endl;
            break; // Error occurred
        }
        buf[len] = 0;
        std::istringstream iss(buf);
        messageQueue.push(Message(s, iss.str()));
        memset(buf, 0, MAX_LINE); // Clear the buffer for the next input
    }
    ClientManager::getInstance()->removeClient(s);
    currentConnections--;
    closesocket(s);
    return;
}



