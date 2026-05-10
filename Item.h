#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item {
public:
    Item(const std::string& name = "Unnamed Item", const std::string& desc = "It's an item.");

    std::string name;
    std::string description;
};

#endif // ITEM_H
