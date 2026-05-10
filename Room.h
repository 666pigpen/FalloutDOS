#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <map>
#include "Item.h"
#include "Enemy.h"

class Room {
public:
    Room(const std::string& desc = "This is an empty room.");

    std::string description;
    std::map<std::string, std::string> exits; // Key: direction, Value: room ID
    std::map<std::string, Item> items;       // Key: item name, Value: Item object
    std::map<std::string, Enemy> enemies;    // Key: enemy name, Value: Enemy object
};

#endif // ROOM_H
