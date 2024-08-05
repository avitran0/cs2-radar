#include "cs2.h"

#include <string.h>

#include "constants.h"
#include "log.h"
#include "math.h"

bool all_offsets_found(const Offsets *offsets) {
    return offsets->controller.pawn && offsets->controller.name &&
           offsets->controller.color && offsets->controller.money_services &&
           offsets->pawn.health && offsets->pawn.armor && offsets->pawn.team &&
           offsets->pawn.life_state && offsets->pawn.weapon &&
           offsets->pawn.bullet_services && offsets->pawn.weapon_services &&
           offsets->pawn.position && offsets->pawn.observer_services &&
           offsets->money_services.money &&
           offsets->bullet_services.total_hits &&
           offsets->weapon_services.my_weapons &&
           offsets->weapon_services.active_weapon &&
           offsets->observer_services.target;
}

std::optional<Offsets> init(ProcessHandle *process) {
    Offsets offsets = {};

    // get client lib base address
    const auto client_address =
        get_module_base_address(process->pid, CLIENT_LIB);
    const auto engine_address =
        get_module_base_address(process->pid, ENGINE_LIB);
    const auto tier0_address = get_module_base_address(process->pid, TIER0_LIB);

    if (!client_address.has_value()) {
        log("failed to get %s base address", CLIENT_LIB);
        return std::nullopt;
    }
    if (!engine_address.has_value()) {
        log("failed to get %s base address", ENGINE_LIB);
        return std::nullopt;
    }
    if (!tier0_address.has_value()) {
        log("failed to get %s base address", TIER0_LIB);
        return std::nullopt;
    }

    offsets.libraries.client = client_address.value();
    offsets.libraries.engine = engine_address.value();
    offsets.libraries.tier0 = tier0_address.value();

    const auto resource_offset = process->get_interface_offset(
        offsets.libraries.engine, "GameResourceServiceClientV0");
    if (!resource_offset.has_value()) {
        log("failed to get resource offset");
        return std::nullopt;
    }
    offsets.interfaces.resource = resource_offset.value();

    const auto local_player = process->scan_pattern(
        std::vector<u8>{
            0x48,
            0x83,
            0x3D,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x0F,
            0x95,
            0xC0,
            0xC3,
        },
        std::vector<bool>{true, true, true, false, false, false, false, true,
                          true, true, true, true},
        offsets.libraries.client);
    if (!local_player.has_value()) {
        log("failed to get local player offset");
        return std::nullopt;
    }
    offsets.direct.local_player =
        process->get_relative_address(local_player.value(), 0x03, 0x08);

    const auto player_address = offsets.interfaces.resource + ENTITY_OFFSET;
    offsets.interfaces.entity = process->read_u64(player_address);
    offsets.interfaces.player = offsets.interfaces.entity + 0x10;
    const auto cvar_offset =
        process->get_interface_offset(offsets.libraries.tier0, "VEngineCvar0");
    if (!cvar_offset.has_value()) {
        log("failed to get cvar offset");
        return std::nullopt;
    }
    offsets.interfaces.cvar = cvar_offset.value();

    offsets.convars.teammates_are_enemies =
        process->get_convar(offsets.interfaces.cvar, "mp_teammates_are_enemies")
            .value();
    offsets.convars.crosshair_alpha =
        process->get_convar(offsets.interfaces.cvar, "cl_crosshairalpha")
            .value();

    // dump all netvars from client lib
    const auto base = offsets.libraries.client;
    const auto section_header_offset =
        process->read_u64(base + ELF_SECTION_HEADER_OFFSET);
    const auto section_header_entry_size =
        process->read_u16(base + ELF_SECTION_HEADER_ENTRY_SIZE);
    const auto section_header_num_entries =
        process->read_u16(base + ELF_SECTION_HEADER_NUM_ENTRIES);

    const auto size = section_header_offset +
                      (section_header_entry_size * section_header_num_entries);

    const auto client_dump_vec = process->dump_module(base);
    const u8 *client_dump = &client_dump_vec[0];

    for (size_t i = (size - 8); i > 0; i -= 8) {
        // read client dump at i from dump directly
        const auto entry = ((u64)client_dump + i);

        bool network_enable = false;
        auto network_enable_name_pointer = *(u64 *)entry;

        if (network_enable_name_pointer == 0) {
            continue;
        }

        if (network_enable_name_pointer >= base &&
            network_enable_name_pointer <= base + size) {
            network_enable_name_pointer =
                *(u64 *)(network_enable_name_pointer - base + client_dump);
            if (network_enable_name_pointer >= base &&
                network_enable_name_pointer <= base + size) {
                const auto name =
                    (char *)(network_enable_name_pointer - base + client_dump);
                if (!strcmp(name, "MNetworkEnable")) {
                    network_enable = true;
                }
            }
        }

        u64 name_pointer = 0;
        if (network_enable == false) {
            name_pointer = *(u64 *)(entry);
        } else {
            name_pointer = *(u64 *)(entry + 0x08);
        }

        if (name_pointer < base || name_pointer > (base + size)) {
            continue;
        }

        const auto name =
            std::string((char *)(name_pointer - base + client_dump));

        if (name == std::string("m_hPawn")) {
            if (!network_enable || offsets.controller.pawn != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08 + 0x10);
            offsets.controller.pawn = offset;
        } else if (name == std::string("m_sSanitizedPlayerName")) {
            if (!network_enable || offsets.controller.name != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08 + 0x10);
            offsets.controller.name = offset;
        } else if (name == std::string("m_iCompTeammateColor")) {
            if (offsets.controller.color != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x10);
            offsets.controller.color = offset;
        } else if (name == std::string("m_pInGameMoneyServices")) {
            if (offsets.controller.money_services != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x10);
            offsets.controller.money_services = offset;
        } else if (name == std::string("m_iHealth")) {
            if (!network_enable || offsets.pawn.health != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08 + 0x10);
            offsets.pawn.health = offset;
        } else if (name == std::string("m_ArmorValue")) {
            const auto offset = *(i32 *)(entry + 0x08 + 0x10);
            if (offset <= 0 || offset > 20000) {
                continue;
            }
            offsets.pawn.armor = offset;
        } else if (name == std::string("m_iTeamNum")) {
            if (!network_enable || offsets.pawn.team != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08 + 0x10);
            offsets.pawn.team = offset;
        } else if (name == std::string("m_lifeState")) {
            if (!network_enable || offsets.pawn.life_state != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08 + 0x10);
            offsets.pawn.life_state = offset;
        } else if (name == std::string("m_pClippingWeapon")) {
            if (offsets.pawn.weapon != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x10);
            offsets.pawn.weapon = offset;
        } else if (name == std::string("m_pBulletServices")) {
            if (offsets.pawn.bullet_services != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08);
            offsets.pawn.bullet_services = offset;
        } else if (name == std::string("m_pWeaponServices")) {
            if (offsets.pawn.weapon_services != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08);
            offsets.pawn.weapon_services = offset;
        } else if (name == std::string("m_vOldOrigin")) {
            if (offsets.pawn.position != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08);
            offsets.pawn.position = offset;
        } else if (name == std::string("m_pObserverServices")) {
            if (offsets.pawn.observer_services != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08);
            offsets.pawn.observer_services = offset;
        } else if (name == std::string("m_iAccount")) {
            if (offsets.money_services.money != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x10);
            offsets.money_services.money = offset;
        } else if (name == std::string("m_totalHitsOnServer")) {
            if (offsets.bullet_services.total_hits != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08);
            offsets.bullet_services.total_hits = offset;
        } else if (name == std::string("m_hMyWeapons")) {
            if (offsets.weapon_services.my_weapons != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08);
            offsets.weapon_services.my_weapons = offset;
        } else if (name == std::string("m_hActiveWeapon")) {
            if (!network_enable || offsets.weapon_services.active_weapon != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08 + 0x10);
            offsets.weapon_services.active_weapon = offset;
        } else if (name == std::string("m_hObserverTarget")) {
            if (offsets.observer_services.target != 0) {
                continue;
            }
            const auto offset = *(i32 *)(entry + 0x08);
            offsets.observer_services.target = offset;
        }

        if (all_offsets_found(&offsets)) {
            return offsets;
        }
    }

    if (!all_offsets_found(&offsets)) {
        log("did not find all offsets");
        return std::nullopt;
    }
    return offsets;
}

