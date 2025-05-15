#include <iostream>
#include <algorithm>
#include <cstring>
#include <deque>
#include <map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "gamestructs.hpp"

#define SERVER_IP "127.0.0.1"
#define PORT 12345

#define BUFFER_SIZE 1024

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TANK_WIDTH = 50;
const int TANK_HEIGHT = 50;
const int BULLET_WIDTH = 10;
const int BULLET_HEIGHT = 10;


int server_socket = socket(AF_INET, SOCK_DGRAM, 0);

void Socket_Init() {
    struct sockaddr_in server_address;

    if (server_socket == -1) {
        std::cerr << "Socket creation error";
        exit(errno);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);

    int connected = connect(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connected < 0) {
        std::cerr << "Connection failed!" << std::endl;
        exit(errno);
    }

    std::cout << "Connected to server!" << std::endl;

    send(server_socket, "READY!", 7, 0);
}

void Handle_Events(bool& running, std::map<SDL_Keycode, bool>& keys) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            keys[event.key.keysym.sym] = true;
        } else if (event.type == SDL_KEYUP) {
            keys[event.key.keysym.sym] = false;
        }
    }
}

void Process_Input(std::map<SDL_Keycode, bool>& keys) {
    if (keys[SDLK_w]) {
        send(server_socket, "w", 2, 0);
        std::cout << "W\n"; 
    }
    if (keys[SDLK_s]) {
        send(server_socket, "s", 2, 0);
        std::cout << "S\n"; 
    }
    if (keys[SDLK_d]) {
        send(server_socket, "d", 2, 0);
        std::cout << "D\n"; 
    }
}

void Draw_Tank(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color color, int x, int y, int index) {
    SDL_Rect srcRect = {index * 16, 0, 16, 16};
    SDL_Rect dstRect = {x, y, TANK_WIDTH, TANK_HEIGHT};
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}

void Draw_Bullet(SDL_Renderer* renderer, int x, int y) {
    SDL_Rect rect = {x, y, BULLET_WIDTH, BULLET_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void Draw_Text(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int main() {
    Socket_Init();
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cerr << "TTF Init Error: " << TTF_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Tank Battle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << "\n";
        return 1;
    }
    
    SDL_Texture* texture = IMG_LoadTexture(renderer, "sprites.png");
    if (!texture) {
        std::cerr << "Failed to load texture: " << IMG_GetError() << "\n";
        return 1;
    }

    char buffer[BUFFER_SIZE];
    std::map<SDL_Keycode, bool> keys;

    bool running = true;

    Data data{};

    while (running) {
        Handle_Events(running, keys);
        Process_Input(keys);

        int bytes_received = recv(server_socket, &data, sizeof(data), MSG_DONTWAIT);
        if (bytes_received < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                std::cerr << "recv error\n";
                break;
            }
        }
        if (bytes_received == 0) continue;


        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        int player_x = 100;
        int enemy_x = SCREEN_WIDTH - 100 - TANK_WIDTH;

        SDL_Color player_color = {0, 255, 0};
        SDL_Color enemy_color = {255, 0, 0};

        Draw_Tank(renderer, texture, player_color, player_x, data.player.y, 0);
        Draw_Tank(renderer, texture, enemy_color, enemy_x, data.enemy.y, 1);

        for (size_t i = 0; i < data.bullets.getsize(); ++i) {
            if (data.bullets[i].x == 0 && data.bullets[i].y == 0) continue;
            Draw_Bullet(renderer, data.bullets[i].x, data.bullets[i].y);
        }
        //std::cout << data.bullets.getsize() << std::endl;

        for (size_t i = 0; i < data.enemy_bullets.getsize(); ++i) {
            if (data.enemy_bullets[i].x == 0 && data.enemy_bullets[i].y == 0) continue;
            Draw_Bullet(renderer, data.enemy_bullets[i].x, data.enemy_bullets[i].y);
        }

        SDL_Color text_color = {255, 255, 255};

        Draw_Text(renderer, font, "HP: " + std::to_string(data.player.health),
            player_x, data.player.y - 30, text_color);

        Draw_Text(renderer, font, "HP: " + std::to_string(data.enemy.health),
            enemy_x, data.enemy.y - 30, text_color);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    close(server_socket);
    return 0;
}
