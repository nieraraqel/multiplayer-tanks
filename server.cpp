#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 12345
#define BUFFER_SIZE 1024

struct Player {
    int id;
    int x, y;
};

std::vector<int> clients;
std::vector<Player> players = {{1, 100, 100}, {2, 400, 100}};

void broadcast(const std::string &message) {
    for (int client : clients) {
        send(client, message.c_str(), message.size(), 0);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    std::cout << "Server started on port " << PORT << std::endl;

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        clients.push_back(new_socket);
        std::cout << "New player connected!" << std::endl;

        char buffer[BUFFER_SIZE];
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes = recv(new_socket, buffer, BUFFER_SIZE, 0);
            if (bytes <= 0) break;

            std::string msg(buffer);
            std::cout << "Received: " << msg << std::endl;

            if (msg == "MOVE LEFT") players[0].x -= 10;
            if (msg == "MOVE RIGHT") players[0].x += 10;

            broadcast("PLAYER 1 X:" + std::to_string(players[0].x) + " Y:" + std::to_string(players[0].y));
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