u64 get_local_controller(ProcessHandle *process, const Offsets *offsets) {
    return process->read_u64(offsets->direct.local_player);
}

std::optional<u64> get_client_entity(ProcessHandle *process,
                                     const Offsets *offsets, i32 index) {
    // what the fuck exactly does this to and how?
    const u64 v2 =
        process->read_u64(offsets->interfaces.entity + 8 * (index >> 9) + 16);
    if (v2 == 0) return std::nullopt;

    return process->read_u64((u64)(120 * (index & 0x1FF) + v2));
}

std::optional<u64> get_pawn(ProcessHandle *process, const Offsets *offsets,
                            u64 controller) {
    // what is happening here?
    const i64 v1 = process->read_u32(controller + offsets->controller.pawn);
    if (v1 == -1) {
        return std::nullopt;
    }

    const auto v2 = process->read_u64(offsets->interfaces.player +
                                      8 * (((u64)v1 & 0x7FFF) >> 9));
    if (v2 == 0) {
        return std::nullopt;
    }
    return process->read_u64(v2 + 120 * (v1 & 0x1FF));
}

std::string get_name(ProcessHandle *process, const Offsets *offsets,
                     u64 controller) {
    const auto name = process->read_string(
        process->read_u64(controller + offsets->controller.name));
    return name;
}

