#include <iostream>
#include "winsock2.h"
#include <ws2tcpip.h>
#include <cctype>
#include <cstring>
#include <sstream>

#define SERVER_PORT 15377
#define MAX_LINE 256

void handleCmd(std::istringstream& cmd, SOCKET s);

void login(SOCKET s, std::istringstream& cmd);
void newUser(SOCKET s, std::istringstream& cmd);
void sendTextMessage(SOCKET s, std::istringstream& cmd);
void removeLeadingWhitespace(std::string& str);

void waitForServerResponse(SOCKET s);

int main(int argc, char **argv)
{
    std::string localAddress = "127.0.0.1";

    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        std::cout << "Error at WSAStartup()";
        return 1;
    }
    
    unsigned int ipaddr;
    ipaddr = inet_addr(localAddress.c_str());
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

    // Send and receive data.
    char buf[MAX_LINE];
    while (true)
    {
        // get user input
        fgets(buf, sizeof(buf), stdin);
        std::istringstream iss(buf);
        handleCmd(iss, s);
    }

    fflush(stdout);
    std::cout << "Closing socket." << std::endl;
    closesocket(s);
    return 0;
}

void handleCmd(std::istringstream& cmd, SOCKET s){
    std::string firstToken;
    cmd >> firstToken;
    if(firstToken == "exit"){
        std::cout << "Exiting client." << std::endl;
        closesocket(s);
        WSACleanup();
        exit(0);
    } else if(firstToken == "login"){
        login(s, cmd);
    } else if(firstToken == "newuser"){
        newUser(s, cmd);
    } else if(firstToken == "send"){
        sendTextMessage(s, cmd);
    } else {
        std::cout << "Unknown command. Available commands: login, newuser, send, exit" << std::endl;
    }
    return;
}



void login(SOCKET s, std::istringstream& cmd){
    std::string username, password;
    cmd >> username >> password;
    std::string loginCmd = "login " + username + " " + password;
    if(username.empty() || password.empty()){
        std::cout << "Invalid login command. Usage: login username password" << std::endl;
        return;
    }
    if(username.length() < 3 || username.length() > 32 || password.length() < 4 || password.length() > 8){
        std::cout << "Username must be between 3 and 32 characters long, and password must be between 4 and 8 characters long." << std::endl;
        return;
    }
    send(s, loginCmd.c_str(), loginCmd.size(), 0);
    waitForServerResponse(s);
    return;
}

void newUser(SOCKET s, std::istringstream& cmd){
    std::string username, password;
    cmd >> username >> password;
    std::string newUserCmd = "newuser " + username + " " + password;
    if(username.empty() || password.empty()){
        std::cout << "Invalid newuser command. Usage: newuser username password" << std::endl;
        return;
    }
    if(username.length() < 3 || username.length() > 32 || password.length() < 4 || password.length() > 8){
        std::cout << "Username must be between 3 and 32 characters long, and password must be between 4 and 8 characters long." << std::endl;
        return;
    }
    send(s, newUserCmd.c_str(), newUserCmd.size(), 0);
    waitForServerResponse(s);
    return;
}

void sendTextMessage(SOCKET s, std::istringstream& cmd){
    
    
    std::string message;
    std::getline(cmd, message);
    removeLeadingWhitespace(message);
    if(message.length() > 256 || message.empty()){
        std::cout << "Message must be between 1 and 256 characters long." << std::endl;
        return;
    }
    // Remove leading whitespace from the message
    message.erase(0, message.find_first_not_of(" \t"));
    std::string sendCmd = "send " + message;
    send(s, sendCmd.c_str(), sendCmd.size(), 0);
    waitForServerResponse(s);
    return;
}   

void removeLeadingWhitespace(std::string& str) {
    size_t firstNonWhitespace = str.find_first_not_of(" \t");
    if (firstNonWhitespace != std::string::npos) {
        str.erase(0, firstNonWhitespace);
    } else {
        str.clear(); // String is all whitespace
    }
}

void waitForServerResponse(SOCKET s){
    char buf[MAX_LINE];
    int len = recv(s, buf, MAX_LINE, 0);
    if (len > 0)
    {
        if (len >= MAX_LINE)
            len = MAX_LINE - 1; // prevent overflow
        buf[len] = '\0';
        std::cout << buf << std::endl;
    }
    else if (len == 0)
    {
        std::cout << "Server closed connection." << std::endl;
    }
    else
    {
        std::cout << "recv failed: " << WSAGetLastError() << std::endl;
    }
    memset(buf, 0, MAX_LINE); // Clear the buffer for the next input
}