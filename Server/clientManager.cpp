#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include "winsock2.h"
#include <ws2tcpip.h>
#include "clientManager.hpp"

Client::Client(SOCKET socket) : clientSocket(socket), isAuthenticated(false) {}

SOCKET Client::getSocket() const { return clientSocket; }
std::string Client::getUser() const { return clientUser; }
void Client::setUser(const std::string &user) { clientUser = user; }
bool Client::getIsAuthenticated() const { return isAuthenticated; }
void Client::setIsAuthenticated(bool authenticated) { isAuthenticated = authenticated; }


ClientManager::ClientManager() {
    // initialization code
}


ClientManager * ClientManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new ClientManager();
    }
    return instance;
}


// it does what it says on the tin, returns the list of clients
std::list<Client>& ClientManager::getClients() { return clients; }




// adds a client to the list of clients, leaves the rest of the client management to the Client class
void ClientManager::addClient(SOCKET clientSocket)
{
    clients.emplace_back(clientSocket);
    std::cout << "Client added. Total clients: " << clients.size() << std::endl;
}




// removes a client from the list of clients, leaves the rest of the client management to the Client class
void ClientManager::removeClient(SOCKET clientSocket)
{
    clients.remove_if([clientSocket](const Client &client)
                    { return client.getSocket() == clientSocket; });
    std::cout << "Client removed. Total clients: " << clients.size() << std::endl;
}

Client *ClientManager::getClient(SOCKET clientSocket)
{
    for (auto &client : clients)
    {
        if (client.getSocket() == clientSocket)
        {
            return &client;
        }
    }
    return nullptr; // Client not found
}

// checks if a user exists in the users.txt file, returns true if the user exists, false otherwise
bool ClientManager::isUserExists(const std::string &username)
{
    std::ifstream file("users.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("(" + username + ",") != std::string::npos)
            {
                file.close();
                return true; // User exists
            }
        }
        file.close();
    }
    else
    {
        std::cerr << "Unable to open users.txt for reading." << std::endl;
    }
    return false; // User does not exist
}




bool ClientManager::createUser(SOCKET clientSocket, const std::string &username, const std::string &password)
{
    std::cout << "Creating user: " << username << " with password: " << password << std::endl;
    std::ofstream file("users.txt", std::ios::app);
    if(isUserExists(username))
    {
        std::cout << "User already exists, cannot create user." << std::endl;
        return false; // User creation failed
    }
    if (file.is_open())
    {
        file << "\n(" << username << ", " << password << ")";
        file.close();
        std::cout << "User created successfully." << std::endl;
    }
    else
    {
        std::cerr << "Unable to open users.txt for writing." << std::endl;
        return false; // User creation failed
    }

    return true; // Assume user creation is successful
}
bool ClientManager::clientLogin(SOCKET clientSocket, const std::string &username, const std::string &password)
{
    Client *client = getClient(clientSocket);
    if (client == nullptr)
    {
        std::cout << "Client not found for socket: " << clientSocket << std::endl;
        return false; // Client not found
    }
    std::ifstream file("users.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("(" + username + ", " + password + ")") != std::string::npos)
            {
                file.close();
                std::cout << "User authenticated successfully." << std::endl;
                client->setUser(username);
                client->setIsAuthenticated(true);
                return true; // User authenticated successfully
            }
        }
        file.close();
    }
    else
    {
        std::cerr << "Unable to open users.txt for reading." << std::endl;
    }
    std::cout << "Authentication failed for user: " << username << std::endl;
    return false; // Authentication failed
}

// Define the static instance pointer
ClientManager *ClientManager::instance = nullptr;