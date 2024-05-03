#include <stdarg.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <thread>

#include "characters.h"
#include "config.h"
#include "cs2.h"
#include "log.h"
#include "memory.h"

std::string format_string(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}

// player to json string
std::string player_to_string(Player player) {
    return format_string(
        "{"
        "\"name\": \"%s\","
        "\"color\": %d,"
        "\"health\": %d,"
        "\"armor\": %d,"
        "\"money\": %d,"
        "\"team\": %d,"
        "\"life_state\": %d,"
        "\"weapon\": \"%s\","
        "\"position\": {\"x\": %f, \"y\": %f, \"z\": %f},"
        "\"local_player\": %d"
        "}",
        player.name.c_str(), player.color, player.health, player.armor,
        player.money, player.team, player.life_state, player.weapon.c_str(),
        player.position.x, player.position.y, player.position.z,
        player.local_player);
}

std::string serialize_players(std::vector<Player> players) {
    std::string result = "[";
    for (size_t i = 0; i < players.size(); i++) {
        result += player_to_string(players[i]);
        if (i < players.size() - 1) {
            result += ",";
        }
    }
    result += "]";
    return result;
}

int main() {
    while (!get_pid(PROCESS_NAME).has_value()) {
        log("Waiting for %s to start...", PROCESS_NAME);
        sleep(1);
    }
    log("pid: %d", get_pid(PROCESS_NAME).value());
    while (!open_process(PROCESS_NAME).has_value()) {
        log("Waiting for %s to start...", PROCESS_NAME);
        sleep(1);
    }
    auto process = open_process(PROCESS_NAME).value();
    auto offsets = init(&process);

    log("game started");

    while (true) {
        if (!validate_pid(process.pid)) {
            log("game closed");
            break;
        }
        auto players = run(&process, &offsets);
        auto players_str = serialize_players(players);

        log_error("%s", players_str.c_str());

        // write to file
        /*std::ofstream file("players.json");
        file << players_str;
        file.close();*/

        std::this_thread::sleep_for(
            std::chrono::milliseconds(REFRESH_INTERVAL));
    }

    return 0;
}
