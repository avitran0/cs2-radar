#ifndef CS2_RADAR_CS2
#define CS2_RADAR_CS2

#include "math.h"
#include "memory.h"

/*
Colors:
0: Blue
1: Green
2: Yellow
3: Orange
4: Purple
5+: White
*/

enum Team {
    TEAM_NONE = 0,
    TEAM_SPECTATOR = 1,
    TEAM_T = 2,
    TEAM_CT = 3,
};

struct InterfaceOffsets {
    u64 resource;
    u64 entity;
    u64 cvar;
    u64 player;
};

struct DirectOffsets {
    u64 local_player;
};

struct ConvarOffsets {
    u64 teammates_are_enemies;
    u64 crosshair_alpha;
};

struct LibraryOffsets {
    u64 client;
    u64 engine;
    u64 tier0;
};

struct PlayerControllerOffsets {
    u64 pawn;            // pointer m_hPawn
    u64 name;            // string m_sSanitizedPlayerName
    u64 color;           // i32 m_iCompTeammateColor
    u64 money_services;  // pointer m_pInGameMoneyServices
};

struct PawnOffsets {
    u64 health;             // i32 m_iHealth
    u64 armor;              // i32 m_ArmorValue
    u64 team;               // u8 m_iTeamNum
    u64 life_state;         // u8 m_lifeState
    u64 weapon;             // pointer m_pClippingWeapon
    u64 bullet_services;    // pointer m_pBulletServices
    u64 weapon_services;    // pointer m_pWeaponServices
    u64 position;           // Vec3 m_vOldOrigin
    u64 observer_services;  // pointer m_pObserverServices
};

struct MoneyServicesOffsets {
    u64 money;  // i32 m_iAccount
};

struct BulletServicesOffsets {
    u64 total_hits;  // i32 m_totalHitsOnServer
};

struct WeaponServicesOffsets {
    u64 my_weapons;     // vector m_hMyWeapons (length at 0x00, data at 0x08)
    u64 active_weapon;  // pointer m_hActiveWeapon
};

struct ObserverServicesOffsets {
    u64 target;  // pointer m_hObserverTarget
};

struct Offsets {
    InterfaceOffsets interfaces;
    DirectOffsets direct;
    ConvarOffsets convars;
    LibraryOffsets libraries;
    PlayerControllerOffsets controller;
    PawnOffsets pawn;
    MoneyServicesOffsets money_services;
    BulletServicesOffsets bullet_services;
    WeaponServicesOffsets weapon_services;
    ObserverServicesOffsets observer_services;
};

struct Player {
    std::string name;
    i32 health;
    i32 armor;
    i32 money;
    u8 team;
    u8 life_state;
    std::string weapon;
    i32 color;
    Vec3 position;
    bool active_player;
};

std::optional<Offsets> init(ProcessHandle* process);
std::vector<Player> run(ProcessHandle* process, const Offsets* offsets);

#endif
