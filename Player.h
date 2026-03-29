#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <map>
#include <vector>
#include "Item.h"

class Player {
public:
    Player();

    std::string currentRoomId;
    std::map<std::string, Item> inventory; // Key: item name, Value: Item object
    std::map<std::string, int> special;    // Key: stat name, Value: stat value
    std::string equippedArmor;
    int health;
    int maxHealth;
    int caps;
    int xp;
    int level;
    std::vector<std::string> perks;
    bool hasMission;
};

#endif // PLAYER_H
