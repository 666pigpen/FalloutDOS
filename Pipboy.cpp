#include "Pipboy.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <vector>
#include <string>

// ── ANSI helpers ─────────────────────────────────────────────────────────────
#define A_RESET   "\033[0m"
#define A_BOLD    "\033[1m"
#define A_DIM     "\033[2m"
#define A_GREEN   "\033[32m"
#define A_BGREEN  "\033[92m"
#define A_BLACK   "\033[30m"
#define A_BG_BLK  "\033[40m"
#define A_BG_GRN  "\033[42m"
#define A_HIDE    "\033[?25l"
#define A_SHOW    "\033[?25h"
#define A_CLR     "\033[2J\033[H"

static void at(int r, int c) { printf("\033[%d;%dH", r, c); }
static void hline(int r, int c, int len, const char* ch) {
    at(r, c);
    for (int i = 0; i < len; i++) fputs(ch, stdout);
}

// ── Terminal raw mode ─────────────────────────────────────────────────────────
static struct termios g_orig;

static void rawOn() {
    tcgetattr(STDIN_FILENO, &g_orig);
    struct termios t = g_orig;
    t.c_lflag &= ~(ECHO | ICANON);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
}

static void rawOff() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig);
}

enum { K_UP=1000, K_DOWN, K_LEFT, K_RIGHT, K_ENTER, K_ESC };

static int readKey() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return 0;
    if (c == '\033') {
        char s[2];
        if (read(STDIN_FILENO, &s[0], 1) != 1) return K_ESC;
        if (read(STDIN_FILENO, &s[1], 1) != 1) return K_ESC;
        if (s[0] == '[') {
            if (s[1] == 'A') return K_UP;
            if (s[1] == 'B') return K_DOWN;
            if (s[1] == 'C') return K_RIGHT;
            if (s[1] == 'D') return K_LEFT;
        }
        return K_ESC;
    }
    if (c == '\r' || c == '\n') return K_ENTER;
    if (c == 'q' || c == 'Q' || c == 'p' || c == 'P') return K_ESC;
    return (unsigned char)c;
}

static void getTermSize(int& rows, int& cols) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    rows = (ws.ws_row > 0) ? ws.ws_row : 24;
    cols = (ws.ws_col > 0) ? ws.ws_col : 80;
}

// ── Pip-Boy entry point ───────────────────────────────────────────────────────
void Pipboy::open(Player& player,
                  const std::map<std::string, Room>& world_,
                  const std::map<std::string, bool>& flags_) {
    p     = &player;
    world = &world_;
    flags = &flags_;
    currentTab = Tab::STAT;
    selectedRow = 0;

    rawOn();
    printf(A_HIDE A_BG_BLK A_BGREEN);
    fflush(stdout);

    bool running = true;
    while (running) {
        draw();
        int k = readKey();
        switch (k) {
            case K_ESC: running = false; break;
            case K_LEFT:
                if      (currentTab == Tab::INV)  currentTab = Tab::STAT;
                else if (currentTab == Tab::DATA) currentTab = Tab::INV;
                else if (currentTab == Tab::MAP)  currentTab = Tab::DATA;
                selectedRow = 0;
                break;
            case K_RIGHT:
                if      (currentTab == Tab::STAT) currentTab = Tab::INV;
                else if (currentTab == Tab::INV)  currentTab = Tab::DATA;
                else if (currentTab == Tab::DATA) currentTab = Tab::MAP;
                selectedRow = 0;
                break;
            case K_UP:
                if (selectedRow > 0) selectedRow--;
                break;
            case K_DOWN:
                selectedRow++;
                break;
            case K_ENTER:
                if (currentTab == Tab::INV) {
                    // Equip/unequip armors from inventory
                    std::vector<std::string> armors;
                    for (auto& [name, _] : p->inventory) {
                        if (name == "power armor helmet" || name == "leather armor")
                            armors.push_back(name);
                    }
                    if (selectedRow < (int)armors.size()) {
                        const std::string& sel = armors[selectedRow];
                        if (p->equippedArmor == sel)
                            p->equippedArmor = "";
                        else
                            p->equippedArmor = sel;
                    }
                }
                break;
        }
    }

    printf(A_RESET A_SHOW A_CLR);
    fflush(stdout);
    rawOff();
}

