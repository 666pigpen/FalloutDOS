#ifndef GAME_H
#define GAME_H

#include "Room.h"
#include "Player.h"
#include "Command.h"
#include <map>
#include <string>

class Game {
public:
    Game();
    void run();

private:
    void setupWorld();
    void processInput();
    void render();
    Command parseCommand(const std::string& input);

    bool isRunning;
    Player player;
    std::map<std::string, Room> world;
    std::map<std::string, bool> flags;
};

#endif // GAME_H
