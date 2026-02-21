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

void waitForServerResponse(SOCKET s);

int main(int argc, char **argv)
{

    // this code causes warnings, but it is necessary for testing, we will connect to the local server
    argc = 3;
    argv[1] = "127.0.0.1";
    argv[2] = "127.0.0.1";


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
    {
        ipaddr = inet_addr(argv[1]);
    }

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
        std::cout << "Type whatever you want: ";
        fgets(buf, sizeof(buf), stdin);
        std::istringstream iss(buf);
        handleCmd(iss, s);

        //send(s, buf, strlen(buf), 0);
        // int len = recv(s, buf, MAX_LINE, 0);
        // if (len > 0)
        // {
        //     if (len >= MAX_LINE)
        //         len = MAX_LINE - 1; // prevent overflow
        //     buf[len] = '\0';
        //     std::cout << "Server says: " << buf;
        // }
        // else if (len == 0)
        // {
        //     std::cout << "Server closed connection." << std::endl;
        //     break;
        // }
        // else
        // {
        //     std::cout << "recv failed: " << WSAGetLastError() << std::endl;
        //     break;
        // }
        // memset(buf, 0, MAX_LINE); // Clear the buffer for the next input
        // std::cout << "Server says: " << buf << std::endl;
    }

    fflush(stdout);
    std::cout << "Closing socket." << std::endl;
    closesocket(s);
    return 0;
}

void handleCmd(std::istringstream& cmd, SOCKET s){
    std::string firstToken;
    cmd >> firstToken;
    std::cout << "First token!!!!: " << firstToken << std::endl;
    if(firstToken == "exit"){
        // If the user types "exit", we want to exit the client program
        std::cout << "Exiting client." << std::endl;
        closesocket(s);
        WSACleanup();
        exit(0);
    } else if(firstToken == "login"){
        // If the user types "login username password", we want to send a login command to the server
        login(s, cmd);
    } else if(firstToken == "newuser"){
        // If the user types "newuser username password", we want to send a newuser command to the server
        newUser(s, cmd);
    } else {
        std::cout << "Unknown command. Available commands: login, newuser, exit" << std::endl;
    }
    return;
}



void login(SOCKET s, std::istringstream& cmd){
    std::string username, password;
    cmd >> username >> password;
    std::string loginCmd = "login " + username + " " + password;
    std::cout << "Login command: " << loginCmd << std::endl;
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
    std::cout << "New user command: " << newUserCmd << std::endl;
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