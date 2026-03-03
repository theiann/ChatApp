#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <winsock2.h>
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <mutex>
#include <queue>




typedef struct Message {
    SOCKET sender;
    std::string text;

    Message(SOCKET s, const std::string& t) : sender(s), text(t) {}
} Message;


class MessageQueue{
private:
    std::queue<Message> m_queue;
    std::mutex m_mutex;

public:
    void push(const Message& message) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(message);
    }

    bool pop(Message& message) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        message = m_queue.front();
        m_queue.pop();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
};


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
    bool sendTextMessage(SOCKET clientSocket, const std::string& message, const std::string& recipient);
    void sendToClient(SOCKET clientSocket, const std::string &message);
    void broadcastMessage(const std::string &message);
    void broadcastToAllExceptSender(const std::string &message, SOCKET senderSocket);
    void listOnlineUsers(SOCKET clientSocket);
};

#endif // CLIENTMANAGER_H