#ifndef PIPBOY_H
#define PIPBOY_H

#include "Player.h"
#include "Room.h"
#include <map>
#include <string>

class Pipboy {
public:
    void open(Player& player,
              const std::map<std::string, Room>& world,
              const std::map<std::string, bool>& flags);

private:
    enum class Tab { STAT, INV, DATA, MAP };

    Tab currentTab = Tab::STAT;
    int selectedRow = 0;

    Player* p = nullptr;
    const std::map<std::string, Room>* world = nullptr;
    const std::map<std::string, bool>* flags = nullptr;

    void draw();
    void drawFrame(int rows, int cols);
    void drawTabs(int cols);
    void drawStat(int contentTop, int contentH, int contentW);
    void drawInv(int contentTop, int contentH, int contentW);
    void drawData(int contentTop, int contentH, int contentW);
    void drawMap(int contentTop, int contentH, int contentW);
};

#endif // PIPBOY_H
