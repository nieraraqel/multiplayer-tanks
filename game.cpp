#include <iostream>
#include <map>
#include <deque>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TANK_WIDTH = 64;
const int TANK_HEIGHT = 64;
const int BULLET_WIDTH = 24;
const int BULLET_HEIGHT = 16;
const int BULLET_SPEED = 5;

struct Tank {
    int x, y;
    int health = 10;
    SDL_Color color;
};

struct Bullet {
    int x, y;
    int speedX;
};

void DrawTank(SDL_Renderer* renderer, SDL_Texture* texture, const Tank& tank, int index) {
    SDL_Rect srcRect = {index * 16, 0, 16, 16};
    SDL_Rect dstRect = {tank.x, tank.y, TANK_WIDTH, TANK_HEIGHT};
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
    // SDL_Rect rect = {tank.x, tank.y, TANK_WIDTH, TANK_HEIGHT};
    // SDL_SetRenderDrawColor(renderer, tank.color.r, tank.color.g, tank.color.b, 255);
    // SDL_RenderFillRect(renderer, &rect);
}

void DrawBullet(SDL_Renderer* renderer, SDL_Texture* texture, const Bullet& bullet) {
    SDL_Rect srcRect = {32, 0, 6, 4};
    SDL_Rect dstRect = {bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT};
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
    // SDL_Rect rect = {bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT};
    // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // SDL_RenderFillRect(renderer, &rect);
}

void DrawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
              int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

bool CheckCollision(const Bullet& bullet, const Tank& tank) {
    SDL_Rect bulletRect = {bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT};
    SDL_Rect tankRect = {tank.x, tank.y, TANK_WIDTH, TANK_HEIGHT};
    return SDL_HasIntersection(&bulletRect, &tankRect);
}

void HandleEvents(bool& running, std::map<SDL_Keycode, bool>& keys) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) running = false;
        else if (event.type == SDL_KEYDOWN) keys[event.key.keysym.sym] = true;
        else if (event.type == SDL_KEYUP) keys[event.key.keysym.sym] = false;
    }
}

void ProcessInput(std::map<SDL_Keycode, bool>& keys, Tank& player, Tank& enemy,
                  std::deque<Bullet>& bulletsPlayer, std::deque<Bullet>& bulletsEnemy) {
    if (keys[SDLK_w]) {
        if (player.y > 0) {
            player.y -= 5;
        }
    }
    if (keys[SDLK_s]) {
        if (player.y < 550) {
            player.y += 5;
        }
    }
    if (keys[SDLK_UP]) {
        if (enemy.y > 0) {
            enemy.y -= 5;
        }
    }
    if (keys[SDLK_DOWN]) {
        if (enemy.y < 550) {
            enemy.y += 5;
        }
    }

    if (keys[SDLK_d]) {
        bulletsPlayer.push_back(Bullet{
            player.x + TANK_WIDTH,
            player.y + TANK_HEIGHT / 2 - BULLET_HEIGHT / 2,
            BULLET_SPEED
        });
        keys[SDLK_d] = false;
    }

    if (keys[SDLK_LEFT]) {
        bulletsEnemy.push_back(Bullet{
            enemy.x - BULLET_WIDTH,
            enemy.y + TANK_HEIGHT / 2 - BULLET_HEIGHT / 2,
            -BULLET_SPEED
        });
        keys[SDLK_LEFT] = false;
    }

    if (keys[SDLK_d]) {
        bulletsPlayer.push_back(Bullet{
            player.x + TANK_WIDTH,
            player.y + TANK_HEIGHT / 2 - BULLET_HEIGHT / 2,
            BULLET_SPEED
        });
        keys[SDLK_d] = false;
    }

    if (keys[SDLK_LEFT]) {
        bulletsEnemy.push_back(Bullet{
            enemy.x - BULLET_WIDTH,
            enemy.y + TANK_HEIGHT / 2 - BULLET_HEIGHT / 2,
            -BULLET_SPEED
        });
        keys[SDLK_LEFT] = false;
    }
}

void UpdateBullets(std::deque<Bullet>& bullets, Tank& target) {
    for (auto& b : bullets) {
        b.x += b.speedX;
    }
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [&](const Bullet& b) {
                if (CheckCollision(b, target)) {
                    if (target.health > 0) {
                        target.health--;
                        std::cout << target.health << std::endl;
                    }
                    return true;
                }
                return (b.x < 0 || b.x > SCREEN_WIDTH);
            }), bullets.end()
    );
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cerr << "TTF Init Error: " << TTF_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Tank Battle",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font\n";
        return 1;
    }
    // ------ SDL IMAGE
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << "\n";
        return 1;
    }
    
    SDL_Texture* texture = IMG_LoadTexture(renderer, "../sprites.png");
    if (!texture) {
        std::cerr << "Failed to load texture: " << IMG_GetError() << "\n";
        return 1;
    }

    // ------ TANKS
    Tank player = {50, 100, 10, {0, 255, 0}};
    Tank enemy = {SCREEN_WIDTH - TANK_WIDTH - 50, 100, 10, {255, 0, 0}};

    std::deque<Bullet> bulletsPlayer;
    std::deque<Bullet> bulletsEnemy;
    std::map<SDL_Keycode, bool> keys;

    bool running = true;

    while (running) {
        HandleEvents(running, keys);
        ProcessInput(keys, player, enemy, bulletsPlayer, bulletsEnemy);

        UpdateBullets(bulletsPlayer, enemy);
        UpdateBullets(bulletsEnemy, player);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        DrawTank(renderer, texture, player, 0);
        DrawTank(renderer, texture, enemy, 1);

        for (auto& b : bulletsPlayer) {
            DrawBullet(renderer, texture, b);
        }
        for (auto& b : bulletsEnemy) {
            DrawBullet(renderer, texture, b);
        }

        SDL_Color white = {255, 255, 255};
        DrawText(renderer, font, "Player1 HP: " + std::to_string(player.health), 10, 10, white);
        DrawText(renderer, font, "Player2 HP: " + std::to_string(enemy.health), SCREEN_WIDTH - 200, 10, white);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(texture);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
