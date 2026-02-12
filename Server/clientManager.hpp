#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <winsock2.h>
#include <string>
#include <list>
#include <iostream>
#include <fstream>

// --------------------
// Client Class
// --------------------
class Client
{
private:
    SOCKET clientSocket;
    std::string clientUser;
    std::string clientPass;
    bool isAuthenticated;

public:
    explicit Client(SOCKET socket);

    SOCKET getSocket() const;
};


// --------------------
// ClientManager Singleton
// --------------------
class ClientManager
{
private:
    std::list<Client> clients;
    static ClientManager* instance;

    ClientManager();  // private constructor

public:
    static ClientManager* getInstance();

    std::list<Client>& getClients();

    void addClient(SOCKET clientSocket);
    void removeClient(SOCKET clientSocket);

    bool isUserExists(const std::string& username);
    bool createUser(SOCKET clientSocket,
                    const std::string& username,
                    const std::string& password);
};

#endif // CLIENTMANAGER_H