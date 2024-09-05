#include <chrono>
#include <format>
#include <thread>

#include "config.h"
#include "constants.h"
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
        player.name, player.color, player.health, player.armor,
        player.money, player.team, player.life_state, player.weapon,
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
        const auto pid = get_pid(PROCESS_NAME);
        if (!pid.has_value()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        log("pid: %d", pid.value());
        const auto proc = open_process(pid.value());
        if (!proc.has_value()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        auto process = proc.value();
        log("process found, finding offsets");
        auto offsets_opt = init(&process);
        if (!offsets_opt.has_value()) {
            log("failed to initialize offsets");
            process.discard();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        const auto offsets = offsets_opt.value();

        log("offsets found");

        while (true) {
            if (!validate_pid(process.pid)) {
                log("game closed");
                process.discard();
                break;
            }
            auto players = run(&process, &offsets);
            auto players_str = serialize_players(players);

            error("%s", players_str.c_str());

            std::this_thread::sleep_for(
                std::chrono::milliseconds(REFRESH_INTERVAL));
        }
    }

    return 0;
}
