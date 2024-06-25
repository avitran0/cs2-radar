#include <stdarg.h>
#include <unistd.h>

#include <chrono>
#include <format>
#include <thread>

#include "config.h"
#include "cs2.h"
#include "log.h"
#include "memory.h"

// player to json string
std::string player_to_string(Player player) {
    return std::format(
        "{{"
        "\"name\": \"{}\","
        "\"color\": {},"
        "\"health\": {},"
        "\"armor\": {},"
        "\"money\": {},"
        "\"team\": {},"
        "\"life_state\": {},"
        "\"weapon\": \"{}\","
        "\"position\": {{\"x\": {}, \"y\": {}, \"z\": {}}},"
        "\"active_player\": {}"
        "}}",
        player.name.c_str(), player.color, player.health, player.armor,
        player.money, player.team, player.life_state, player.weapon.c_str(),
        player.position.x, player.position.y, player.position.z,
        player.active_player);
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
    while (true) {
        if (!get_pid(PROCESS_NAME).has_value()) {
            // log("waiting for %s to start...", PROCESS_NAME);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        log("pid: %d", get_pid(PROCESS_NAME).value());
        if (!open_process(PROCESS_NAME).has_value()) {
            // log("waiting for %s to start...", PROCESS_NAME);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        auto process = open_process(PROCESS_NAME).value();
        log("process found, finding offsets");
        auto offsets_opt = init(&process);
        if (!offsets_opt.has_value()) {
            log("failed to initialize offsets");
            process.discard();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        auto offsets = offsets_opt.value();

        log("offsets found");

        while (true) {
            if (!validate_pid(process.pid)) {
                log("game closed");
                process.discard();
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
    }

    return 0;
}
