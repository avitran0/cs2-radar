#ifndef CS2_RADAR_MEMORY
#define CS2_RADAR_MEMORY

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

class ProcessHandle {
  public:
    int pid;
    int memory;
    std::map<std::string, u64> lib_addresses;

    i8 read_i8(u64 address);
    i16 read_i16(u64 address);
    i32 read_i32(u64 address);
    i64 read_i64(u64 address);

    u8 read_u8(u64 address);
    u16 read_u16(u64 address);
    u32 read_u32(u64 address);
    u64 read_u64(u64 address);

    f32 read_f32(u64 address);
    f64 read_f64(u64 address);

    std::vector<u8> read_bytes(u64 address, size_t size);
    void write_bytes(u64 address, std::vector<u8> bytes);

    std::string read_string(u64 address);

    std::vector<u8> dump_module(u64 address);
    std::optional<u64> get_module_export(u64 offset, std::string name);
    std::optional<u64> get_address_from_dynamic_section(u64 offset, u64 tag);
    std::optional<u64> get_segment_from_pht(u64 offset, u64 tag);
    std::optional<u64> scan_pattern(std::vector<u8> pattern,
                                    std::vector<bool> mask, u64 module_offset);
    u64 get_relative_address(u64 instruction, u64 offset, u64 instruction_size);
    std::optional<u64> get_interface_offset(u64 lib_address,
                                            std::string interface_name);
    std::optional<u64> get_convar(u64 convar_offset, std::string convar_name);

    void discard();
};

std::optional<int> get_pid(std::string process_name);

bool validate_pid(int pid);

std::optional<ProcessHandle> open_process(int pid);
std::optional<ProcessHandle> open_process(std::string process_name);

std::optional<u64> get_module_base_address(int pid, std::string module_name);

i8 read_i8_from_vector(u8* bytes, u64 address);
i16 read_i16_from_vector(u8* bytes, u64 address);
i32 read_i32_from_vector(u8* bytes, u64 address);
i64 read_i64_from_vector(u8* bytes, u64 address);

u8 read_u8_from_vector(u8* bytes, u64 address);
u16 read_u16_from_vector(u8* bytes, u64 address);
u32 read_u32_from_vector(u8* bytes, u64 address);
u64 read_u64_from_vector(u8* bytes, u64 address);

f32 read_f32_from_vector(u8* bytes, u64 address);
f64 read_f64_from_vector(u8* bytes, u64 address);

#endif
