#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#define SERVER_PORT   9999
#define MAX_PENDING   5
#define MAX_LINE      256


   int main() {
   
    // Initialize Winsock.
      WSADATA wsaData;
      int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
      if ( iResult != NO_ERROR ){
         std::cout << "Error at WSAStartup()\n";
         return 1;
      }
      std::cout << "Winsock initialized.\n";
      return 0;
   }