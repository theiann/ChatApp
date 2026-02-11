#include <iostream>
#include <list>
#include "winsock2.h"
#include <ws2tcpip.h>

class Client
{
    private:
        SOCKET clientSocket;
        std::string clientUser;
        std::string clientPass;
        bool isAuthenticated;
        

    public:
        Client(SOCKET socket) : clientSocket(socket), isAuthenticated(false) {}


        SOCKET getSocket() const { return clientSocket; }
};



// this is a singleton class that manages all clients connected to the server
class ClientManager
{
    private:
        std::list<Client> clients;
        static ClientManager* instance;
        ClientManager() {}

    public:
        static ClientManager* getInstance()
        {
            if (instance == nullptr)
            {
                instance = new ClientManager();
            }
            return instance;
        }

        void addClient(SOCKET clientSocket)
        {
            clients.emplace_back(clientSocket);
            std::cout << "Client added. Total clients: " << clients.size() << std::endl;
        }

        void removeClient(SOCKET clientSocket)
        {
            clients.remove_if([clientSocket](const Client& client) {
                return client.getSocket() == clientSocket;
            });
            std::cout << "Client removed. Total clients: " << clients.size() << std::endl;
        }
};