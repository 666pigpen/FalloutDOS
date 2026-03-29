#ifndef COMMAND_H
#define COMMAND_H

#include <string>

struct Command {
    std::string verb;
    std::string noun;
    std::string preposition;
    std::string indirect_noun;
};

#endif // COMMAND_H
