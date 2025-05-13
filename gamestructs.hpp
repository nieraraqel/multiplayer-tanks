#include <cstdlib>

#pragma pack(push, 1)
struct Tank {
    int x, y;
    int health;
};

struct Bullet {
    int x, y;
    int speedX;
};

class Bullets {
private:
    const static size_t capacity = 10;
    size_t size = 0;
    Bullet bullets[capacity];
public:
    Bullet& operator[](size_t index);
    void push(Bullet bullet);
    void pop(size_t index);
    size_t getsize();
};

struct Data {
    Tank player;
    Tank enemy;
    Bullets bullets;
    Bullets enemy_bullets;
    bool is_player_one;
};

#pragma pack(pop)
