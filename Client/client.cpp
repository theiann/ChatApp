#include <iostream>
#include "winsock2.h"

#define SERVER_PORT 9999
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
    std::cout << "Winsock initialized.";
    return 0;

    // translate the server name or IP address (128.90.54.1) to resolved IP address
    unsigned int ipaddr;
    // If the user input is an alpha name for the host, use gethostbyname()
    // If not, get host by addr (assume IPv4)
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

    
        ipaddr = inet_addr(argv[1]);
}