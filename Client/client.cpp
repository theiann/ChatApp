#include <iostream>
#include "winsock2.h"
#include <ws2tcpip.h>
#include <cctype>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <thread>
#include <limits>

#include <mutex>
#include <atomic>


#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

std::mutex consoleMutex;
std::string currentInput;

// COMPILE COMMAND: g++ -g *.cpp isocline/src/isocline.c -o client.exe -lws2_32

// Name: XXXXXXXXXXXXXX
// Date: 2/26/2026
// Description: This is the client for a simple chat room application. It connects to a server
// and allows the user to login, create a new user, send messages, and logout. The client sends commands to the server and waits for responses.
//s
// Commands:
// login username password - Logs in with the specified username and password.
// newuser username password - Creates a new user with the specified username and password.
// send message - Sends a message to the chat room.
// logout - Logs out of the chat room.
// The client also handles invalid commands and displays appropriate error messages.



#define SERVER_PORT 15377
#define MAX_LINE 256

void handleCmd(std::istringstream& cmd, SOCKET s);
void login(SOCKET s, std::istringstream& cmd);
void newUser(SOCKET s, std::istringstream& cmd);
void sendTextMessage(SOCKET s, std::istringstream& cmd);
void removeLeadingWhitespace(std::string& str);
void logout(SOCKET s);
void waitForServerResponseLoop(SOCKET s);
void printServerMessage(const std::string& message);


// Function to print server messages without interfering with user input
void printServerMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(consoleMutex);

    // Erase current line
    std::cout << "\r" << std::string(currentInput.size() + 32, ' ') << "\r";

    // Print server message
    std::cout << message << "\n";

    // Reprint prompt and what the user was typing
    std::cout << currentInput;
    std::cout.flush();
}

// Function to read user input character by character, allowing for real-time display and editing
// This stupid function is necessary because the standard input functions block until the user presses enter, 
// which prevents us from displaying server messages in real-time while the user is typing.
// Works for both Windows and Unix-like systems, though I've only tested it on Windows. Cross your fingers for Unix compatibility!
std::string readInput() {
#ifdef _WIN32
    //std::cout << "> ";
    std::cout.flush();

    char c;
    while (true) {
        c = _getch();

        std::lock_guard<std::mutex> lock(consoleMutex);

        if (c == '\r') {                        // Enter
            std::cout << "\n";
            std::string submitted = currentInput;
            currentInput.clear();
            return submitted;

        } else if (c == '\b') {                 // Backspace
            if (!currentInput.empty()) {
                currentInput.pop_back();
                std::cout << "\b \b";
                std::cout.flush();
            }

        } else if (c >= 32 && c < 127) {        // Printable characters
            currentInput += c;
            std::cout << c;
            std::cout.flush();
        }
    }
#else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cout << "> ";
    std::cout.flush();

    char c;
    while (true) {
        read(STDIN_FILENO, &c, 1);

        std::lock_guard<std::mutex> lock(consoleMutex);

        if (c == '\n') {
            std::cout << "\n";
            std::string submitted = currentInput;
            currentInput.clear();
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return submitted;

        } else if (c == 127 || c == '\b') {
            if (!currentInput.empty()) {
                currentInput.pop_back();
                std::cout << "\b \b";
                std::cout.flush();
            }

        } else if (c >= 32 && c < 127) {
            currentInput += c;
            std::cout << c;
            std::cout.flush();
        }
    }
#endif
}








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
    std::cout << "My chat room client. Version One." << std::endl;
    std::thread responseThread(waitForServerResponseLoop, s);
    responseThread.detach();
    // Send and receive data.
    char buf[MAX_LINE];
    while (true)
    {
        std::string line;
        while (!(line = readInput()).empty()) {
            std::istringstream iss(line);
            handleCmd(iss, s);
        }
    }

    fflush(stdout);
    std::cout << "Closing socket." << std::endl;
    closesocket(s);
    return 0;
}


// Function to handle user commands and call the appropriate functions to send commands to the server
void handleCmd(std::istringstream& cmd, SOCKET s){
    std::string firstToken;
    cmd >> firstToken;
    std::transform(firstToken.begin(), firstToken.end(), firstToken.begin(),
        [](unsigned char c){ return std::tolower(c); }
    );
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
    } else if(firstToken == "logout"){
        logout(s);
    } else if(firstToken == "who"){
        // This command is handled in the server, so we just send it to the server and wait for the response
        std::string whoCmd = "who";
        send(s, whoCmd.c_str(), whoCmd.size(), 0);
    } else {
        std::cout << "Unknown command. Available commands: login, newuser, send, who, logout, exit" << std::endl;
    }
    return;
}


// Function to handle the login command, validating input and sending the command to the server
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
    return;
}


// Function to handle the newuser command, validating input and sending the command to the server
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
    return;
}


// Function to handle the send command, validating input and sending the command to the server
void sendTextMessage(SOCKET s, std::istringstream& cmd){
    
    
    std::string message;
    std::getline(cmd, message);
    removeLeadingWhitespace(message);
    // Remove leading whitespace from the message
    message.erase(0, message.find_first_not_of(" \t"));
    std::string sendCmd = "send " + message;
    if (message.size() > 256) {
        std::cout << "Denied. Message must be between 1 and 256 characters long." << std::endl;
        return;
    }
    send(s, sendCmd.c_str(), sendCmd.size(), 0);
    return;
}   

// Function to remove leading whitespace from a string
void removeLeadingWhitespace(std::string& str) {
    size_t firstNonWhitespace = str.find_first_not_of(" \t");
    if (firstNonWhitespace != std::string::npos) {
        str.erase(0, firstNonWhitespace);
    } else {
        str.clear(); // String is all whitespace
    }
}


// Function to handle the logout command, sending the command to the server
// The server will handle the logout and close the connection, so we just send the command and wait for the response
void logout(SOCKET s){
    std::string logoutCmd = "logout";
    send(s, logoutCmd.c_str(), logoutCmd.size(), 0);
    //waitForServerResponse(s);
    return;
}


// Function to wait for a response from the server and print it to the console
// This function is called in a separate thread to continuously listen for server messages without blocking user input
void waitForServerResponseLoop(SOCKET s){
    while(true){
        


        char buf[MAX_LINE];
        int len = recv(s, buf, MAX_LINE, 0);
        

        if (len > 0)
        {
            if (len >= MAX_LINE)
                len = MAX_LINE - 1; // prevent overflow
            buf[len] = '\0';
            printServerMessage(std::string(buf));
        }
        else if (len == 0)
        {
            printServerMessage("Server closed connection.");
            break;
        }
        else
        {
            printServerMessage("recv failed: " + std::to_string(WSAGetLastError()));
        }
        memset(buf, 0, MAX_LINE); // Clear the buffer for the next input
    }
}
