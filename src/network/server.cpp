// imports for multithreading
#include <thread>
#include <mutex>
// imports for data parsing, and basic data structures
#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <stdexcept>
#include <cstring>
// imports for basic UDP and TCP sockets
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// Imports for random number generator
#include <random>
#include <chrono>

struct Packet
{
    int type;
    int roomID;
    char senderName[1024];
    char message[1024];
};
struct Message
{
    Packet packet;
    sockaddr_in sender;
};
struct roomID
{
    int roomID;
    char roomName[1024];
};

// Gemini Gave me this
int generate_simple_id()
{
    static std::mt19937 generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> distribution(0, 9999);
    return distribution(generator);
}

class Server
{
private:
    std::queue<Message> packQueue;
    std::unordered_map<int, std::vector<sockaddr_in>> rooms;
    std::mutex mux;
    int server_socket;
    sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int BUFFER_SIZE = 1024;
    template <typename T>
    int sendReply(T &x)
    {
        int n = sendto(server_socket, &x, sizeof(T), 0, (struct sockaddr *)&client_addr, client_len);
        return n;
    }
    template <typename T>
    int sendMessage(T &x, sockaddr_in &destination)
    {
        int n = sendto(server_socket, &x, sizeof(T), 0, (struct sockaddr *)&destination, client_len);
        return n;
    }

public:
    Server(int port)
    {
        server_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_socket < 0)
        {
            throw std::runtime_error("Socket creation failed");
        }
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            close(server_socket);
            throw std::runtime_error("Bind failed");
        }
        std::cout << "Server listening on port " << port << std::endl;
    }
    void recvString()
    {
        while (true)
        {
            Packet msg;
            Message message;
            ssize_t n = recvfrom(server_socket, &msg, sizeof(msg), 0, (sockaddr *)&client_addr, &client_len);
            if (n < 0)
            {
                throw std::runtime_error("Recvfrom failed");
            }
            mux.lock();
            message.packet = msg;
            message.sender = client_addr;
            packQueue.push(message);
            mux.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    void operations()
    {
        while (true)
        {
            if (packQueue.size() > 0)
            {
                mux.lock();
                Message front = packQueue.front();
                switch (front.packet.type)
                {
                case 0:
                    addRoom();
                    break;
                case 1:
                    addPlayerToRoom(front);
                    break;
                case 3:
                    broadcast(front.packet.roomID, front);
                }
                packQueue.pop();
                mux.unlock();
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    void addRoom()
    {
        std::cout << "New Room requested\n";
        int uuid = generate_simple_id();
        rooms[uuid];
        roomID room;
        room.roomID = uuid;
        sendReply<roomID>(room);
    }
    void addPlayerToRoom(Message &front)
    {
        std::cout << "Joining a room\n";
        auto req_room_it = rooms.find(front.packet.roomID);
        if (req_room_it != rooms.end())
        {
            std::cout << req_room_it->second.size() << "\n";
            req_room_it->second.push_back(front.sender);
            std::cout << req_room_it->second.size() << "\n";
            sendReply<const char[5]>("sent");
        }
        else
        {
            sendReply<const char[10]>("not found");
        }
    }
    void broadcast(int roomId, Message recvived)
    {
        auto req_room_it = rooms.find(roomId);
        if (req_room_it != rooms.end())
        {
            for (int i = 0; i < req_room_it->second.size(); i++)
            {
                sendMessage<char[1024]>(recvived.packet.message, req_room_it->second[i]);
            }
        }
    }
    ~Server()
    {
        if (server_socket > 0)
        {
            close(server_socket);
        }
    }
};
int main()
{
    Server serve(8000);
    std::thread t2(&Server::operations, &serve);
    std::thread t1(&Server::recvString, &serve);
    t2.join();
    t1.join();
}