i32 get_health(ProcessHandle *process, const Offsets *offsets, u64 pawn) {
    const auto health = process->read_i32(pawn + offsets->pawn.health);
    if (health < 0 || health > 100) {
        return 0;
    }
    return health;
}

i32 get_armor(ProcessHandle *process, const Offsets *offsets, u64 pawn) {
    const auto armor = process->read_i32(pawn + offsets->pawn.armor);
    if (armor < 0 || armor > 100) {
        return 0;
    }
    return armor;
}

i32 get_money(ProcessHandle *process, const Offsets *offsets, u64 controller) {
    const auto money_services =
        process->read_u64(controller + offsets->controller.money_services);
    if (money_services == 0) {
        return 0;
    }
    return process->read_i32(money_services + offsets->money_services.money);
}

u8 get_team(ProcessHandle *process, const Offsets *offsets, u64 pawn) {
    return process->read_u8(pawn + offsets->pawn.team);
}

u8 get_life_state(ProcessHandle *process, const Offsets *offsets, u64 pawn) {
    return process->read_u8(pawn + offsets->pawn.life_state);
}

std::string get_weapon_name(ProcessHandle *process, const Offsets *offsets,
                            u64 weapon_instance) {
    const auto weapon_entity_identity =
        process->read_u64(weapon_instance + 0x10);
    if (weapon_entity_identity == 0) {
        return "unknown";
    }

    const auto weapon_name_pointer =
        process->read_u64(weapon_entity_identity + 0x20);
    if (weapon_name_pointer == 0) {
        return "unknown";
    }

    return process->read_string(weapon_name_pointer);
}

std::string get_weapon(ProcessHandle *process, const Offsets *offsets,
                       u64 pawn) {
    auto weapon_entity_instance =
        process->read_u64(pawn + offsets->pawn.weapon);
    if (weapon_entity_instance == 0) {
        return "unknown";
    }

    return get_weapon_name(process, offsets, weapon_entity_instance);
}

