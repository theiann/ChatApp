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
    bool isAuthenticated;

public:
    explicit Client(SOCKET socket);

    SOCKET getSocket() const;
    std::string getUser() const;
    void setUser(const std::string& user);
    bool getIsAuthenticated() const;
    void setIsAuthenticated(bool authenticated);
    void logout() {
        setIsAuthenticated(false);
        setUser("");
    }
    std::string toString() const {
        return "socket=" + std::to_string(clientSocket) + "\nuser=" + clientUser + "\nauthenticated=" + (isAuthenticated ? "true" : "false");
    }
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
    Client* getClient(SOCKET clientSocket);
    bool isUserExists(const std::string& username);
    bool createUser(SOCKET clientSocket,
                    const std::string& username,
                    const std::string& password);
    void printClients();
    bool clientLogin(SOCKET clientSocket,
                    const std::string& username,
                    const std::string& password);
    bool userLogout(SOCKET clientSocket);
    bool sendTextMessage(SOCKET clientSocket, const std::string& message);
    void sendToClient(SOCKET clientSocket, const std::string &message);
};

#endif // CLIENTMANAGER_H