// ── Frame & tabs ─────────────────────────────────────────────────────────────
void Pipboy::drawFrame(int rows, int cols) {
    // Top border
    at(1, 1); printf(A_BOLD "╔");
    for (int c = 2; c < cols; c++) fputs("═", stdout);
    fputs("╗", stdout);

    // Side borders
    for (int r = 2; r < rows; r++) {
        at(r, 1);    fputs("║", stdout);
        at(r, cols); fputs("║", stdout);
    }

    // Bottom border
    at(rows, 1); fputs("╚", stdout);
    for (int c = 2; c < cols; c++) fputs("═", stdout);
    fputs("╝", stdout);

    // Header line
    at(2, 3);
    printf(A_BOLD A_BGREEN "  P I P - B O Y  3 0 0 0" A_RESET A_BG_BLK A_BGREEN);
    at(3, 1); fputs("╠", stdout);
    for (int c = 2; c < cols; c++) fputs("═", stdout);
    fputs("╣", stdout);
    printf(A_RESET A_BG_BLK A_BGREEN);
}

void Pipboy::drawTabs(int cols) {
    const char* labels[] = { " STAT ", " INV  ", " DATA ", " MAP  " };
    int numTabs = 4;
    int tabRow = 4;

    at(tabRow, 2);
    for (int i = 0; i < numTabs; i++) {
        if ((int)currentTab == i) {
            printf(A_BLACK A_BG_GRN "%s" A_RESET A_BG_BLK A_BGREEN, labels[i]);
        } else {
            printf(A_DIM "%s" A_RESET A_BG_BLK A_BGREEN, labels[i]);
        }
        fputs(" ", stdout);
    }

    // Separator below tabs
    at(5, 1); fputs("╠", stdout);
    for (int c = 2; c < cols; c++) fputs("═", stdout);
    fputs("╣", stdout);

    at(6, 3);
    printf(A_DIM "← → change tab   ↑ ↓ scroll   ENTER select   P close" A_RESET A_BG_BLK A_BGREEN);
}

// ── Main draw dispatcher ──────────────────────────────────────────────────────
void Pipboy::draw() {
    int rows, cols;
    getTermSize(rows, cols);

    printf(A_CLR A_BG_BLK A_BGREEN);
    drawFrame(rows, cols);
    drawTabs(cols);

    int contentTop = 8;      // first content row
    int contentH = rows - contentTop - 1;
    int contentW = cols - 4; // inside the borders with padding

    switch (currentTab) {
        case Tab::STAT: drawStat(contentTop, contentH, contentW); break;
        case Tab::INV:  drawInv (contentTop, contentH, contentW); break;
        case Tab::DATA: drawData(contentTop, contentH, contentW); break;
        case Tab::MAP:  drawMap (contentTop, contentH, contentW); break;
    }

    fflush(stdout);
}

// ── STAT tab ──────────────────────────────────────────────────────────────────
void Pipboy::drawStat(int top, int h, int w) {
    int r = top;
    at(r++, 3); printf(A_BOLD "S.P.E.C.I.A.L." A_RESET A_BG_BLK A_BGREEN);
    at(r++, 3); fputs("──────────────────────────", stdout);

    const char* statOrder[] = {
        "Strength","Perception","Endurance","Charisma","Intelligence","Agility","Luck"
    };
    for (const char* s : statOrder) {
        if (r - top >= h) break;
        auto it = p->special.find(s);
        int val = (it != p->special.end()) ? it->second : 0;
        at(r++, 5);
        printf("%-14s %d", s, val);
    }

    r++;
    if (r - top < h) {
        at(r++, 3);
        printf(A_BOLD "Vitals" A_RESET A_BG_BLK A_BGREEN);
        at(r++, 3);
        fputs("──────────────────────────", stdout);
        at(r++, 5); printf("Health    %d / %d", p->health, p->maxHealth);
        at(r++, 5); printf("Level     %d", p->level);
        at(r++, 5); printf("XP        %d / %d", p->xp, p->level * 100);
        at(r++, 5); printf("Caps      %d", p->caps);
    }

    r++;
    if (r - top < h && !p->perks.empty()) {
        at(r++, 3); printf(A_BOLD "Perks" A_RESET A_BG_BLK A_BGREEN);
        at(r++, 3); fputs("──────────────────────────", stdout);
        for (const auto& perk : p->perks) {
            if (r - top >= h) break;
            at(r++, 5); printf("* %s", perk.c_str());
        }
    }
}

