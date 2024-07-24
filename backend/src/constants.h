#ifndef CS2_RADAR_CONSTANTS
#define CS2_RADAR_CONSTANTS

#define PROCESS_NAME "cs2"
#define CLIENT_LIB "libclient.so"
#define ENGINE_LIB "libengine2.so"
#define TIER0_LIB "libtier0.so"

#define ENTITY_OFFSET 0x50

#define ELF_PROGRAM_HEADER_OFFSET 0x20
#define ELF_PROGRAM_HEADER_ENTRY_SIZE 0x36
#define ELF_PROGRAM_HEADER_NUM_ENTRIES 0x38

#define ELF_SECTION_HEADER_OFFSET 0x28
#define ELF_SECTION_HEADER_ENTRY_SIZE 0x3A
#define ELF_SECTION_HEADER_NUM_ENTRIES 0x3C

#define ELF_DYNAMIC_SECTION_PHT_TYPE 0x02

#endif