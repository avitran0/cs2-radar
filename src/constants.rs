pub const PROCESS_NAME: &str = "cs2";
pub const CLIENT_LIB: &str = "libclient.so";
pub const ENGINE_LIB: &str = "libengine2.so";
pub const TIER0_LIB: &str = "libtier0.so";

pub const ENTITY_OFFSET: u64 = 0x50;

pub const ELF_PROGRAM_HEADER_OFFSET: u64 = 0x20;
pub const ELF_PROGRAM_HEADER_ENTRY_SIZE: u64 = 0x36;
pub const ELF_PROGRAM_HEADER_NUM_ENTRIES: u64 = 0x38;

pub const ELF_SECTION_HEADER_OFFSET: u64 = 0x28;
pub const ELF_SECTION_HEADER_ENTRY_SIZE: u64 = 0x3A;
pub const ELF_SECTION_HEADER_NUM_ENTRIES: u64 = 0x3C;
pub const ELF_SECTION_HEADER_STRING_TABLE_INDEX: u64 = 0x3E;

pub const ELF_DYNAMIC_SECTION_PHT_TYPE: u64 = 0x02;

pub const WEAPON_UNKNOWN: &str = "unknown";

#[allow(unused)]
pub const TEAM_SPECTATOR: u8 = 1;
pub const TEAM_T: u8 = 2;
pub const TEAM_CT: u8 = 3;