// ── INV tab ───────────────────────────────────────────────────────────────────
void Pipboy::drawInv(int top, int h, int w) {
    int r = top;
    at(r++, 3);
    printf(A_BOLD "Inventory" A_RESET A_BG_BLK A_BGREEN
           "                              "
           A_DIM "  (ENTER to equip/unequip armor)" A_RESET A_BG_BLK A_BGREEN);
    at(r++, 3); fputs("──────────────────────────────────────────────────", stdout);

    if (p->inventory.empty()) {
        at(r, 5); fputs("Nothing in your pack.", stdout);
        return;
    }

    at(r++, 3);
    printf(A_DIM "  %-28s  %s" A_RESET A_BG_BLK A_BGREEN, "Item", "Description");
    at(r++, 3); fputs("──────────────────────────────────────────────────", stdout);

    int idx = 0;
    int maxDesc = w - 36;
    if (maxDesc < 10) maxDesc = 10;

    for (auto& [name, item] : p->inventory) {
        if (r - top >= h) break;
        bool isArmor = (name == "power armor helmet" || name == "leather armor");
        bool equipped = (p->equippedArmor == name);
        at(r++, 3);
        if (isArmor && idx == selectedRow) {
            printf(A_BLACK A_BG_GRN);
        }
        std::string prefix = equipped ? "[E] " : "    ";
        std::string desc = item.description;
        if ((int)desc.size() > maxDesc) desc = desc.substr(0, maxDesc - 3) + "...";
        printf("%s%-28s  %s", prefix.c_str(), name.c_str(), desc.c_str());
        if (isArmor && idx == selectedRow) {
            printf(A_RESET A_BG_BLK A_BGREEN);
        }
        if (isArmor) idx++;
    }
}

// ── DATA tab ──────────────────────────────────────────────────────────────────
void Pipboy::drawData(int top, int h, int w) {
    int r = top;
    at(r++, 3); printf(A_BOLD "Quests" A_RESET A_BG_BLK A_BGREEN);
    at(r++, 3); fputs("──────────────────────────────────────────", stdout);

    auto flag = [&](const std::string& key) -> bool {
        auto it = flags->find(key);
        return it != flags->end() && it->second;
    };

    auto line = [&](bool done, const char* text) {
        if (r - top >= h) return;
        at(r++, 5);
        if (done) printf(A_DIM "[DONE] %s" A_RESET A_BG_BLK A_BGREEN, text);
        else      printf("[ ... ] %s", text);
    };

    // Bartender quest
    if (p->hasMission || flag("bartenderDone")) {
        line(flag("bartenderDone"), "Bartender: find the laser pistol");
    }
    // Scavenger quest
    if (flag("scavengerMissionAssigned")) {
        line(flag("scavengerMissionComplete"), "Scavenger: retrieve fusion core from Red Rocket");
    }
    // BoS quest
    if (flag("bosMissionAssigned")) {
        line(flag("flightRecorderDelivered") && flag("bosReputation"),
             "Brotherhood: recover Vertibird flight recorder");
    }
    // Enclave quest
    if (flag("enclaveMissionAssigned")) {
        line(flag("flightRecorderDelivered") && flag("enclaveReputation"),
             "Enclave: steal the flight recorder");
    }
    // Railroad quest
    if (flag("railroadMissionAssigned")) {
        line(flag("railroadMissionComplete"), "Railroad: steal Enclave decryption key");
    }
    // Rosie quest
    if (flag("rosieMissionAssigned")) {
        line(flag("rosieMissionComplete"), "Rosie: collect 3 pretty bent tin cans");
    }

    r++;
    if (r - top < h) {
        at(r++, 3); printf(A_BOLD "Faction Standing" A_RESET A_BG_BLK A_BGREEN);
        at(r++, 3); fputs("──────────────────────────────────────────", stdout);
        at(r++, 5); printf("Brotherhood of Steel  %s", flag("bosReputation")     ? "Allied" : "Neutral");
        at(r++, 5); printf("Enclave               %s", flag("enclaveReputation") ? "Allied" : "Neutral");
        at(r++, 5); printf("The Railroad          %s", flag("railroadReputation")? "Allied" : "Neutral");
    }

    r++;
    if (r - top < h) {
        at(r++, 3); printf(A_BOLD "Notes" A_RESET A_BG_BLK A_BGREEN);
        at(r++, 3); fputs("──────────────────────────────────────────", stdout);
        if (flag("bridgeCollapsed")) {
            if (r - top < h) { at(r++, 5); fputs("The old river bridge has collapsed.", stdout); }
        }
        if (flag("crateOpened")) {
            if (r - top < h) { at(r++, 5); fputs("The shed crate has been looted.", stdout); }
        }
    }
}

