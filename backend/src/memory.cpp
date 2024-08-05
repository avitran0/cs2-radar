#include "memory.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "constants.h"
#include "log.h"

std::optional<int> get_pid(std::string process_name) {
    for (const auto &entry : std::filesystem::directory_iterator("/proc")) {
        if (!entry.is_directory()) {
            continue;
        }

        const auto filename = entry.path().filename().string();
        const auto exe_path = "/proc/" + filename + "/exe";
        if (access(exe_path.c_str(), F_OK) != 0) {
            continue;
        }
        const auto exe_name = std::filesystem::read_symlink(exe_path).string();
        const auto pos = exe_name.rfind('/');
        // rfind returns npos on fail
        if (pos == std::string::npos) {
            continue;
        }

        const auto name = exe_name.substr(pos + 1);
        if (name == process_name) {
            return std::stoi(filename);
        }
    }

    return std::nullopt;
}

bool validate_pid(int pid) {
    return access(("/proc/" + std::to_string(pid)).c_str(), F_OK) == 0;
}

std::optional<ProcessHandle> open_process(int pid) {
    if (!validate_pid(pid)) {
        return std::nullopt;
    }
    ProcessHandle handle;
    handle.pid = pid;
    handle.memory =
        open(("/proc/" + std::to_string(pid) + "/mem").c_str(), O_RDONLY);
    if (handle.memory == -1) {
        return std::nullopt;
    }
    return handle;
}

std::optional<u64> get_module_base_address(int pid, std::string module_name) {
    const std::string maps_path = "/proc/" + std::to_string(pid) + "/maps";
    const auto maps = fopen(maps_path.c_str(), "r");
    if (!maps) {
        return std::nullopt;
    }

    char line[256];
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, module_name.c_str())) {
            fclose(maps);
            return std::strtoull(line, nullptr, 16);
        }
    }
    fclose(maps);
    return std::nullopt;
}