i32 get_total_hits(ProcessHandle *process, const Offsets *offsets, u64 pawn) {
    const auto bullet_services =
        process->read_u64(pawn + offsets->pawn.bullet_services);
    if (bullet_services == 0) {
        return 0;
    }
    return process->read_i32(bullet_services +
                             offsets->bullet_services.total_hits);
}

i32 get_color(ProcessHandle *process, const Offsets *offsets, u64 controller) {
    return process->read_i32(controller + offsets->controller.color);
}

Vec3 get_position(ProcessHandle *process, const Offsets *offsets, u64 pawn) {
    const auto position = pawn + offsets->pawn.position;
    return Vec3{
        process->read_f32(position),
        process->read_f32(position + 0x04),
        process->read_f32(position + 0x08),
    };
}

u64 get_spectator_target(ProcessHandle *process, const Offsets *offsets,
                         u64 pawn) {
    const auto observer_services =
        process->read_u64(pawn + offsets->pawn.observer_services);
    if (observer_services == 0) {
        return 0;
    }
    const auto target = process->read_u32(observer_services +
                                          offsets->observer_services.target) &
                        0x7FFF;
    if (target == 0) {
        return 0;
    }

    const auto v2 =
        process->read_u64(offsets->interfaces.player + 8 * (target >> 9));
    if (v2 == 0) {
        return 0;
    }

    return process->read_u64(v2 + 120 * (target & 0x1FF));
}

bool is_ffa(ProcessHandle *process, const Offsets *offsets) {
    return process->read_i32(offsets->convars.teammates_are_enemies + 0x40) !=
           0;
}

Player get_player_info(ProcessHandle *process, const Offsets *offsets,
                       u64 controller) {
    Player player = {};
    const auto pawn_opt = get_pawn(process, offsets, controller);
    if (!pawn_opt.has_value()) {
        return player;
    }
    const auto pawn = pawn_opt.value();

    player.name = get_name(process, offsets, controller);
    player.health = get_health(process, offsets, pawn);
    player.armor = get_armor(process, offsets, pawn);
    player.money = get_money(process, offsets, controller);
    player.team = get_team(process, offsets, pawn);
    player.life_state = get_life_state(process, offsets, pawn);
    player.weapon = get_weapon(process, offsets, pawn);
    player.color = get_color(process, offsets, controller);
    player.position = get_position(process, offsets, pawn);
    player.active_player = false;

    return player;
}

std::vector<Player> run(ProcessHandle *process, const Offsets *offsets) {
    const auto local_controller = get_local_controller(process, offsets);
    const auto local_pawn_opt = get_pawn(process, offsets, local_controller);
    if (!local_pawn_opt.has_value()) {
        return std::vector<Player>();
    }
    const auto local_pawn = local_pawn_opt.value();
    auto local_player = get_player_info(process, offsets, local_controller);
    const auto spectator_target =
        get_spectator_target(process, offsets, local_pawn);

    auto players = std::vector<Player>();
    for (i32 i = 1; i <= 64; i++) {
        const auto controller_opt = get_client_entity(process, offsets, i);
        if (!controller_opt.has_value()) {
            continue;
        }
        const auto controller = controller_opt.value();
        if (controller == local_controller) {
            continue;
        }

        const auto pawn = get_pawn(process, offsets, controller);

        auto player = get_player_info(process, offsets, controller);
        if (player.team != TEAM_T && player.team != TEAM_CT) {
            continue;
        }
        if (spectator_target == pawn) {
            player.active_player = true;
        }
        players.push_back(player);
    }
    if (!spectator_target) {
        local_player.active_player = true;
    }
    if (local_player.team == TEAM_T || local_player.team == TEAM_CT) {
        players.push_back(local_player);
    }
    return players;
}
