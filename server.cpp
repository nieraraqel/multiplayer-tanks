#include <iostream>
#include <algorithm>
#include <thread>
#include <vector>
#include <map>
#include <deque>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>
#include "gamestructs.hpp"

#define PORT 12345
#define BUFFER_SIZE 1024

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TANK_WIDTH = 50;
const int TANK_HEIGHT = 50;
const int BULLET_WIDTH = 10;
const int BULLET_HEIGHT = 10;
const int BULLET_SPEED = 5;

std::mutex state_mutex;

Data game_state;
int clients[2];

void Handle_Client(int client_socket, int client_id) {
    char buffer[BUFFER_SIZE];
    
    memset(buffer, 0, BUFFER_SIZE);
    int valread = recv(client_socket, buffer, BUFFER_SIZE, MSG_DONTWAIT);
    if (valread < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {       
            std::cerr << "Error from reading client\n";
            return;
        }
    }
    if (valread != 0) {
        Tank& myTank = client_id == 0 ? game_state.player : game_state.enemy;
        Tank& opponent = client_id == 0 ? game_state.enemy : game_state.player;
        auto& myBullets = client_id == 0 ? game_state.bullets : game_state.enemy_bullets;

        if (strcmp(buffer, "w") == 0 && myTank.y > 0) {
            myTank.y -= 5;
        } else if (strcmp(buffer, "s") == 0 && myTank.y < SCREEN_HEIGHT - TANK_HEIGHT) {
            myTank.y += 5;
        } else if (strcmp(buffer, "d") == 0) {
            int bulletX = (client_id == 0) ? myTank.x + TANK_WIDTH : myTank.x - BULLET_WIDTH;
            int bulletDir = (client_id == 0) ? BULLET_SPEED : -BULLET_SPEED;

            myBullets.push(Bullet{
                bulletX,
                myTank.y + TANK_HEIGHT / 2 - BULLET_HEIGHT / 2,
                bulletDir
            });
            std::cout << myBullets.getsize() << std::endl;
        }
    }
}

bool Check_Collision(const Bullet& bullet, const Tank& tank) {
    return bullet.x < tank.x + TANK_WIDTH &&
           bullet.x + BULLET_WIDTH > tank.x &&
           bullet.y < tank.y + TANK_HEIGHT &&
           bullet.y + BULLET_HEIGHT > tank.y;
}

void Update_Game_State() {
    Handle_Client(clients[0], 0);
    Handle_Client(clients[1], 1);

    auto& enemy = game_state.enemy;
    auto& bullets = game_state.bullets;
    auto& enemy_bullets = game_state.enemy_bullets;

    for (size_t i = 0; i < bullets.getsize(); ++ i)
        bullets[i].x += bullets[i].speedX;
    for (size_t i = 0; i < bullets.getsize(); ++ i){
        if (Check_Collision(bullets[i], enemy)) {
            if (enemy.health > 0) {
                enemy.health--;
                bullets.pop(i);
            }
        }
    }
    auto& bullets2 = game_state.enemy_bullets;
    auto& player = game_state.player;
    for (size_t i = 0; i < bullets2.getsize(); ++ i)
        bullets2[i].x += bullets2[i].speedX;
    for (size_t i = 0; i < bullets2.getsize(); ++ i){
        if (Check_Collision(bullets2[i], enemy)) {
            if (enemy.health > 0) {
                enemy.health--;
                bullets2.pop(i);
            }
        }
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 2) < 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started. Waiting for players..." << std::endl;

    for (int i = 0; i < 2; ++i) {
        clients[i] = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (clients[i] < 0) {
            std::cerr << "Accept failed" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Player " << i + 1 << " connected" << std::endl;
    }

    game_state.player = {50, 100, 10};
    game_state.enemy = {SCREEN_WIDTH - TANK_WIDTH - 50, 100, 10};

    while (true) {
        Update_Game_State();
        {
            for (int i = 0; i < 2; ++i) {
                Data view;

                if (i == 0) {
                    view.player = game_state.player;
                    view.enemy = game_state.enemy;
                } else {
                    view.player = game_state.enemy;
                    view.enemy = game_state.player;
                }

                send(clients[i], &view, sizeof(view), 0);
            }
        }

        usleep(80000);
    }

    return 0;
}
