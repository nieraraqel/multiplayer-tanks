#include "gamestructs.hpp"
#include <iostream>

Bullet& Bullets::operator[](size_t index){
    return bullets[index];
}

void Bullets::push(Bullet bullet){
    if (size >= capacity) { 
        std::cerr << "Out of capacity\n";
        return;
    }
    bullets[size++] = bullet; 
}

void Bullets::pop(size_t index){
    if (index >= size) {
        std::cerr << "Index out of range\n";
        return;
    }
    for (size_t i = index; i < size - 1; ++i) {
        bullets[i] = bullets[i + 1];
    }
}

size_t Bullets::getsize(){
    return size;
}

