#include "Game.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include "json.hpp"

using json = nlohmann::json;

void to_json(json& j, const Item& i) {
    j = json{{"name", i.name}, {"description", i.description}};
}

void from_json(const json& j, Item& i) {
    j.at("name").get_to(i.name);
    j.at("description").get_to(i.description);
}

void to_json(json& j, const Enemy& e) {
    j = json{{"name", e.name}, {"description", e.description}, {"health", e.health}, {"damage", e.damage}, {"xpValue", e.xpValue}, {"loot", e.loot}};
}

void from_json(const json& j, Enemy& e) {
    j.at("name").get_to(e.name);
    j.at("description").get_to(e.description);
    j.at("health").get_to(e.health);
    j.at("damage").get_to(e.damage);
    if (j.contains("xpValue")) j.at("xpValue").get_to(e.xpValue);
    else e.xpValue = 10; // Default XP
    j.at("loot").get_to(e.loot);
}

void to_json(json& j, const Room& r) {
    j = json{{"description", r.description}, {"exits", r.exits}, {"items", r.items}, {"enemies", r.enemies}};
}

void from_json(const json& j, Room& r) {
    j.at("description").get_to(r.description);
    j.at("exits").get_to(r.exits);
    j.at("items").get_to(r.items);
    j.at("enemies").get_to(r.enemies);
}

void to_json(json& j, const Player& p) {
    j = json{{"currentRoomId", p.currentRoomId}, {"inventory", p.inventory}, {"special", p.special}, {"equippedArmor", p.equippedArmor}, {"health", p.health}, {"maxHealth", p.maxHealth}, {"caps", p.caps}, {"xp", p.xp}, {"level", p.level}, {"perks", p.perks}, {"hasMission", p.hasMission}};
}

void from_json(const json& j, Player& p) {
    j.at("currentRoomId").get_to(p.currentRoomId);
    j.at("inventory").get_to(p.inventory);
    j.at("special").get_to(p.special);
    j.at("equippedArmor").get_to(p.equippedArmor);
    j.at("health").get_to(p.health);
    j.at("maxHealth").get_to(p.maxHealth);
    j.at("caps").get_to(p.caps);
    if (j.contains("xp")) j.at("xp").get_to(p.xp); else p.xp = 0;
    if (j.contains("level")) j.at("level").get_to(p.level); else p.level = 1;
    if (j.contains("perks")) j.at("perks").get_to(p.perks); else p.perks = {};
    j.at("hasMission").get_to(p.hasMission);
}

Game::Game() : isRunning(true) {
    std::srand(std::time(nullptr));
    setupWorld();
}

void Game::run() {
    std::cout << "Fallout DOS Adventure" << std::endl;
    std::cout << "=====================" << std::endl;
    std::cout << "Type 'go <direction>', 'look', 'look <item>', 'take <item>', 'drop <item>', 'inventory', 'character', 'use <item>', 'play <game>', 'talk <npc>', 'buy <item>', or 'quit'." << std::endl;
    std::cout << "Other commands may be available depending on the situation (e.g., 'open crate')." << std::endl << std::endl;


    while (isRunning) {
        render();
        processInput();
    }

    std::cout << "The wastes claim another victim... Thanks for playing." << std::endl;
}

void Game::setupWorld() {
    std::ifstream i("world.json");
    if (i.is_open()) {
        json j;
        try {
            i >> j;
            j.get_to(world);
        } catch (const json::exception& e) {
            std::cout << "Error parsing world.json: " << e.what() << std::endl;
        }
    } else {
        std::cout << "Could not open world.json!" << std::endl;
    }
}