// ── MAP tab ───────────────────────────────────────────────────────────────────

struct MapNode {
    const char* id;
    int gc, gr;   // grid col, grid row
    const char* label; // exactly 5 chars
};

static const MapNode MAP_NODES[] = {
    {"RED_ROCKET_BACKROOM", 2, 0, "RR-BK"},
    {"CASINO_BAR",          6, 0, "C.BAR"},
    {"RIVERBOAT_CASINO",    7, 0, "CASIN"},
    {"ENCLAVE_CAMP",        1, 1, "ENCLV"},
    {"RED_ROCKET",          2, 1, "R.RKT"},
    {"ABANDONED_OUTPOST",   4, 1, "OUTPT"},
    {"SCAVENGER_DEN",       5, 1, "SCAVG"},
    {"SHED",                2, 2, "SHED "},
    {"BOS_BUNKER",          4, 2, "BOS  "},
    {"GANGWAY",             7, 2, "GNGWY"},
    {"ABANDONED_MANSION",   0, 3, "MANSN"},
    {"CRASHED_VERTIBIRD",   1, 3, "CRASH"},
    {"WASTELAND_1",         2, 3, "WSTE1"},
    {"SCORCHED_FARM",       4, 3, "SFARM"},
    {"RIVER_BANK",          7, 3, "RBANK"},
    {"LILLYPAD_ISLAND",     8, 3, "LILY "},
    {"VAULT_ENTRANCE",      2, 4, "VAULT"},
    {"RAILROAD_HIDEOUT",    4, 4, "RAILR"},
    {"UNDER_BRIDGE",        7, 4, "UNDRB"},
    {nullptr, 0, 0, nullptr}
};

struct MapEdge { int gc1,gr1,gc2,gr2; };
static const MapEdge MAP_EDGES[] = {
    {2,0, 2,1},  // RR-BK  – R.RKT
    {6,0, 7,0},  // C.BAR  – CASIN
    {7,0, 7,2},  // CASIN  – GNGWY (span 2 rows)
    {2,1, 2,2},  // R.RKT  – SHED
    {2,1, 4,1},  // R.RKT  – OUTPT (span 2 cols)
    {4,1, 5,1},  // OUTPT  – SCAVG
    {4,1, 4,2},  // OUTPT  – BOS
    {1,1, 1,3},  // ENCLV  – CRASH (span 2 rows)
    {2,2, 2,3},  // SHED   – WSTE1
    {7,2, 7,3},  // GNGWY  – RBANK
    {0,3, 1,3},  // MANSN  – CRASH
    {1,3, 2,3},  // CRASH  – WSTE1
    {2,3, 4,3},  // WSTE1  – SFARM (span 2 cols)
    {4,3, 7,3},  // SFARM  – RBANK (span 3 cols)
    {7,3, 8,3},  // RBANK  – LILY
    {2,3, 2,4},  // WSTE1  – VAULT
    {4,3, 4,4},  // SFARM  – RAILR
    {7,3, 7,4},  // RBANK  – UNDRB
    {-1,-1,-1,-1}
};

