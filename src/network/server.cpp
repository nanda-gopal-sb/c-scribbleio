#include <thread>
#include <mutex>

#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <stdexcept>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <random>
#include <chrono>

struct Packet
{
    int type;
    int roomID;
    char senderName[1024];
    char message[1024];
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
std::queue<Packet> packQueue;
std::unordered_map<int, std::vector<sockaddr_in>> rooms;
std::mutex mux;
class Server
{
private:
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int BUFFER_SIZE = 1024;

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
            ssize_t n = recvfrom(server_socket, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, &client_len);
            if (n < 0)
            {
                throw std::runtime_error("Recvfrom failed");
            }
            mux.lock();
            packQueue.push(msg);
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
                // std::cout << packQueue.front().type << "\n";
                int type = packQueue.front().type;
                switch (type)
                {
                case 0:
                    std::cout << "New Room requested\n";
                    int uuid = generate_simple_id();
                    rooms[uuid];
                    roomID room;
                    room.roomID = uuid;
                    int n = sendto(server_socket, &room, sizeof(roomID), 0, (struct sockaddr *)&client_addr, client_len);
                    std::cout << n << "\n";
                    break;
                }

                packQueue.pop();
                mux.unlock();
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
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