void Game::render() {
    Room& currentRoom = world[player.currentRoomId];
    std::cout << currentRoom.description << std::endl;
    
    if (!currentRoom.items.empty()) {
        std::cout << "You see: ";
        for (auto const& [name, item] : currentRoom.items) {
            std::cout << item.name << " ";
        }
        std::cout << std::endl;
    }

    if (!currentRoom.enemies.empty()) {
        std::cout << "Enemies here: ";
        for (auto const& [name, enemy] : currentRoom.enemies) {
            std::cout << enemy.name << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Exits: ";
    for (auto const& [direction, roomId] : currentRoom.exits) {
        std::cout << direction << " ";
    }
    std::cout << std::endl;
}

void Game::processInput() {
    std::cout << "> ";
    std::string input;
    std::getline(std::cin, input);
    std::cout << std::endl; // Add a space for readability

    Command command = parseCommand(input);

    if (command.verb == "quit") {
        isRunning = false;
    } else if (command.verb == "save") {
        saveGame("savegame.json");
    } else if (command.verb == "load") {
        loadGame("savegame.json");
    } else if (command.verb == "go") {
        Room& currentRoom = world[player.currentRoomId];
        if (player.currentRoomId == "RIVER_BANK" && command.noun == "east" && !flags["bridgeCollapsed"]) {
            std::cout << "As you step onto the bridge, the rotting wood groans and gives way! You scramble back to the bank just in time as the entire structure collapses into the radioactive sludge." << std::endl;
            flags["bridgeCollapsed"] = true;
            currentRoom.exits.erase("east");
            currentRoom.description = "You stand on the muddy western bank of a glowing, irradiated river. The remains of a collapsed wooden bridge float in the water. To the north, a gangway leads up to a surprisingly well-lit riverboat. The scorched farm is to the west.";
        } else if (player.currentRoomId == "GANGWAY" && command.noun == "north") {
            if (player.caps >= 20) {
                std::cout << "The guard nods, seeing the pouch of caps at your belt. \"Welcome to the Lady Luck. Try not to lose it all at once,\" he grunts, stepping aside." << std::endl;
                player.currentRoomId = currentRoom.exits[command.noun];
            } else {
                std::cout << "The guard steps into your path, blocking the way with his shotgun. \"The Lady Luck is for high rollers only, smoothskin. You need at least 20 caps to get in. Scram.\"" << std::endl;
            }
        } else if (currentRoom.exits.count(command.noun)) {
            std::string nextRoomId = currentRoom.exits[command.noun];
            if (nextRoomId == "LILLYPAD_ISLAND") {
                bool hasStuffie = false;
                for (auto const& [name, item] : player.inventory) {
                    if (name.find("stuffed") != std::string::npos) {
                        hasStuffie = true;
                        break;
                    }
                }
                
                if (!hasStuffie) {
                    std::cout << "You dive into the irradiated water and swim toward the island. As you approach the shore, the beautiful voice turns into a terrifying screech. Lillypad lunges from the reeds, her webbed hands closing around your throat. \"NO STUFFIE? NO BREATHIE!\" she gurgles, dragging you down into the glowing depths." << std::endl;
                    player.health = 0;
                    isRunning = false;
                    return;
                } else {
                    std::cout << "As you swim closer, Lillypad watches you with wide, unblinking eyes. She sees the stuffed animal tucked into your pack and lets out a soft, trilling coo, beckoning you ashore." << std::endl;
                    player.currentRoomId = nextRoomId;
                }
            } else {
                player.currentRoomId = nextRoomId;
            }
        } else {
            std::cout << "You can't go that way." << std::endl;
        }
    } else if (command.verb == "look") {
        if (command.noun.empty()) {
            // Handled by render() in the main loop
        } else {
            Room& currentRoom = world[player.currentRoomId];
            if (currentRoom.items.count(command.noun)) {
                std::cout << currentRoom.items[command.noun].description << std::endl;
            } else if (player.inventory.count(command.noun)) {
                std::cout << player.inventory[command.noun].description << std::endl;
            } else {
                std::cout << "You don't see a '" << command.noun << "' here." << std::endl;
            }
        }
    } else if (command.verb == "take") {
        Room& currentRoom = world[player.currentRoomId];
        if (command.noun == "all") {
            if (currentRoom.items.empty()) {
                std::cout << "There is nothing here to take." << std::endl;
            } else {
                for (auto const& [name, item] : currentRoom.items) {
                    player.inventory[name] = item;
                    std::cout << "You take the " << item.name << "." << std::endl;
                }
                currentRoom.items.clear();
            }
        } else if (currentRoom.items.count(command.noun)) {
            Item item = currentRoom.items[command.noun];
            player.inventory[command.noun] = item;
            currentRoom.items.erase(command.noun);
            std::cout << "You take the " << item.name << "." << std::endl;
        } else {
            std::cout << "You don't see a '" << command.noun << "' here." << std::endl;
        }
    } else if (command.verb == "drop") {
        if (command.noun == "all") {
            if (player.inventory.empty()) {
                std::cout << "You are not carrying anything." << std::endl;
            } else {
                for (auto const& [name, item] : player.inventory) {
                    world[player.currentRoomId].items[name] = item;
                    std::cout << "You drop the " << item.name << "." << std::endl;
                }
                player.inventory.clear();
                player.equippedArmor = ""; // Unequip all
            }
        } else if (player.inventory.count(command.noun)) {
            Item item = player.inventory[command.noun];
            world[player.currentRoomId].items[command.noun] = item;
            player.inventory.erase(command.noun);
            std::cout << "You drop the " << item.name << "." << std::endl;
            if (player.equippedArmor == command.noun) {
                player.equippedArmor = "";
            }
        } else {
            std::cout << "You don't have a '" << command.noun << "'." << std::endl;
        }
    } else if (command.verb == "inventory") {
        if (player.inventory.empty()) {
            std::cout << "You are not carrying anything." << std::endl;
        } else {
            std::cout << "You are carrying:" << std::endl;
            for (auto const& [name, item] : player.inventory) {
                std::cout << "  - " << item.name << std::endl;
            }
        }
    } else if (command.verb == "character") {
        std::cout << "S.P.E.C.I.A.L. Stats:" << std::endl;
        for (auto const& [stat, value] : player.special) {
            std::cout << "  - " << stat << ": " << value << std::endl;
        }
        std::cout << "Health: " << player.health << "/" << player.maxHealth << std::endl;
        std::cout << "Caps: " << player.caps << std::endl;
        std::cout << "Equipped Armor: " << (player.equippedArmor.empty() ? "None" : player.equippedArmor) << std::endl;
    } else if (command.verb == "equip") {
        if (player.inventory.count(command.noun)) {
            if (command.noun == "power armor helmet" || command.noun == "leather armor") { // Add valid armor items here
                player.equippedArmor = command.noun;
                std::cout << "You equip the " << command.noun << "." << std::endl;
            } else {
                std::cout << "You can't equip that." << std::endl;
            }
        } else {
            std::cout << "You don't have a '" << command.noun << "' to equip." << std::endl;
        }
    } else if (command.verb == "unequip") {
        if (!player.equippedArmor.empty()) {
            if (command.noun.empty() || command.noun == player.equippedArmor) {
                std::cout << "You unequip the " << player.equippedArmor << "." << std::endl;
                player.equippedArmor = "";
            } else {
                std::cout << "You don't have that equipped." << std::endl;
            }
        } else {
            std::cout << "You have nothing equipped." << std::endl;
        }
    } else if (command.verb == "use") {
        if (player.inventory.count(command.noun)) {
            if (command.noun == "stimpak") {
                int healing = 25;
                for (const auto& perk : player.perks) if (perk == "Medic") healing = 37;
                player.health += healing;
                if (player.health > player.maxHealth) {
                    player.health = player.maxHealth;
                }
                player.inventory.erase(command.noun);
                std::cout << "You inject the stimpak. You feel a surge of energy as some of your wounds close." << std::endl;
                std::cout << "Health: " << player.health << "/" << player.maxHealth << std::endl;
            } else if (command.noun == "wasteland stew") {
                player.health += 35;
                if (player.health > player.maxHealth) player.health = player.maxHealth;
                player.inventory.erase(command.noun);
                std::cout << "You eat the warm wasteland stew. It's surprisingly good, and very filling." << std::endl;
                std::cout << "Health: " << player.health << "/" << player.maxHealth << std::endl;
            } else if (command.noun == "nuka-cola") {
                player.health += 5;
                if (player.health > player.maxHealth) player.health = player.maxHealth;
                player.inventory.erase(command.noun);
                std::cout << "You pop the cap on a bottle of warm Nuka-Cola. It's sugary, radioactive, and delicious." << std::endl;
            } else if (command.noun == "iguana-on-a-stick") {
                player.health += 15;
                if (player.health > player.maxHealth) player.health = player.maxHealth;
                player.inventory.erase(command.noun);
                std::cout << "You eat the charred lizard meat. It's chewy, but filling." << std::endl;
            } else {
                std::cout << "You can't use that item in this way." << std::endl;
            }
        } else {
            std::cout << "You don't have a '" << command.noun << "'." << std::endl;
        }
    } else if (command.verb == "attack") {
        Room& currentRoom = world[player.currentRoomId];
        if (currentRoom.enemies.count(command.noun)) {
            Enemy& enemy = currentRoom.enemies[command.noun];
            
            int playerDamage = player.special["Strength"];
            for (const auto& perk : player.perks) if (perk == "Bloody Mess") playerDamage += 5;
            
            std::string weaponUsed = "fists";
            
            if (!command.indirect_noun.empty() && command.preposition == "with") {
                if (player.inventory.count(command.indirect_noun)) {
                    if (command.indirect_noun == "plasma rifle") { playerDamage += 20; weaponUsed = command.indirect_noun; }
                    else if (command.indirect_noun == "fat man") { playerDamage += 100; weaponUsed = command.indirect_noun; }
                    else if (command.indirect_noun == "laser pistol") { playerDamage += 10; weaponUsed = command.indirect_noun; }
                    else if (command.indirect_noun == "spiked pipe") { playerDamage += 15; weaponUsed = command.indirect_noun; }
                    else if (command.indirect_noun == "rusty knife") { playerDamage += 5; weaponUsed = command.indirect_noun; }
                    else if (command.indirect_noun == "pipe") { playerDamage += 5; weaponUsed = command.indirect_noun; }
                    else if (command.indirect_noun == "wrench") { playerDamage += 5; weaponUsed = command.indirect_noun; }
                    else {
                        std::cout << "You can't use the '" << command.indirect_noun << "' as a weapon." << std::endl;
                        return;
                    }
                } else {
                    std::cout << "You don't have a '" << command.indirect_noun << "'." << std::endl;
                    return;
                }
            } else {
                // Auto-equip best weapon if not specified
                if (player.inventory.count("fat man")) { playerDamage += 100; weaponUsed = "fat man"; }
                else if (player.inventory.count("plasma rifle")) { playerDamage += 20; weaponUsed = "plasma rifle"; }
                else if (player.inventory.count("laser pistol")) { playerDamage += 10; weaponUsed = "laser pistol"; }
                else if (player.inventory.count("spiked pipe")) { playerDamage += 15; weaponUsed = "spiked pipe"; }
                else if (player.inventory.count("rusty knife")) { playerDamage += 5; weaponUsed = "rusty knife"; }
                else if (player.inventory.count("pipe")) { playerDamage += 5; weaponUsed = "pipe"; }
                else if (player.inventory.count("wrench")) { playerDamage += 5; weaponUsed = "wrench"; }
            }
            
            std::cout << "You attack the " << enemy.name << " with your " << weaponUsed << " for " << playerDamage << " damage!" << std::endl;
            enemy.health -= playerDamage;
            
            if (enemy.health <= 0) {
                std::cout << "You defeated the " << enemy.name << "!" << std::endl;
                gainXP(enemy.xpValue);
                if (!enemy.loot.name.empty() && enemy.loot.name != "Unnamed Item") {
                    std::cout << "It dropped: " << enemy.loot.name << "." << std::endl;
                    currentRoom.items[enemy.loot.name] = enemy.loot;
                }
                currentRoom.enemies.erase(command.noun);
            } else {
                int damageTaken = enemy.damage;
                if (player.equippedArmor == "power armor helmet") damageTaken -= 5;
                else if (player.equippedArmor == "leather armor") damageTaken -= 2;
                
                if (damageTaken < 1) damageTaken = 1;

                std::cout << "The " << enemy.name << " attacks you for " << damageTaken << " damage!" << std::endl;
                player.health -= damageTaken;
                std::cout << "Health: " << player.health << "/" << player.maxHealth << std::endl;
                if (player.health <= 0) {
                    std::cout << "You have been killed..." << std::endl;
                    isRunning = false;
                }
            }
        } else {
            std::cout << "There is no '" << command.noun << "' here to attack." << std::endl;
        }
    } else if (command.verb == "talk") {
        if (player.currentRoomId == "CASINO_BAR" && (command.noun == "bartender" || command.noun == "ghoul")) {
            if (!player.hasMission) {
                std::cout << "The bartender leans in. \"You look like the adventurous type. I'm looking for a laser pistol I heard was in a crashed vertibird to the west. Bring it to me, and I'll make it worth your while.\"" << std::endl;
                player.hasMission = true;
            } else if (player.inventory.count("laser pistol")) {
                std::cout << "\"You found it! Magnificent,\" the bartender beams. He takes the pistol and hands you a heavy pouch. \"Here's 100 caps for your trouble.\"" << std::endl;
                player.inventory.erase("laser pistol");
                player.caps += 100;
                player.hasMission = false; // Mission complete
                gainXP(50);
            } else {
                std::cout << "\"Any luck finding that laser pistol at the crash site to the west?\" the bartender asks." << std::endl;
            }
        } else if (player.currentRoomId == "SCAVENGER_DEN" && command.noun == "scavenger") {
            if (!flags["scavengerMissionAssigned"]) {
                std::cout << "The scavenger looks up from his radio. \"You! Smoothskin! Don't touch my junk! Wait... you look capable. I need a fusion core to power my rig. I think there's one locked in the old Red Rocket safe to the west. Bring it to me, and I'll give you something that packs a real punch.\"" << std::endl;
                flags["scavengerMissionAssigned"] = true;
            } else if (flags["scavengerMissionComplete"]) {
                std::cout << "\"Thanks for the core, smoothskin. Now leave me to my work. And don't point that rifle at me!\"" << std::endl;
            } else if (player.inventory.count("fusion core")) {
                std::cout << "The scavenger's eyes light up. \"A fusion core! Unbelievable! Hand it over!\" He snatches it eagerly, then tosses a bulky, glowing weapon to you. \"A deal's a deal. Take this plasma rifle. Don't blow your own arm off.\"" << std::endl;
                player.inventory.erase("fusion core");
                player.inventory["plasma rifle"] = Item("plasma rifle", "An advanced energy weapon firing superheated plasma. Extremely lethal.");
                flags["scavengerMissionComplete"] = true;
                gainXP(50);
            } else {
                std::cout << "\"You find that fusion core yet? Check the safe in the Red Rocket gas station!\"" << std::endl;
            }
        } else if (player.currentRoomId == "BOS_BUNKER" && command.noun == "scribe") {
            if (!flags["bosMissionAssigned"]) {
                std::cout << "The scribe looks up from her terminal. \"Civilian. We tracked a crashed Vertibird to the west. We need its flight recorder to secure our data. Retrieve it, and the Brotherhood will reward you.\"" << std::endl;
                flags["bosMissionAssigned"] = true;
            } else if (flags["flightRecorderDelivered"]) {
                std::cout << "\"Ad Victoriam. The data is secure. Good work.\"" << std::endl;
            } else if (player.inventory.count("flight recorder")) {
                std::cout << "\"You have the recorder! Excellent work. The Brotherhood honors its agreements. Take this spare Power Armor Helmet.\"" << std::endl;
                player.inventory.erase("flight recorder");
                player.inventory["power armor helmet"] = Item("power armor helmet", "A sturdy T-60 Power Armor helmet. Offers great protection.");
                flags["flightRecorderDelivered"] = true;
                flags["bosReputation"] = true;
                gainXP(75);
            } else {
                std::cout << "\"Any luck finding the crashed Vertibird to the west? We need that flight recorder.\"" << std::endl;
            }
        } else if (player.currentRoomId == "ENCLAVE_CAMP" && command.noun == "officer") {
            if (!flags["enclaveMissionAssigned"]) {
                std::cout << "The Enclave Officer sneers at you. \"A local. Listen closely. A Brotherhood Vertibird went down south of here. I want its flight recorder before they get their hands on it. Bring it to me, and you'll be compensated.\"" << std::endl;
                flags["enclaveMissionAssigned"] = true;
            } else if (flags["flightRecorderDelivered"]) {
                std::cout << "\"The Enclave always gets what it wants. Now get out of my sight.\"" << std::endl;
            } else if (player.inventory.count("flight recorder")) {
                std::cout << "\"You actually found it. Surprising for a wasteland savage. Hand it over. Here are some plasma grenades. Don't blow yourself up.\"" << std::endl;
                player.inventory.erase("flight recorder");
                player.inventory["plasma grenade"] = Item("plasma grenade", "A highly destructive energy explosive.");
                flags["flightRecorderDelivered"] = true;
                flags["enclaveReputation"] = true;
                gainXP(75);
            } else {
                std::cout << "\"I am losing my patience. Find the crashed Vertibird and bring me the flight recorder.\"" << std::endl;
            }
        } else if (player.currentRoomId == "RAILROAD_HIDEOUT" && command.noun == "agent") {
            if (!flags["railroadMissionAssigned"]) {
                std::cout << "The agent whispers, \"Keep your voice down. The Enclave has set up a camp north of the crashed Vertibird. Their guard carries a decryption key we desperately need to save a synth. Get it for us.\"" << std::endl;
                flags["railroadMissionAssigned"] = true;
            } else if (flags["railroadMissionComplete"]) {
                std::cout << "\"Thanks to you, a synth is going to make it out of the Commonwealth safely.\"" << std::endl;
            } else if (player.inventory.count("decryption key")) {
                std::cout << "\"You got the key! You're a lifesaver. Literally. Here, take this Stealth Boy. It might save your skin out there.\"" << std::endl;
                player.inventory.erase("decryption key");
                player.inventory["stealth boy"] = Item("stealth boy", "A personal cloaking device. Renders the user temporarily invisible.");
                flags["railroadMissionComplete"] = true;
                flags["railroadReputation"] = true;
                gainXP(100);
            } else {
                std::cout << "\"Please hurry. The Enclave guard at their camp has the decryption key we need.\"" << std::endl;
            }
        } else if (player.currentRoomId == "ABANDONED_MANSION" && command.noun == "rosie") {
            int prettyCanCount = 0;
            for (auto const& [name, item] : player.inventory) {
                if (name == "pretty bent tin can") prettyCanCount++;
            }

            if (!flags["rosieMissionAssigned"]) {
                std::cout << "Rosie looks up from her mountain of cans, her eyes surprisingly sharp within her weathered, once-beautiful face. She adjusts herself in her rattletrap wheelchair with a groan. \"Oh! A visitor! Mind the cans, dear, they're quite precious. I'm always looking to expand my collection.\"" << std::endl;
                std::cout << "\"Actually... I'm looking for something specific. I've heard rumors of 'pretty bent tin cans' that have rusted a beautiful purple color. If you could find 3 of them for me, I'll give you something truly special I found in the attic. A 'Fat Man' I think the pre-war folks called it. It's quite heavy, but I'm sure it packs a punch!\"" << std::endl;
                flags["rosieMissionAssigned"] = true;
            } else if (flags["rosieMissionComplete"]) {
                std::cout << "Rosie hums a raspy, cheerful tune as she polishes one of her purple cans, her hands trembling slightly. \"Thank you again for the additions to my collection, dear!\"" << std::endl;
            } else if (prettyCanCount >= 3) {
                std::cout << "Rosie's eyes widen, a ghost of her former radiance appearing for a moment. \"Oh my! They're even more beautiful than I imagined! Simply marvelous.\"" << std::endl;
                std::cout << "She takes three of the cans, her wheelchair rattling as she maneuvers to the back of the mansion, returning moments later with a massive, bulky weapon. \"Here you go, dear. A promise is a promise. Do be careful with it, it looks... explosive.\"" << std::endl;
                
                // Remove 3 cans
                int removed = 0;
                auto it = player.inventory.begin();
                while (it != player.inventory.end() && removed < 3) {
                    if (it->first == "pretty bent tin can") {
                        it = player.inventory.erase(it);
                        removed++;
                    } else {
                        ++it;
                    }
                }
                
                player.inventory["fat man"] = Item("fat man", "A pre-war tactical nuclear catapult. Extremely heavy and devastatingly powerful.");
                flags["rosieMissionComplete"] = true;
                gainXP(150);
            } else {
                std::cout << "\"Any luck finding those pretty purple cans, dear? I only need 3 of them!\" Rosie asks hopefully." << std::endl;
            }
        } else if (player.currentRoomId == "UNDER_BRIDGE" && command.noun == "crust") {
            std::cout << "Crust looks up, a piece of something unidentifiable stuck in his beard. \"Scabs... you got scabs? I need 'em. I'll pay you a cap for every one you find. Don't ask what they're for. Just bring 'em here.\"" << std::endl;
        } else if (player.currentRoomId == "LILLYPAD_ISLAND" && command.noun == "lillypad") {
            std::cout << "Lillypad tilts her head, her skin a pale, translucent green. \"Pretty... so soft. You brought a stuffie? I love them. They don't scream. They don't rot. They just stay... soft. Give me your stuffies, and I'll give you my pretty shiny caps.\"" << std::endl;
        } else {
            std::cout << "There's no one here by that name to talk to." << std::endl;
        }
    } else if (command.verb == "sell") {
        if (player.currentRoomId == "ABANDONED_MANSION") {
            if (player.inventory.count(command.noun)) {
                Item item = player.inventory[command.noun];
                int price = 10; // Base price
                if (command.noun == "pretty bent tin can") price = 50;
                else if (command.noun == "bent tin can") price = 25;
                else if (command.noun == "fat man") price = 250;
                else if (command.noun == "plasma rifle") price = 250;
                else if (command.noun == "laser pistol") price = 150;
                else if (command.noun == "fusion core") price = 100;
                else if (command.noun == "power armor helmet") price = 200;
                else if (command.noun == "stimpak") price = 20;
                else if (command.noun == "nuka-cola") price = 15;
                else if (command.noun == "iguana-on-a-stick") price = 25;
                else if (command.noun == "wasteland stew") price = 40;
                else if (command.noun == "spiked pipe") price = 60;
                else if (command.noun == "rusty knife") price = 15;
                else if (command.noun == "pipe") price = 10;
                else if (command.noun == "wrench") price = 15;
                else if (command.noun == "office key") price = 50;
                else if (command.noun == "decryption key") price = 100;
                else if (command.noun == "flight recorder") price = 150;
                else if (command.noun == "stealth boy") price = 150;
                else if (command.noun == "mutated fruit") price = 10;
                else if (command.noun == "roach meat") price = 10;

                player.caps += price;
                player.inventory.erase(command.noun);
                std::cout << "Rosie's eyes light up as she takes the " << item.name << ". \"Oh, this is marvelous! Here are " << price << " caps, dear.\"" << std::endl;
                if (command.noun == "bent tin can") {
                    std::cout << "Rosie hums happily and adds the can to one of her towering stacks." << std::endl;
                }
            } else {
                std::cout << "\"You don't seem to have a '" << command.noun << "' to sell me, dear,\" Rosie says gently." << std::endl;
            }
        } else if (player.currentRoomId == "LILLYPAD_ISLAND") {
            if (command.noun.find("stuffed") != std::string::npos) {
                if (player.inventory.count(command.noun)) {
                    Item item = player.inventory[command.noun];
                    player.inventory.erase(command.noun);
                    player.caps += 50;
                    std::cout << "Lillypad lets out a trilling coo as she takes the " << item.name << ", hugging it tightly against her wet chest. \"So soft... thank you, pretty thing. Here, take the shiny.\"" << std::endl;
                } else {
                    std::cout << "Lillypad's face darkens. \"You lie? You have no " << command.noun << " for Lillypad?\"" << std::endl;
                }
            } else {
                std::cout << "Lillypad ignores your junk. \"Not soft. Not a stuffie. I only want stuffies!\"" << std::endl;
            }
        } else if (player.currentRoomId == "UNDER_BRIDGE") {
            if (command.noun == "scab") {
                if (player.inventory.count("scab")) {
                    player.inventory.erase("scab");
                    player.caps += 1;
                    std::cout << "Crust snatches the scab from your hand and stuffs it into a dirty pouch. \"Good... good. Here's your cap.\"" << std::endl;
                } else {
                    std::cout << "\"You ain't got no scabs, smoothskin! Don't waste my time!\" Crust snarls." << std::endl;
                }
            } else {
                std::cout << "\"I only buy scabs. Keep your other junk,\" Crust grunts." << std::endl;
            }
        } else {
            std::cout << "There's no one here to sell anything to." << std::endl;
        }
    } else if (command.verb == "buy") {
        if (player.currentRoomId == "CASINO_BAR") {
            if (command.noun == "nuka-cola") {
                if (player.caps >= 10) {
                    player.caps -= 10;
                    player.inventory["nuka-cola"] = Item("nuka-cola", "A bottle of pre-war soda. Still fizzy, somehow.");
                    std::cout << "You buy a Nuka-Cola for 10 caps." << std::endl;
                } else {
                    std::cout << "\"You're short on caps, friend,\" the bartender says." << std::endl;
                }
            } else if (command.noun == "iguana-on-a-stick") {
                if (player.caps >= 20) {
                    player.caps -= 20;
                    player.inventory["iguana-on-a-stick"] = Item("iguana-on-a-stick", "A classic wasteland snack.");
                    std::cout << "You buy an iguana-on-a-stick for 20 caps." << std::endl;
                } else {
                    std::cout << "\"You're short on caps, friend,\" the bartender says." << std::endl;
                }
            } else {
                std::cout << "\"We don't sell that here. Try the Nuka-Cola (10 caps) or the Iguana-on-a-stick (20 caps),\" the bartender suggests." << std::endl;
            }
        } else {
            std::cout << "There's no one here to buy anything from." << std::endl;
        }
    } else if (command.verb == "craft") {
        craft(command.noun);
    } else if (command.verb == "open") {
        if (command.noun == "crate" && player.currentRoomId == "SHED") {
            if (flags["crateOpened"]) {
                std::cout << "The crate is already open and empty." << std::endl;
            } else if (player.special["Strength"] > 7) {
                int reward = 25;
                for (const auto& perk : player.perks) if (perk == "Fortune Finder") reward = 50;
                std::cout << "With a mighty heave, you wrench the crate open! Inside, you find a rusty pipe and a small pouch containing " << reward << " bottle caps." << std::endl;
                world[player.currentRoomId].items["pipe"] = Item("pipe", "A short, rusty metal pipe. It's heavy and could probably be used for bashing things.");
                player.caps += reward;
                flags["crateOpened"] = true;
                gainXP(20);
            } else {
                std::cout << "You strain to open the crate, but the lid won't budge. You're not strong enough." << std::endl;
            }
        } else if (command.noun == "door" && player.currentRoomId == "RED_ROCKET") {
            if (!command.indirect_noun.empty() && command.indirect_noun != "office key") {
                std::cout << "You can't open the door with that." << std::endl;
            } else if (player.inventory.count("office key")) {
                std::cout << "You insert the tarnished key into the lock. It turns with a satisfying click. The door to the backroom is now open!" << std::endl;
                world["RED_ROCKET"].exits["north"] = "RED_ROCKET_BACKROOM";
                world["RED_ROCKET"].description = "The skeletal remains of a Red Rocket gas station stand defiantly against the wind. The pumps are rusted solid. Inside the garage, an old terminal flickers faintly. An open door leads to the back room. The road heads south back to the shed. A faint trail leads east.";
                gainXP(30);
            } else {
                std::cout << "The metal door is locked tight. You need a key to open it." << std::endl;
            }
        } else if (command.noun == "safe" && player.currentRoomId == "RED_ROCKET_BACKROOM") {
            if (world[player.currentRoomId].description.find("open pre-war safe") != std::string::npos) {
                std::cout << "The safe is already open." << std::endl;
            } else if (!command.indirect_noun.empty() && command.indirect_noun != "bobby pin") {
                std::cout << "You can't pick the safe with that." << std::endl;
            } else if (player.inventory.count("bobby pin")) {
                int reward = 50;
                for (const auto& perk : player.perks) if (perk == "Fortune Finder") reward = 100;
                std::cout << "Using the bobby pin and a bit of finesse, you hear a satisfying *click*. The safe pops open! Inside you find a fusion core and " << reward << " bottle caps." << std::endl;
                world[player.currentRoomId].items["fusion core"] = Item("fusion core", "A small, humming cylinder of nuclear energy. Highly valuable.");
                player.caps += reward;
                world[player.currentRoomId].description = "You're in the dusty back office of the Red Rocket. Old holotapes and decaying paperwork litter a desk. In the corner sits an open pre-war safe.";
                player.inventory.erase("bobby pin");
                std::cout << "The bobby pin breaks in the process, but it was worth it." << std::endl;
                gainXP(50);
            } else {
                std::cout << "The safe is locked tight. You'll need something to pick the lock, like a bobby pin." << std::endl;
            }
        } else {
            std::cout << "You can't open that." << std::endl;
        }
    } else if (command.verb == "play") {
        if (player.currentRoomId == "RIVERBOAT_CASINO" && command.noun == "slots") {
            if (player.caps < 5) {
                std::cout << "You don't have enough caps to play. It costs 5 caps per pull." << std::endl;
            } else {
                player.caps -= 5;
                std::cout << "You feed 5 caps into the rusty slot machine and pull the lever..." << std::endl;
                int roll = std::rand() % 100;
                int multiplier = 1;
                for (const auto& perk : player.perks) if (perk == "Fortune Finder") multiplier = 2;

                if (roll < 50) {
                    std::cout << "The reels spin and land on... nothing matching. You lost your caps." << std::endl;
                } else if (roll < 80) {
                    int win = 10 * multiplier;
                    std::cout << "The reels spin and land on two cherries! You win " << win << " caps." << std::endl;
                    player.caps += win;
                } else if (roll < 95) {
                    int win = 25 * multiplier;
                    std::cout << "The reels spin and land on three bells! You win " << win << " caps!" << std::endl;
                    player.caps += win;
                } else {
                    int win = 100 * multiplier;
                    std::cout << "JACKPOT! Three Vault Boys line up! A cascade of " << win << " caps pours into the tray!" << std::endl;
                    player.caps += win;
                }
                std::cout << "Caps: " << player.caps << std::endl;
            }
        } else {
            std::cout << "You can't play that here." << std::endl;
        }
    } else {
        std::cout << "I don't understand that command." << std::endl;
    }
}

Command Game::parseCommand(const std::string& input) {
    Command command;
    
    // Convert to lowercase
    std::string lower_input = input;
    std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    std::stringstream ss(lower_input);
    std::string word;
    if (!(ss >> command.verb)) return command;
    
    std::vector<std::string> prepositions = {"in", "on", "with", "from", "to", "at", "under"};
    std::vector<std::string> articles = {"the", "a", "an"};
    
    bool found_prep = false;
    while(ss >> word) {
        if (std::find(articles.begin(), articles.end(), word) != articles.end()) {
            continue; // Skip articles
        }
        
        if (!found_prep && std::find(prepositions.begin(), prepositions.end(), word) != prepositions.end()) {
            command.preposition = word;
            found_prep = true;
        } else if (!found_prep) {
            if (!command.noun.empty()) command.noun += " ";
            command.noun += word;
        } else {
            if (!command.indirect_noun.empty()) command.indirect_noun += " ";
            command.indirect_noun += word;
        }
    }
    
    // Handle cases where the verb implicitly takes a preposition (e.g., "talk to ghoul")
    // If we only found a preposition and an indirect noun, shift them for easier processing
    if (command.noun.empty() && !command.preposition.empty() && !command.indirect_noun.empty()) {
        command.noun = command.indirect_noun;
        command.indirect_noun = "";
    }

    return command;
}

void Game::saveGame(const std::string& filename) {
    json j;
    j["player"] = player;
    j["world"] = world;
    j["flags"] = flags;

    std::ofstream o(filename);
    if (o.is_open()) {
        o << std::setw(4) << j << std::endl;
        std::cout << "Game saved successfully to " << filename << "." << std::endl;
    } else {
        std::cout << "Failed to open " << filename << " for saving." << std::endl;
    }
}

void Game::loadGame(const std::string& filename) {
    std::ifstream i(filename);
    if (!i.is_open()) {
        std::cout << "Could not open save file " << filename << "." << std::endl;
        return;
    }
    json j;
    try {
        i >> j;
        j.at("player").get_to(player);
        j.at("world").get_to(world);
        j.at("flags").get_to(flags);
        std::cout << "Game loaded successfully from " << filename << "." << std::endl;
    } catch (const json::exception& e) {
        std::cout << "Failed to load game: " << e.what() << std::endl;
    }
}

void Game::gainXP(int amount) {
    player.xp += amount;
    std::cout << "You gained " << amount << " XP! (Total: " << player.xp << "/" << player.level * 100 << ")" << std::endl;
    
    if (player.xp >= player.level * 100) {
        levelUp();
    }
}

void Game::levelUp() {
    player.level++;
    player.xp = 0; // Reset XP for next level or keep overflow? Let's reset for simplicity.
    player.maxHealth += 10;
    
    // Toughness Perk check
    for (const auto& perk : player.perks) {
        if (perk == "Toughness") player.maxHealth += 10; // Extra health if they already have it? No, wait.
    }
    
    player.health = player.maxHealth;
    
    std::cout << std::endl;
    std::cout << "*******************************" << std::endl;
    std::cout << "LEVEL UP! You are now level " << player.level << "!" << std::endl;
    std::cout << "Your health has been restored and increased." << std::endl;
    std::cout << "Choose a Perk:" << std::endl;
    std::cout << "1. Toughness (+20 Max Health)" << std::endl;
    std::cout << "2. Bloody Mess (+5 Base Damage)" << std::endl;
    std::cout << "3. Medic (Stimpaks heal 50% more)" << std::endl;
    std::cout << "4. Fortune Finder (50% more caps from gambling/crates)" << std::endl;
    std::cout << "Enter the number of your choice: ";
    
    int choice;
    std::string line;
    std::getline(std::cin, line);
    try {
        choice = std::stoi(line);
    } catch (...) {
        choice = 1; // Default to Toughness if invalid input
    }
    
    if (choice == 1) {
        player.perks.push_back("Toughness");
        player.maxHealth += 20;
        player.health = player.maxHealth;
        std::cout << "You chose: Toughness. Max Health is now " << player.maxHealth << "." << std::endl;
    } else if (choice == 2) {
        player.perks.push_back("Bloody Mess");
        std::cout << "You chose: Bloody Mess. Your attacks will do more damage." << std::endl;
    } else if (choice == 3) {
        player.perks.push_back("Medic");
        std::cout << "You chose: Medic. Stimpaks are now more effective." << std::endl;
    } else if (choice == 4) {
        player.perks.push_back("Fortune Finder");
        std::cout << "You chose: Fortune Finder. You'll find more caps now." << std::endl;
    } else {
        player.perks.push_back("Toughness");
        player.maxHealth += 20;
        player.health = player.maxHealth;
        std::cout << "Invalid choice. Defaulting to Toughness. Max Health is now " << player.maxHealth << "." << std::endl;
    }
    std::cout << "*******************************" << std::endl << std::endl;
}

void Game::craft(const std::string& itemName) {
    if (itemName == "wasteland stew") {
        if (player.inventory.count("mutated fruit") && player.inventory.count("roach meat")) {
            player.inventory.erase("mutated fruit");
            player.inventory.erase("roach meat");
            player.inventory["wasteland stew"] = Item("wasteland stew", "A hearty, if slightly glowing, stew made from wasteland ingredients. Restores 35 health.");
            std::cout << "You carefully prepare a bowl of wasteland stew. It smells... interesting." << std::endl;
        } else {
            std::cout << "You need a 'mutated fruit' and 'roach meat' to craft 'wasteland stew'." << std::endl;
        }
    } else if (itemName == "spiked pipe") {
        if (player.inventory.count("pipe") && player.inventory.count("wrench")) {
            player.inventory.erase("pipe");
            player.inventory.erase("wrench");
            player.inventory["spiked pipe"] = Item("spiked pipe", "A heavy metal pipe with wicked-looking metal spikes welded onto it. Does significant damage.");
            std::cout << "You use the wrench to hammer some sharp metal bits into the pipe. The resulting weapon looks formidable." << std::endl;
        } else {
            std::cout << "You need a 'pipe' and 'wrench' to craft a 'spiked pipe'." << std::endl;
        }
    } else {
        std::cout << "You don't know how to craft '" << itemName << "'. Available recipes: 'wasteland stew', 'spiked pipe'." << std::endl;
    }
}
