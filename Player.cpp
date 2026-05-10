#include "Player.h"

Player::Player() {
    // Set the player's starting location
    currentRoomId = "VAULT_ENTRANCE";

    // Initialize S.P.E.C.I.A.L. stats
    special["Strength"] = 8;
    special["Perception"] = 5;
    special["Endurance"] = 5;
    special["Charisma"] = 5;
    special["Intelligence"] = 5;
    special["Agility"] = 5;
    special["Luck"] = 5;

    // Initialize health based on Endurance
    maxHealth = 50 + (special["Endurance"] * 10);
    health = maxHealth;
    caps = 0;
    xp = 0;
    level = 1;
    hasMission = false;
    equippedArmor = "";
}
