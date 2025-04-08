#include <iostream>
#include <map>
#include <SDL2/SDL.h>


int main() {
    std::cout << "Connected to server!" << std::endl;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Tank Battle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int rect_pos_y = 100;
    int enemy_rect_pos_y = 100;
    int bullet_pos_y;
    int bullet_pos_x;
    
    bool running = true;
    std::map<int, bool> keyboard;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                
                if (event.key.keysym.sym == SDLK_w) {
                    keyboard[event.key.keysym.sym] = false;
                    std::cout << "w" << "\n";
                    rect_pos_y -= 10;
                    break;
                }
                if (event.key.keysym.sym == SDLK_s) {
                    keyboard[event.key.keysym.sym] = true;
                    std::cout << "s" << "\n";
                    rect_pos_y += 10;
                    break;
                }
                if (event.key.keysym.sym == SDLK_d) {
                    keyboard[event.key.keysym.sym] = true;
                    std::cout << "d" << "\n";
                    bullet_pos_y = rect_pos_y;
                    bullet_pos_x = 50;
                    while (bullet_pos_x != 400) {
                        bullet_pos_x++;
                        SDL_Rect bullet = {bullet_pos_x, bullet_pos_y, 100, 100};
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderFillRect(renderer, &bullet);
                    }
                    break;
                }
                if (event.key.keysym.sym == SDLK_UP) {
                        keyboard[event.key.keysym.sym] = false;
                        std::cout << "w" << "\n";
                        enemy_rect_pos_y -= 10;
                        break;
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                        keyboard[event.key.keysym.sym] = true;
                        std::cout << "s" << "\n";
                        enemy_rect_pos_y += 10;
                        break;
                }
            }
            
            SDL_Rect tank = {0, rect_pos_y, 50, 50};
            
            SDL_Rect enemy_tank = {400, enemy_rect_pos_y, 50, 50};

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderFillRect(renderer, &tank);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &enemy_tank);
            SDL_RenderPresent(renderer);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
