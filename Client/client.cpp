#include <iostream>
#include "winsock2.h"

#define SERVER_PORT 9999
#define MAX_LINE 256

int main(int argc, char **argv){
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
    std::cout << "Winsock initialized.";
    return 0;
}