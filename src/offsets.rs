#[derive(Debug, Default)]
pub struct InterfaceOffsets {
    pub resource: u64,
    pub entity: u64,
    pub cvar: u64,
    pub player: u64,
}

#[derive(Debug, Default)]
pub struct DirectOffsets {
    pub local_player: u64,
}

#[derive(Debug, Default)]
pub struct ConvarOffsets {
    pub teammates_are_enemies: u64,
}

#[derive(Debug, Default)]
pub struct LibraryOffsets {
    pub client: u64,
    pub engine: u64,
    pub tier0: u64,
}

#[derive(Debug, Default)]
pub struct PlayerControllerOffsets {
    pub name: u64,           // string (m_iszPlayerName)
    pub pawn: u64,           // pointer -> Pawn (m_hPawn)
    pub color: u64,          // i32 (m_iCompTeammateColor)
    pub ping: u64,           // i32 (m_iPing)
    pub money_services: u64, // pointer -> MoneyServices (m_pInGameMoneyServices)
    pub steam_id: u64,       // u64 (m_steamID)
}

impl PlayerControllerOffsets {
    pub fn all_found(&self) -> bool {
        self.name != 0
            && self.pawn != 0
            && self.color != 0
            && self.money_services != 0
            && self.steam_id != 0
    }
}

#[derive(Debug, Default)]
pub struct PawnOffsets {
    pub health: u64,            // i32 (m_iHealth)
    pub armor: u64,             // i32 (m_ArmorValue)
    pub team: u64,              // i32 (m_iTeamNum)
    pub life_state: u64,        // i32 (m_lifeState)
    pub weapon: u64,            // pointer -> WeaponBase (m_pClippingWeapon)
    pub position: u64,          // vec3 (m_vOldOrigin)
    pub eye_angles: u64,        // vec3? (m_angEyeAngles)
    pub weapon_services: u64,   // pointer -> WeaponServices (m_pWeaponServices)
    pub observer_services: u64, // pointer -> ObserverServices (m_pObserverServices)
    pub item_services: u64,     // pointer -> ItemServices (m_pItemServices)

    pub flash_max_alpha: u64, // f32 (m_flFlashMaxAlpha)
}

impl PawnOffsets {
    pub fn all_found(&self) -> bool {
        self.health != 0
            && self.armor != 0
            && self.team != 0
            && self.life_state != 0
            && self.weapon != 0
            && self.position != 0
            && self.eye_angles != 0
            && self.weapon_services != 0
            && self.observer_services != 0
            && self.item_services != 0
            && self.flash_max_alpha != 0
    }
}

#[derive(Debug, Default)]
pub struct WeaponServiceOffsets {
    pub active_weapon: u64, // pointer -> Weapon (m_hActiveWeapon)
    pub all_weapons: u64,   // pointer -> Vec<pointer -> Weapon> (m_hMyWeapons)
}

impl WeaponServiceOffsets {
    pub fn all_found(&self) -> bool {
        self.active_weapon != 0 && self.all_weapons != 0
    }
}

#[derive(Debug, Default)]
pub struct MoneyServiceOffsets {
    pub money: u64, // i32 (m_iAccount)
}

impl MoneyServiceOffsets {
    pub fn all_found(&self) -> bool {
        self.money != 0
    }
}

#[derive(Debug, Default)]
pub struct ObserverServiceOffsets {
    pub target: u64, // pointer -> Pawn (m_hObserverTarget)
}

impl ObserverServiceOffsets {
    pub fn all_found(&self) -> bool {
        self.target != 0
    }
}

#[derive(Debug, Default)]
pub struct ItemServiceOffsets {
    pub has_defuser: u64, // bool (m_bHasDefuser)
    pub has_helmet: u64,  // bool (m_bHasHelmet)
}

impl ItemServiceOffsets {
    pub fn all_found(&self) -> bool {
        self.has_defuser != 0 && self.has_helmet != 0
    }
}

#[derive(Debug, Default)]
pub struct Offsets {
    pub interface: InterfaceOffsets,
    pub direct: DirectOffsets,
    pub convar: ConvarOffsets,
    pub library: LibraryOffsets,
    pub controller: PlayerControllerOffsets,
    pub pawn: PawnOffsets,
    pub weapon_service: WeaponServiceOffsets,
    pub money_service: MoneyServiceOffsets,
    pub observer_service: ObserverServiceOffsets,
    pub item_service: ItemServiceOffsets,
}

impl Offsets {
    pub fn all_found(&self) -> bool {
        self.controller.all_found()
            && self.pawn.all_found()
            && self.weapon_service.all_found()
            && self.money_service.all_found()
            && self.observer_service.all_found()
            && self.item_service.all_found()
    }
}
