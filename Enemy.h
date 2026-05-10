#ifndef ENEMY_H
#define ENEMY_H

#include <string>
#include "Item.h"

struct Enemy {
    std::string name;
    std::string description;
    int health;
    int damage;
    int xpValue;
    Item loot; // Dropped upon death

    Enemy(const std::string& name = "Unknown", const std::string& desc = "A mysterious foe.", int hp = 10, int dmg = 2, int xp = 10, const Item& lootItem = Item())
        : name(name), description(desc), health(hp), damage(dmg), xpValue(xp), loot(lootItem) {}
};

#endif // ENEMY_H