void Pipboy::drawMap(int top, int h, int w) {
    // Cell: "[XXXXX]" = 7 chars.  h_gap = 1 char.  Per col = 8.
    // Cell row = 1 line.         v_gap = 1 row.    Per row = 2.
    const int CELL_W = 7;
    const int H_GAP  = 1;
    const int V_GAP  = 1;
    const int COL_STEP = CELL_W + H_GAP;   // 8
    const int ROW_STEP = 1 + V_GAP;         // 2

    int mapLeft = 3;  // absolute col in terminal
    int mapTop  = top;

    // Legend
    at(mapTop, mapLeft);
    printf(A_BOLD "MAP" A_RESET A_BG_BLK A_BGREEN
           A_DIM "  ★ current location  [    ] visited  fog = unvisited" A_RESET A_BG_BLK A_BGREEN);
    at(mapTop + 1, mapLeft);
    fputs("──────────────────────────────────────────────────────────", stdout);

    int gridTop = mapTop + 2;

    auto screenCol = [&](int gc) { return mapLeft + gc * COL_STEP; };
    auto screenRow = [&](int gr) { return gridTop + gr * ROW_STEP; };

    auto visited = [&](const char* id) -> bool {
        return p->visitedRooms.count(id) > 0;
    };

    // Draw edges first (so nodes draw over them)
    for (int e = 0; MAP_EDGES[e].gc1 >= 0; e++) {
        const MapEdge& ed = MAP_EDGES[e];
        // find the two node IDs
        const char* id1 = nullptr, *id2 = nullptr;
        for (int n = 0; MAP_NODES[n].id; n++) {
            if (MAP_NODES[n].gc == ed.gc1 && MAP_NODES[n].gr == ed.gr1) id1 = MAP_NODES[n].id;
            if (MAP_NODES[n].gc == ed.gc2 && MAP_NODES[n].gr == ed.gr2) id2 = MAP_NODES[n].id;
        }
        if (!id1 || !id2) continue;
        if (!visited(id1) || !visited(id2)) continue;

        if (ed.gr1 == ed.gr2) {
            // Horizontal edge
            int sr = screenRow(ed.gr1);
            int sc_start = screenCol(ed.gc1) + CELL_W; // after ']'
            int sc_end   = screenCol(ed.gc2) - 1;       // before '['
            hline(sr, sc_start, sc_end - sc_start + 1, "─");
        } else {
            // Vertical edge
            int sc = screenCol(ed.gc1) + CELL_W / 2;
            int sr_start = screenRow(ed.gr1) + 1;
            int sr_end   = screenRow(ed.gr2) - 1;
            for (int r = sr_start; r <= sr_end; r++) {
                at(r, sc); fputs("│", stdout);
            }
        }
    }

    // Draw nodes
    for (int n = 0; MAP_NODES[n].id; n++) {
        const MapNode& nd = MAP_NODES[n];
        int sr = screenRow(nd.gr);
        int sc = screenCol(nd.gc);
        if (sr >= top + h) continue; // off screen

        bool vis = visited(nd.id);
        bool cur = (p->currentRoomId == nd.id);

        at(sr, sc);
        if (!vis) {
            // Fog of war: invisible
            fputs("       ", stdout); // blank 7 chars
        } else if (cur) {
            printf(A_BOLD A_BLACK A_BG_GRN "[%s]" A_RESET A_BG_BLK A_BGREEN, nd.label);
        } else {
            printf("[%s]", nd.label);
        }
    }
}