i8 ProcessHandle::read_i8(u64 address) {
    i8 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

i16 ProcessHandle::read_i16(u64 address) {
    i16 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

i32 ProcessHandle::read_i32(u64 address) {
    i32 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

i64 ProcessHandle::read_i64(u64 address) {
    i64 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

u8 ProcessHandle::read_u8(u64 address) {
    u8 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

u16 ProcessHandle::read_u16(u64 address) {
    u16 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

u32 ProcessHandle::read_u32(u64 address) {
    u32 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

u64 ProcessHandle::read_u64(u64 address) {
    u64 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

f32 ProcessHandle::read_f32(u64 address) {
    f32 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

f64 ProcessHandle::read_f64(u64 address) {
    f64 value = 0;
    pread(memory, &value, sizeof(value), address);
    return value;
}

std::vector<u8> ProcessHandle::read_bytes(u64 address, size_t size) {
    std::vector<u8> buffer(size);
    pread(memory, buffer.data(), size, address);
    return buffer;
}

void ProcessHandle::write_bytes(u64 address, std::vector<u8> bytes) {
    pwrite(memory, bytes.data(), bytes.size(), address);
}

std::string ProcessHandle::read_string(u64 address) {
    std::string buffer;
    char c;
    while ((c = read_u8(address++)) != 0) {
        buffer.push_back(c);
    }
    return buffer;
}

std::vector<u8> ProcessHandle::dump_module(u64 address) {
    const auto section_header_offset =
        read_u64(address + ELF_SECTION_HEADER_OFFSET);
    const auto section_header_entry_size =
        read_u16(address + ELF_SECTION_HEADER_ENTRY_SIZE);
    const auto section_header_num_entries =
        read_u16(address + ELF_SECTION_HEADER_NUM_ENTRIES);

    const u64 module_size =
        section_header_offset +
        section_header_entry_size * section_header_num_entries;

    return read_bytes(address, module_size);
}

bool check_elf_header(std::vector<u8> data) {
    if (data.size() < 4) {
        return false;
    }
    return data[0] == 0x7f && data[1] == 'E' && data[2] == 'L' &&
           data[3] == 'F';
}

std::optional<u64> ProcessHandle::get_module_export(u64 offset,
                                                    std::string export_name) {
    const auto data = dump_module(offset);
    if (data.empty() || !check_elf_header(data)) {
        return std::nullopt;
    }

    const auto add = 0x18;
    const auto length = 0x08;

    const auto string_table_opt =
        get_address_from_dynamic_section(offset, 0x05);
    const auto symbol_table_opt =
        get_address_from_dynamic_section(offset, 0x06);
    if (!string_table_opt || !symbol_table_opt) {
        return std::nullopt;
    }

    const auto string_table = string_table_opt.value();
    auto symbol_table = symbol_table_opt.value();
    symbol_table += add;

    u32 st_name;
    while (read_u32(symbol_table) != 0) {
        st_name = read_u32(symbol_table);
        const auto name_bytes = read_bytes(string_table + st_name, 120);

        auto name = std::string(name_bytes.begin(), name_bytes.end());
        name.push_back(0x00);
        // find first occurrence of null byte (necessary?)
        const auto null_byte = name.find('\0');
        name = name.substr(0, null_byte);
        if (export_name == name) {
            const auto address_vec = read_bytes(symbol_table + length, length);
            // return u64 from vector
            return *(u64 *)address_vec.data() + offset;
        }
        symbol_table += add;
    }

    return std::nullopt;
}

std::optional<u64> ProcessHandle::get_address_from_dynamic_section(u64 offset,
                                                                   u64 tag) {
    const auto dynamic_section_offset =
        get_segment_from_pht(offset, ELF_DYNAMIC_SECTION_PHT_TYPE).value();

    const auto register_size = 8;

    u64 address = read_u64(dynamic_section_offset + 2 * register_size) + offset;

    while (true) {
        auto tag_address = address;
        const u64 tag_value = read_u64(tag_address);

        if (tag_value == 0) {
            break;
        }

        if (tag_value == tag) {
            return read_u64(tag_address + register_size);
        }

        address += register_size * 2;
    }

    return std::nullopt;
}

std::optional<u64> ProcessHandle::get_segment_from_pht(u64 offset, u64 tag) {
    const u64 first_entry =
        read_u32(offset + ELF_PROGRAM_HEADER_OFFSET) + offset;

    const auto pht_entry_size =
        read_u16(offset + ELF_PROGRAM_HEADER_ENTRY_SIZE);

    for (size_t i = 0; i < read_u16(offset + ELF_PROGRAM_HEADER_NUM_ENTRIES);
         i++) {
        const u64 entry = first_entry + i * pht_entry_size;
        if (read_u32(entry) == tag) {
            return entry;
        }
    }
    return std::nullopt;
}

std::optional<u64> ProcessHandle::scan_pattern(std::vector<u8> pattern,
                                               std::vector<bool> mask,
                                               u64 module_offset) {
    if (pattern.size() != mask.size()) {
        log("pattern and mask size mismatch");
        return std::nullopt;
    }
    const auto mem = dump_module(module_offset);
    if (mem.empty()) {
        return std::nullopt;
    }
    for (size_t i = 0; i < mem.size() - pattern.size(); i++) {
        bool found = true;
        for (size_t j = 0; j < pattern.size(); j++) {
            if (mask[j] && mem[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return module_offset + i;
        }
    }

    return std::nullopt;
}

u64 ProcessHandle::get_relative_address(u64 instruction, u64 offset,
                                        u64 instruction_size) {
    // THIS HAS TO BE I32!!!
    const u64 rip_address = read_i32(instruction + offset);
    return (u64)(instruction + instruction_size + rip_address);
}

std::optional<u64> ProcessHandle::get_interface_offset(
    u64 lib_address, std::string interface_name) {
    const auto interface_export =
        get_module_export(lib_address, "CreateInterface");
    if (!interface_export.has_value()) {
        log("failed to get CreateInterface export");
        return 0;
    }

    const auto export_address =
        get_relative_address(interface_export.value(), 0x01, 0x05) + 0x10;

    auto interface_entry =
        read_u64(export_address + 0x07 + read_u32(export_address + 0x03));
    const auto name_length = interface_name.length();

    while (true) {
        const auto interface_name_address = read_u64((interface_entry + 8));
        const auto name_bytes = read_bytes(interface_name_address, name_length);

        auto name = std::string(name_bytes.begin(), name_bytes.end());
        name.push_back(0x00);
        // find first occurrence of null byte
        const auto null_byte = name.find('\0');
        name = name.substr(0, null_byte);
        if (interface_name == name) {
            const auto vfunc_address = read_u64(interface_entry);
            return read_u32(vfunc_address + 0x03) + vfunc_address + 0x07;
        }
        interface_entry = read_u64(interface_entry + 0x10);
        if (interface_entry == 0) {
            break;
        }
    }

    return std::nullopt;
}

std::optional<u64> ProcessHandle::get_convar(u64 convar_offset,
                                             std::string convar_name) {
    const auto objects = read_u64(convar_offset + 64);
    const auto name_length = convar_name.length();

    for (size_t i = 0; i < read_u32(convar_offset + 160); i++) {
        const auto object = read_u64(objects + i * 16);
        if (object == 0) {
            break;
        }

        const auto name_bytes = read_bytes(read_u64(object), name_length);

        auto name = std::string(name_bytes.begin(), name_bytes.end());
        name.push_back(0x00);
        // find first occurrence of null byte
        const auto null_byte = name.find('\0');
        name = name.substr(0, null_byte);
        if (convar_name == name) {
            return object;
        }
    }

    return std::nullopt;
}

void ProcessHandle::discard() { close(memory); }

i8 read_i8_from_vector(u8 *bytes, u64 address) {
    return *(i8 *)(bytes + address);
}

i16 read_i16_from_vector(u8 *bytes, u64 address) {
    return *(i16 *)(bytes + address);
}

i32 read_i32_from_vector(u8 *bytes, u64 address) {
    return *(i32 *)(bytes + address);
}

i64 read_i64_from_vector(u8 *bytes, u64 address) {
    return *(i64 *)(bytes + address);
}

u8 read_u8_from_vector(u8 *bytes, u64 address) {
    return *(u8 *)(bytes + address);
}

u16 read_u16_from_vector(u8 *bytes, u64 address) {
    return *(u16 *)(bytes + address);
}

u32 read_u32_from_vector(u8 *bytes, u64 address) {
    return *(u32 *)(bytes + address);
}

u64 read_u64_from_vector(u8 *bytes, u64 address) {
    return *(u64 *)(bytes + address);
}

f32 read_f32_from_vector(u8 *bytes, u64 address) {
    return *(f32 *)(bytes + address);
}

f64 read_f64_from_vector(u8 *bytes, u64 address) {
    return *(f64 *)(bytes + address);
}
