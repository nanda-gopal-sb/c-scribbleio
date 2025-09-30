#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
using namespace std;
struct message
{
    int type;
    int roomID;
    char senderName[1024];
    char message[1024];
} second_message;
struct roomID
{
    int roomID;
    char roomName[1024];
} room;
message *msg = new message;
mutex mux;
int client_socket;
struct sockaddr_in server_addr;
char buffer[1024];
socklen_t server_len = sizeof(server_addr);

#include <atomic>
std::atomic<bool> running{true};

static void print_prompt()
{
    std::cout << "You: " << std::flush;
}

void recv_loop()
{
    char recvbuf[2048];
    while (running)
    {
        int n = recvfrom(client_socket, recvbuf, sizeof(recvbuf) - 1, 0,
                         (struct sockaddr *)&server_addr, &server_len);
        if (n <= 0)
        {
            if (!running)
                break;
            continue;
        }
        recvbuf[n] = '\0';
        std::lock_guard<std::mutex> lock(mux);
        std::cout << "\r\033[KPeer: " << recvbuf << std::endl;
        print_prompt();
    }
}

void send_loop()
{
    std::string line;
    // consume leftover newline from previous >> input
    std::getline(std::cin, line);
    print_prompt();
    while (running && std::getline(std::cin, line))
    {
        if (line == "/quit" || line == "/exit")
        {
            running = false;
            close(client_socket);
            break;
        }
        if (line.empty())
        {
            print_prompt();
            continue;
        }
        msg->type = 3;
        strncpy(msg->message, line.c_str(), sizeof(msg->message) - 1);
        msg->message[sizeof(msg->message) - 1] = '\0';
        sendto(client_socket, msg, sizeof(*msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        print_prompt();
    }
}
int main()
{

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("invalid address");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    char name[1024] = {0};
    cout << "Enter your name\n";
    cin >> name;

    int ch = 0;

    cout << "1-Create Room\n";
    cout << "2- Join Room\n";
    cin >> ch;
    if (ch == 1)
    {
        msg->type = 0;
        sendto(client_socket, msg, sizeof(*msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        int n = recvfrom(client_socket, &room, sizeof(roomID), 0, (struct sockaddr *)&server_addr, &server_len);
        printf("Your Created Room %d\n", room.roomID);
        msg->type = 1;
        msg->roomID = room.roomID;
        sendto(client_socket, msg, sizeof(*msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        thread t2(send_loop);
        thread t1(recv_loop);

        t1.join();
        t2.join();
        // system("clear");
        // system("clear");
    }
    if (ch == 2)
    {
        int roomID = 0;
        cin >> roomID;
        msg->type = 1;
        msg->roomID = roomID;
        sendto(client_socket, msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        thread t2(send_loop);
        thread t1(recv_loop);
        t1.join();
        t2.join();
    }
}
