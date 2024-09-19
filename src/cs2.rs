use serde::Serialize;

use crate::{
    constants::{
        CLIENT_LIB, ENGINE_LIB, ENTITY_OFFSET, TEAM_CT, TEAM_T, TIER0_LIB, WEAPON_UNKNOWN,
    },
    math::Vec3,
    memory::{get_module_base_address, read_string_vec, read_u32_vec, read_u64_vec},
    offsets::Offsets,
    process_handle::ProcessHandle,
};

#[derive(Clone, Debug, Default, Serialize)]
pub struct Player {
    pub name: String,
    pub health: i32,
    pub armor: i32,
    pub money: i32,
    pub team: u8,
    pub life_state: u8,
    pub weapon: String,
    pub weapons: Vec<String>,
    pub has_defuser: bool,
    pub has_helmet: bool,
    pub color: i32,
    pub position: Vec3,
    pub rotation: f32,
    pub ping: i32,
    pub steam_id: u64,
    pub active_player: bool,
}

pub fn find_offsets(process: &ProcessHandle) -> Option<Offsets> {
    let mut offsets = Offsets::default();

    let client_address = get_module_base_address(process, CLIENT_LIB);
    offsets.library.client = client_address?;

    let engine_address = get_module_base_address(process, ENGINE_LIB);
    offsets.library.engine = engine_address?;

    let tier0_address = get_module_base_address(process, TIER0_LIB);
    offsets.library.tier0 = tier0_address?;

    let resource_offset =
        process.get_interface_offset(offsets.library.engine, "GameResourceServiceClientV0");
    offsets.interface.resource = resource_offset?;

    // seems to be in .text section (executable instructions)
    let local_player = process.scan_pattern(
        &[
            0x48, 0x83, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x95, 0xC0, 0xC3,
        ],
        "xxx????xxxxx".as_bytes(),
        offsets.library.client,
    );
    offsets.direct.local_player = process.get_relative_address(local_player?, 0x03, 0x08);

    // global vars
    /*let global_vars = process.scan_pattern(
        &[0x48, 0x89, 0x35, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x46],
        "xxx????xxx".as_bytes(),
        offsets.library.client,
    );
    let global_vars = process.get_relative_address(global_vars?, 0x03, 0x07);*/

    let player_address = offsets.interface.resource + ENTITY_OFFSET;
    offsets.interface.entity = process.read_u64(player_address);
    offsets.interface.player = offsets.interface.entity + 0x10;

    let cvar_offset = process.get_interface_offset(offsets.library.tier0, "VEngineCvar0");
    offsets.interface.cvar = cvar_offset?;

    offsets.convar.teammates_are_enemies =
        process.get_convar(&offsets, "mp_teammates_are_enemies")?;

    let client_module_size = process.module_size(offsets.library.client);
    let client_dump = process.dump_module(offsets.library.client);

    let base = offsets.library.client;
    for i in (0..=(client_module_size - 8)).rev().step_by(8) {
        let mut network_enable = false;

        let mut name_pointer = read_u64_vec(&client_dump, i);
        if name_pointer >= base && name_pointer <= base + client_module_size {
            name_pointer = read_u64_vec(&client_dump, name_pointer - base);
            if name_pointer >= base && name_pointer <= base + client_module_size {
                let name = read_string_vec(&client_dump, name_pointer - base);
                if name.to_lowercase() == "MNetworkEnable".to_lowercase() {
                    network_enable = true;
                }
            }
        }

        let name_ptr = match network_enable {
            true => read_u64_vec(&client_dump, i + 0x08),
            false => read_u64_vec(&client_dump, i),
        };

        if name_ptr < base || name_ptr > base + client_module_size {
            continue;
        }

        let netvar_name = read_string_vec(&client_dump, name_ptr - base);

        match netvar_name.as_str() {
            "m_sSanitizedPlayerName" => {
                if !network_enable || offsets.controller.name != 0 {
                    continue;
                }
                offsets.controller.name = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            "m_hPawn" => {
                if !network_enable || offsets.controller.pawn != 0 {
                    continue;
                }
                offsets.controller.pawn = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            "m_iCompTeammateColor" => {
                if offsets.controller.color != 0 {
                    continue;
                }
                offsets.controller.color = read_u32_vec(&client_dump, i + 0x10) as u64;
            }
            "m_iPing" => {
                if !network_enable || offsets.controller.ping != 0 {
                    continue;
                }
                offsets.controller.ping = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            "m_pInGameMoneyServices" => {
                if offsets.controller.money_services != 0 {
                    continue;
                }
                offsets.controller.money_services = read_u32_vec(&client_dump, i + 0x10) as u64;
            }
            "m_steamID" => {
                if !network_enable || offsets.controller.steam_id != 0 {
                    continue;
                }
                offsets.controller.steam_id = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            "m_iHealth" => {
                if !network_enable || offsets.pawn.health != 0 {
                    continue;
                }
                offsets.pawn.health = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            "m_ArmorValue" => {
                if !network_enable || offsets.pawn.armor != 0 {
                    continue;
                }
                offsets.pawn.armor = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            "m_iTeamNum" => {
                if !network_enable || offsets.pawn.team != 0 {
                    continue;
                }
                offsets.pawn.team = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            "m_lifeState" => {
                if !network_enable || offsets.pawn.life_state != 0 {
                    continue;
                }
                offsets.pawn.life_state = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            "m_pClippingWeapon" => {
                if offsets.pawn.weapon != 0 {
                    continue;
                }
                offsets.pawn.weapon = read_u32_vec(&client_dump, i + 0x10) as u64;
            }
            "m_vOldOrigin" => {
                if offsets.pawn.position != 0 {
                    continue;
                }
                offsets.pawn.position = read_u32_vec(&client_dump, i + 0x08) as u64;
            }
            "m_angEyeAngles" => {
                if offsets.pawn.eye_angles != 0 {
                    continue;
                }
                offsets.pawn.eye_angles = read_u32_vec(&client_dump, i + 0x10) as u64;
            }
            "m_pWeaponServices" => {
                if offsets.pawn.weapon_services != 0 {
                    continue;
                }
                offsets.pawn.weapon_services = read_u32_vec(&client_dump, i + 0x08) as u64;
            }
            "m_pObserverServices" => {
                if offsets.pawn.observer_services != 0 {
                    continue;
                }
                offsets.pawn.observer_services = read_u32_vec(&client_dump, i + 0x08) as u64;
            }
            "m_pItemServices" => {
                if offsets.pawn.item_services != 0 {
                    continue;
                }
                offsets.pawn.item_services = read_u32_vec(&client_dump, i + 0x08) as u64;
            }
            "m_hActiveWeapon" => {
                if !network_enable || offsets.weapon_service.active_weapon != 0 {
                    continue;
                }
                offsets.weapon_service.active_weapon =
                    read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            "m_hMyWeapons" => {
                if offsets.weapon_service.all_weapons != 0 {
                    continue;
                }
                offsets.weapon_service.all_weapons = read_u32_vec(&client_dump, i + 0x08) as u64;
            }
            "m_iAccount" => {
                if offsets.money_service.money != 0 {
                    continue;
                }
                offsets.money_service.money = read_u32_vec(&client_dump, i + 0x10) as u64;
            }
            "m_hObserverTarget" => {
                if offsets.observer_service.target != 0 {
                    continue;
                }
                offsets.observer_service.target = read_u32_vec(&client_dump, i + 0x08) as u64;
            }
            "m_bHasDefuser" => {
                if offsets.item_service.has_defuser != 0 {
                    continue;
                }
                offsets.item_service.has_defuser = read_u32_vec(&client_dump, i + 0x10) as u64;
            }
            "m_bHasHelmet" => {
                if !network_enable || offsets.item_service.has_helmet != 0 {
                    continue;
                }
                offsets.item_service.has_helmet =
                    read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
            }
            _ => {}
        }

        if offsets.all_found() {
            return Some(offsets);
        }
    }

    None
}

fn get_local_controller(process: &ProcessHandle, offsets: &Offsets) -> u64 {
    process.read_u64(offsets.direct.local_player)
}

fn get_client_entity(process: &ProcessHandle, offsets: &Offsets, index: u64) -> Option<u64> {
    // wtf is this doing, and how?
    let v1 = process.read_u64(offsets.interface.entity + 0x08 * (index >> 9) + 0x10);
    if v1 == 0 {
        return None;
    }
    // what?
    let entity = process.read_u64(v1 + 120 * (index & 0x1ff));
    if entity == 0 {
        return None;
    }
    Some(entity)
}

fn get_pawn(process: &ProcessHandle, offsets: &Offsets, controller: u64) -> Option<u64> {
    let v1 = process.read_i32(controller + offsets.controller.pawn);
    if v1 == -1 {
        return None;
    }

    // what the fuck is this doing?
    let v2 = process.read_u64(offsets.interface.player + 8 * ((v1 as u64 & 0x7fff) >> 9));
    if v2 == 0 {
        return None;
    }

    // bit-fuckery, why is this needed exactly?
    let entity = process.read_u64(v2 + 120 * (v1 as u64 & 0x1ff));
    if entity == 0 {
        return None;
    }
    Some(entity)
}

fn get_name(process: &ProcessHandle, offsets: &Offsets, controller: u64) -> String {
    // simple string pointer
    let name_pointer = process.read_u64(controller + offsets.controller.name);
    if name_pointer == 0 {
        return String::from("?");
    }
    process.read_string(name_pointer)
}

fn get_health(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> i32 {
    let health = process.read_i32(pawn + offsets.pawn.health);
    if !(0..=100).contains(&health) {
        return 0;
    }
    health
}

fn get_armor(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> i32 {
    let armor = process.read_i32(pawn + offsets.pawn.armor);
    if !(0..=100).contains(&armor) {
        return 0;
    }
    armor
}

fn get_money(process: &ProcessHandle, offsets: &Offsets, controller: u64) -> i32 {
    let money_services = process.read_u64(controller + offsets.controller.money_services);
    if money_services == 0 {
        return 0;
    }
    let money = process.read_i32(money_services + offsets.money_service.money);
    if !(0..=99999).contains(&money) {
        return 0;
    }
    money
}

fn get_team(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> u8 {
    process.read_u8(pawn + offsets.pawn.team)
}

fn get_life_state(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> u8 {
    process.read_u8(pawn + offsets.pawn.life_state)
}

fn get_weapon(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> String {
    // CEntityInstance
    let weapon_entity_instance = process.read_u64(pawn + offsets.pawn.weapon);
    if weapon_entity_instance == 0 {
        return String::from("unknown");
    }
    get_weapon_name(process, weapon_entity_instance)
}

fn get_weapon_name(process: &ProcessHandle, weapon_instance: u64) -> String {
    // CEntityIdentity, 0x10 = m_pEntity
    let weapon_entity_identity = process.read_u64(weapon_instance + 0x10);
    if weapon_entity_identity == 0 {
        return String::from(WEAPON_UNKNOWN);
    }
    // 0x20 = m_designerName (pointer -> string)
    let weapon_name_pointer = process.read_u64(weapon_entity_identity + 0x20);
    if weapon_name_pointer == 0 {
        return String::from(WEAPON_UNKNOWN);
    }
    process.read_string(weapon_name_pointer)
}

fn get_weapons(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> Vec<String> {
    let weapon_services = process.read_u64(pawn + offsets.pawn.weapon_services);
    if weapon_services == 0 {
        return vec![];
    }

    // 8 bytes size, 8 bytes pointer to data
    let size = process.read_u64(weapon_services + offsets.weapon_service.all_weapons);
    let weapon_vector =
        process.read_u64(weapon_services + offsets.weapon_service.all_weapons + 0x08);

    let mut weapon_names = vec![];
    for i in 0..size {
        // weird bit-fuckery, why exactly does it need the & 0xfff?
        let weapon_index = process.read_u32(weapon_vector + i * 0x04) & 0xfff;
        let weapon_entity = get_client_entity(process, offsets, weapon_index as u64);
        if let Some(entity) = weapon_entity {
            let weapon_name = get_weapon_name(process, entity);
            weapon_names.push(weapon_name);
        }
    }

    weapon_names
}

fn get_defuser(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> bool {
    let item_services = process.read_u64(pawn + offsets.pawn.item_services);
    if item_services == 0 {
        return false;
    }

    process.read_u8(item_services + offsets.item_service.has_defuser) != 0
}

fn get_helmet(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> bool {
    let item_services = process.read_u64(pawn + offsets.pawn.item_services);
    if item_services == 0 {
        return false;
    }

    process.read_u8(item_services + offsets.item_service.has_helmet) != 0
}

fn get_color(process: &ProcessHandle, offsets: &Offsets, controller: u64) -> i32 {
    process.read_i32(controller + offsets.controller.color)
}

fn get_position(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> Vec3 {
    // 3 32-bit floats
    let position = pawn + offsets.pawn.position;
    Vec3 {
        x: process.read_f32(position),
        y: process.read_f32(position + 0x04),
        z: process.read_f32(position + 0x08),
    }
}

fn get_rotation(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> f32 {
    process.read_f32(pawn + offsets.pawn.eye_angles + 0x04)
}

fn get_ping(process: &ProcessHandle, offsets: &Offsets, controller: u64) -> i32 {
    process.read_i32(controller + offsets.controller.ping)
}

fn get_steam_id(process: &ProcessHandle, offsets: &Offsets, controller: u64) -> u64 {
    process.read_u64(controller + offsets.controller.steam_id)
}

/// returns spectated pawn
fn get_spectator_target(process: &ProcessHandle, offsets: &Offsets, pawn: u64) -> Option<u64> {
    let observer_services = process.read_u64(pawn + offsets.pawn.observer_services);
    if observer_services == 0 {
        return None;
    }

    let target = process.read_u32(observer_services + offsets.observer_service.target) & 0x7fff;
    if target == 0 {
        return None;
    }

    let v2 = process.read_u64(offsets.interface.player + 8 * (target as u64 >> 9));
    if v2 == 0 {
        return None;
    }

    let entity = process.read_u64(v2 + 120 * (target as u64 & 0x1ff));
    if entity == 0 {
        return None;
    }
    Some(entity)
}

fn get_player(process: &ProcessHandle, offsets: &Offsets, controller: u64) -> Option<Player> {
    let mut player = Player::default();
    let pawn = get_pawn(process, offsets, controller)?;

    let team = get_team(process, offsets, pawn);
    if !(1..=3).contains(&team) {
        return None;
    }

    player.name = get_name(process, offsets, controller);
    player.health = get_health(process, offsets, pawn);
    player.armor = get_armor(process, offsets, pawn);
    player.money = get_money(process, offsets, controller);
    player.team = team;
    player.life_state = get_life_state(process, offsets, pawn);
    player.weapon = get_weapon(process, offsets, pawn);
    player.weapons = get_weapons(process, offsets, pawn);
    player.has_defuser = get_defuser(process, offsets, pawn);
    player.has_helmet = get_helmet(process, offsets, pawn);
    player.color = get_color(process, offsets, controller);
    player.position = get_position(process, offsets, pawn);
    player.rotation = get_rotation(process, offsets, pawn);
    player.ping = get_ping(process, offsets, controller);
    player.steam_id = get_steam_id(process, offsets, controller);

    Some(player)
}

pub fn get_player_info(process: &ProcessHandle, offsets: &Offsets) -> Vec<Player> {
    let local_controller = get_local_controller(process, offsets);
    let local_pawn = match get_pawn(process, offsets, local_controller) {
        Some(pawn) => pawn,
        None => {
            return vec![];
        }
    };

    let mut local_player = match get_player(process, offsets, local_controller) {
        Some(player) => player,
        None => {
            return vec![];
        }
    };
    let spectator_target = get_spectator_target(process, offsets, local_pawn);

    let mut players = vec![];
    for i in 1..=64 {
        let controller = match get_client_entity(process, offsets, i) {
            Some(controller) => controller,
            None => {
                continue;
            }
        };

        if controller == local_controller {
            continue;
        }

        let pawn = match get_pawn(process, offsets, controller) {
            Some(pawn) => pawn,
            None => {
                continue;
            }
        };

        let mut player = match get_player(process, offsets, controller) {
            Some(player) => player,
            None => {
                continue;
            }
        };
        if player.team != TEAM_T && player.team != TEAM_CT {
            continue;
        }

        if spectator_target.is_some_and(|target| pawn == target) {
            player.active_player = true;
        }

        players.push(player);
    }

    if spectator_target.is_none() {
        local_player.active_player = true;
    }
    if local_player.team == TEAM_T || local_player.team == TEAM_CT {
        players.push(local_player);
    }

    players
}
