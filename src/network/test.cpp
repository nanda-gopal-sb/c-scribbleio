#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
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

int main()
{

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];
    socklen_t server_len = sizeof(server_addr);

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

    if (ch == 1)
    {
        msg->type = 0;
        strncpy(msg->message, "LMAOO", 1023);
        strncpy(msg->senderName, "Gren", 1023);
        sendto(client_socket, msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        int n = recvfrom(client_socket, &room, sizeof(roomID), 0, (struct sockaddr *)&server_addr, &server_len);
        printf("Your Created Room %d\n", room.roomID);
        second_message.type = 1;
        second_message.roomID = room.roomID;
        n = recvfrom(client_socket, buffer, 10, 0, (struct sockaddr *)&server_addr, &server_len);
        if (strcmp(buffer, "Created"))
        {
            system("clear");
        }
    }

    while (1)
    {
        printf("Client: ");
        std::cin >> msg->type;
        buffer[strcspn(buffer, "\n")] = 0;
        sendto(client_socket, msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        if (strcmp(buffer, "exit") == 0)
        {
            break;
        }
        int n = recvfrom(client_socket, &room, sizeof(roomID), 0, (struct sockaddr *)&server_addr, &server_len);
        if (n < 0)
        {
            perror("Recvfrom failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';
        printf("Echoed string from server: %d\n", room.roomID);
        second_message.type = 1;
        second_message.roomID = room.roomID;
        sendto(client_socket, &second_message, sizeof(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        n = recvfrom(client_socket, buffer, 10, 0, (struct sockaddr *)&server_addr, &server_len);
        std::cout << buffer << "\n";
    }

    close(client_socket);
    return 0;
}
