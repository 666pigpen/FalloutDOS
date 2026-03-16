#include "Game.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

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
    Room vault_entrance("You are standing at the massive, circular entrance of Vault 13. The door is sealed behind you. A harsh, unfamiliar sunlight beats down from above. A path leads north into the wasteland.");
    vault_entrance.exits["north"] = "WASTELAND_1";
    world["VAULT_ENTRANCE"] = vault_entrance;

    Room wasteland_1("You are in a barren stretch of wasteland. The ground is cracked and dry. To the south is the Vault entrance. To the north, you see a dilapidated shed. A dirt track leads east, and a trail of debris leads west. A desiccated corpse lies near a rock.");
    wasteland_1.exits["south"] = "VAULT_ENTRANCE";
    wasteland_1.exits["north"] = "SHED";
    wasteland_1.exits["east"] = "SCORCHED_FARM";
    wasteland_1.exits["west"] = "CRASHED_VERTIBIRD";
    wasteland_1.items["office key"] = Item("office key", "A tarnished key with an old plastic tag. It might open something nearby.");
    world["WASTELAND_1"] = wasteland_1;

    Room crashed_vertibird("You follow the trail of debris to the wreckage of an old, rusted Vertibird. The chassis offers little shelter from the wind. The only clear path is back east, but there's an ominous camp up north.");
    crashed_vertibird.exits["east"] = "WASTELAND_1";
    crashed_vertibird.exits["north"] = "ENCLAVE_CAMP";
    crashed_vertibird.items["laser pistol"] = Item("laser pistol", "A sleek, energy-based weapon. It looks like it still has some charge.");
    crashed_vertibird.items["flight recorder"] = Item("flight recorder", "A black box containing the Vertibird's pre-crash data. Highly valuable to the Brotherhood.");
    world["CRASHED_VERTIBIRD"] = crashed_vertibird;

    Room red_rocket("The skeletal remains of a Red Rocket gas station stand defiantly against the wind. The pumps are rusted solid. Inside the garage, an old terminal flickers faintly. A locked metal door leads to the back room. The road heads south back to the shed. A faint trail leads east.");
    red_rocket.exits["south"] = "SHED";
    red_rocket.exits["east"] = "ABANDONED_OUTPOST";
    red_rocket.items["wrench"] = Item("wrench", "A heavy steel wrench. Useful for repairs or cracking skulls.");
    red_rocket.items["bobby pin"] = Item("bobby pin", "A small piece of bent metal. Perfect for picking locks.");
    world["RED_ROCKET"] = red_rocket;

    Room red_rocket_backroom("You're in the dusty back office of the Red Rocket. Old holotapes and decaying paperwork litter a desk. In the corner sits a sturdy, locked pre-war safe.");
    red_rocket_backroom.exits["south"] = "RED_ROCKET";
    world["RED_ROCKET_BACKROOM"] = red_rocket_backroom;

    Room shed("You've reached a small, rusty shed. The door hangs precariously on one hinge. It smells of dust and decay. A large, heavy-looking crate sits in the corner. The wasteland is to the south, and an old road leads further north.");
    shed.exits["south"] = "WASTELAND_1";
    shed.exits["north"] = "RED_ROCKET";
    shed.items["stimpak"] = Item("stimpak", "A syringe full of a miraculous healing chemical. Using it should restore some health.");
    shed.enemies["radroach"] = Enemy("radroach", "A disgusting, oversized roach the size of a dog.", 15, 3, Item("roach meat", "Slightly irradiated roach meat. Yuck."));
    world["SHED"] = shed;

    Room scorched_farm("You arrive at a collection of ruined farm buildings. The ground is charred and black. A two-headed cow lies dead in a field. A dirt track leads west, and another path leads east toward a river. You notice a hidden trapdoor on the floor leading down.");
    scorched_farm.exits["west"] = "WASTELAND_1";
    scorched_farm.exits["east"] = "RIVER_BANK";
    scorched_farm.exits["down"] = "RAILROAD_HIDEOUT";
    scorched_farm.items["mutated fruit"] = Item("mutated fruit", "A strange, glowing fruit. It looks... vaguely edible.");
    world["SCORCHED_FARM"] = scorched_farm;

    Room river_bank("You stand on the muddy western bank of a glowing, irradiated river. A dangerously rickety wooden bridge spans the water to the east. To the north, a gangway leads up to a surprisingly well-lit riverboat. The scorched farm is to the west.");
    river_bank.exits["west"] = "SCORCHED_FARM";
    river_bank.exits["north"] = "GANGWAY";
    river_bank.exits["east"] = "ACROSS_RIVER";
    world["RIVER_BANK"] = river_bank;

    Room gangway("You walk up the creaking wooden gangway toward the riverboat. A tall, scarred ghoul wearing a pre-war security uniform stands guard at the entrance, resting a hand casually on a combat shotgun. He watches you closely but doesn't stop you from entering.");
    gangway.exits["south"] = "RIVER_BANK";
    gangway.exits["north"] = "RIVERBOAT_CASINO";
    world["GANGWAY"] = gangway;

    Room casino("You enter the dilapidated paddlewheel riverboat retrofitted into a casino. Smooth jazz plays over a crackling PA system. Ghouls in worn suits deal cards, and a row of functioning slot machines clink in the corner. To the west is a neon-lit bar. The exit is south.");
    casino.exits["south"] = "GANGWAY";
    casino.exits["west"] = "CASINO_BAR";
    world["RIVERBOAT_CASINO"] = casino;

    Room bar("The casino bar is surprisingly clean. The air is thick with the smell of cheap booze and stale cigarettes. A ghoul bartender with a neatly trimmed mustache is busy polishing a glass. A list of drinks and snacks is scrawled on a chalkboard. The casino floor is back east.");
    bar.exits["east"] = "RIVERBOAT_CASINO";
    world["CASINO_BAR"] = bar;

    Room outpost("An abandoned Brotherhood of Steel outpost. Sandbags are piled up around a rusty command tent. The wind howls through the torn canvas. A rugged path leads east toward some jagged rocks. The Red Rocket station is to the west. Inside the tent, a heavy metal hatch leads down.");
    outpost.exits["west"] = "RED_ROCKET";
    outpost.exits["east"] = "SCAVENGER_DEN";
    outpost.exits["down"] = "BOS_BUNKER";
    outpost.enemies["raider"] = Enemy("raider", "A scrawny, chem-addled raider holding a rusty knife.", 25, 5, Item("rusty knife", "A poorly maintained knife. Better than using your fists."));
    world["ABANDONED_OUTPOST"] = outpost;

    Room scavenger_den("A small, hidden cave that has been converted into a makeshift home. Junk and scrap parts are piled everywhere. A grimy, heavily armed scavenger sits on an overturned bucket, tinkering with a broken radio. The exit to the outpost is back west.");
    scavenger_den.exits["west"] = "ABANDONED_OUTPOST";
    world["SCAVENGER_DEN"] = scavenger_den;

    Room bos_bunker("You have climbed down into a dimly lit, fortified Brotherhood of Steel bunker. The hum of generators fills the air. A heavily armored scribe is studying a terminal. The ladder back up is the only exit.");
    bos_bunker.exits["up"] = "ABANDONED_OUTPOST";
    world["BOS_BUNKER"] = bos_bunker;

    Room enclave_camp("You stumble upon a heavily fortified Enclave camp. High-tech barricades and energy turrets surround the perimeter. An Enclave Officer stands by a tactical map, glaring at you. An Enclave Guard patrols nearby. The crashed vertibird is to the south.");
    enclave_camp.exits["south"] = "CRASHED_VERTIBIRD";
    enclave_camp.enemies["guard"] = Enemy("guard", "An Enclave guard clad in advanced Power Armor.", 40, 10, Item("decryption key", "A high-tech Enclave decryption key."));
    world["ENCLAVE_CAMP"] = enclave_camp;

    Room railroad_hideout("You descend into a dimly lit, damp cellar that serves as a covert Railroad hideout. Maps and scattered intelligence reports line the walls. A cautious Railroad Agent watches you closely. The trapdoor leading up to the farm is the only way out.");
    railroad_hideout.exits["up"] = "SCORCHED_FARM";
    world["RAILROAD_HIDEOUT"] = railroad_hideout;
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
            player.currentRoomId = currentRoom.exits[command.noun];
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
                player.health += 25;
                if (player.health > player.maxHealth) {
                    player.health = player.maxHealth;
                }
                player.inventory.erase(command.noun);
                std::cout << "You inject the stimpak. You feel a surge of energy as some of your wounds close." << std::endl;
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
            if (player.inventory.count("plasma rifle")) playerDamage += 20;
            else if (player.inventory.count("laser pistol")) playerDamage += 10;
            else if (player.inventory.count("rusty knife")) playerDamage += 5;
            else if (player.inventory.count("pipe")) playerDamage += 5;
            else if (player.inventory.count("wrench")) playerDamage += 5;
            
            std::cout << "You attack the " << enemy.name << " for " << playerDamage << " damage!" << std::endl;
            enemy.health -= playerDamage;
            
            if (enemy.health <= 0) {
                std::cout << "You defeated the " << enemy.name << "!" << std::endl;
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
            } else {
                std::cout << "\"Please hurry. The Enclave guard at their camp has the decryption key we need.\"" << std::endl;
            }
        } else {
            std::cout << "There's no one here by that name to talk to." << std::endl;
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
    } else if (command.verb == "open") {
        if (command.noun == "crate" && player.currentRoomId == "SHED") {
            if (flags["crateOpened"]) {
                std::cout << "The crate is already open and empty." << std::endl;
            } else if (player.special["Strength"] > 7) {
                std::cout << "With a mighty heave, you wrench the crate open! Inside, you find a rusty pipe and a small pouch containing 25 bottle caps." << std::endl;
                world[player.currentRoomId].items["pipe"] = Item("pipe", "A short, rusty metal pipe. It's heavy and could probably be used for bashing things.");
                player.caps += 25;
                flags["crateOpened"] = true;
            } else {
                std::cout << "You strain to open the crate, but the lid won't budge. You're not strong enough." << std::endl;
            }
        } else if (command.noun == "door" && player.currentRoomId == "RED_ROCKET") {
            if (player.inventory.count("office key")) {
                std::cout << "You insert the tarnished key into the lock. It turns with a satisfying click. The door to the backroom is now open!" << std::endl;
                world["RED_ROCKET"].exits["north"] = "RED_ROCKET_BACKROOM";
                world["RED_ROCKET"].description = "The skeletal remains of a Red Rocket gas station stand defiantly against the wind. The pumps are rusted solid. Inside the garage, an old terminal flickers faintly. An open door leads to the back room. The road heads south back to the shed. A faint trail leads east.";
            } else {
                std::cout << "The metal door is locked tight. You need a key to open it." << std::endl;
            }
        } else if (command.noun == "safe" && player.currentRoomId == "RED_ROCKET_BACKROOM") {
            if (world[player.currentRoomId].description.find("open pre-war safe") != std::string::npos) {
                std::cout << "The safe is already open." << std::endl;
            } else if (player.inventory.count("bobby pin")) {
                std::cout << "Using the bobby pin and a bit of finesse, you hear a satisfying *click*. The safe pops open! Inside you find a fusion core and 50 bottle caps." << std::endl;
                world[player.currentRoomId].items["fusion core"] = Item("fusion core", "A small, humming cylinder of nuclear energy. Highly valuable.");
                player.caps += 50;
                world[player.currentRoomId].description = "You're in the dusty back office of the Red Rocket. Old holotapes and decaying paperwork litter a desk. In the corner sits an open pre-war safe.";
                player.inventory.erase("bobby pin");
                std::cout << "The bobby pin breaks in the process, but it was worth it." << std::endl;
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
                if (roll < 50) {
                    std::cout << "The reels spin and land on... nothing matching. You lost your caps." << std::endl;
                } else if (roll < 80) {
                    std::cout << "The reels spin and land on two cherries! You win 10 caps." << std::endl;
                    player.caps += 10;
                } else if (roll < 95) {
                    std::cout << "The reels spin and land on three bells! You win 25 caps!" << std::endl;
                    player.caps += 25;
                } else {
                    std::cout << "JACKPOT! Three Vault Boys line up! A cascade of 100 caps pours into the tray!" << std::endl;
                    player.caps += 100;
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
    ss >> command.verb;
    
    // Handle multi-word nouns, e.g. "open heavy crate"
    if (ss >> word) {
        command.noun = word;
        while(ss >> word) {
            command.noun += " " + word;
        }
    }

    return command;
}
