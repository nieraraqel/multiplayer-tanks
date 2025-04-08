#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <SDL2/SDL.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection failed!" << std::endl;
        return -1;
    }

    std::cout << "Connected to server!" << std::endl;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Tank Battle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Rect tank = {100, 100, 50, 50};
    SDL_Rect enemy_tank = {400, 100, 50, 50};
    
    char buffer[BUFFER_SIZE];
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                std::string command;
                if (event.key.keysym.sym == SDLK_LEFT) command = "MOVE LEFT";
                if (event.key.keysym.sym == SDLK_RIGHT) command = "MOVE RIGHT";
                if (!command.empty()) send(sock, command.c_str(), command.size(), 0);
            }
        }
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        std::string response(buffer);
        if (response.find("PLAYER 1") != std::string::npos) {
            size_t x_pos = response.find("X:") + 2;
            size_t y_pos = response.find("Y:") + 2;
            tank.x = std::stoi(response.substr(x_pos, response.find(" ", x_pos) - x_pos));
            tank.y = std::stoi(response.substr(y_pos));
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &tank);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &enemy_tank);
        SDL_RenderPresent(renderer);
    }

    close(sock);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
