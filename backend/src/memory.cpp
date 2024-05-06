#include "memory.h"

#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "config.h"
#include "log.h"

std::optional<int> get_pid(std::string process_name) {
    auto dir = opendir("/proc");
    if (!dir) {
        return std::nullopt;
    }

    dirent* entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_DIR) {
            continue;
        }

        // get executable name
        std::string exe_path = std::string("/proc/") + entry->d_name + "/exe";
        char exe[256] = {0};
        if (readlink(exe_path.c_str(), exe, sizeof(exe) - 1) == -1) {
            continue;
        }

        // get executable name
        std::string name = std::string(exe);
        size_t pos = name.rfind('/');
        if (pos == std::string::npos) {
            continue;
        }

        name = name.substr(pos + 1);
        if (name == process_name) {
            closedir(dir);
            return std::stoi(entry->d_name);
        }
    }
    closedir(dir);
    return std::nullopt;
}

bool validate_pid(int pid) {
    return access(("/proc/" + std::to_string(pid)).c_str(), F_OK) != -1;
}

std::optional<ProcessHandle> open_process(int pid) {
    if (!validate_pid(pid)) {
        return std::nullopt;
    }
    ProcessHandle handle;
    handle.pid = pid;
    handle.memory =
        open(("/proc/" + std::to_string(pid) + "/mem").c_str(), O_RDWR);
    if (handle.memory == -1) {
        return std::nullopt;
    }
    handle.lib_addresses = {};
    return handle;
}

std::optional<ProcessHandle> open_process(std::string process_name) {
    auto pid = get_pid(process_name);
    if (!pid) {
        return std::nullopt;
    }
    return open_process(pid.value());
}

std::optional<u64> get_module_base_address(int pid, std::string module_name) {
    std::string maps_path = "/proc/" + std::to_string(pid) + "/maps";
    auto maps = fopen(maps_path.c_str(), "r");
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
    auto section_header_offset = read_u64(address + ELF_SECTION_HEADER_OFFSET);
    auto section_header_entry_size =
        read_u16(address + ELF_SECTION_HEADER_ENTRY_SIZE);
    auto section_header_num_entries =
        read_u16(address + ELF_SECTION_HEADER_NUM_ENTRIES);

    u64 module_size = section_header_offset +
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
    auto data = dump_module(offset);
    if (data.empty() || !check_elf_header(data)) {
        return std::nullopt;
    }

    auto wow64 = data[ELF_MACHINE_TYPE] == ELF_MACHINE_x64;

    auto add = wow64 ? 0x18 : 0x10;
    auto length = wow64 ? 0x08 : 0x04;

    auto string_table_opt = get_dynamic_address(offset, 0x05);
    auto symbol_table_opt = get_dynamic_address(offset, 0x06);
    if (!string_table_opt || !symbol_table_opt) {
        return std::nullopt;
    }

    auto string_table = string_table_opt.value();
    auto symbol_table = symbol_table_opt.value();
    symbol_table += add;

    u32 st_name;
    while (read_u32(symbol_table) != 0) {
        st_name = read_u32(symbol_table);
        auto name_bytes = read_bytes(string_table + st_name, 120);

        auto name = std::string(name_bytes.begin(), name_bytes.end());
        name.push_back(0x00);
        // find first occurrence of null byte
        auto null_byte = name.find('\0');
        name = name.substr(0, null_byte);
        if (export_name == name) {
            auto address_vec = read_bytes(symbol_table + length, length);
            // return u64 from vector
            return (wow64 ? *(u64*)address_vec.data()
                          : *(u32*)address_vec.data()) +
                   offset;
        }
        symbol_table += add;
    }

    return std::nullopt;
}

std::optional<u64> ProcessHandle::get_dynamic_address(u64 offset, u64 tag) {
    auto dynamic_section_offset =
        get_elf_address(offset, ELF_DYNAMIC_SECTION).value();

    auto wow64 = read_u16(offset + ELF_MACHINE_TYPE) == ELF_MACHINE_x64;
    auto register_size = wow64 ? 8 : 4;

    u64 address =
        wow64 ? read_u64(dynamic_section_offset + 2 * register_size) + offset
              : read_u32(dynamic_section_offset + 2 * register_size) + offset;

    while (true) {
        auto tag_address = address;
        u64 tag_value = wow64 ? read_u64(tag_address) : read_u32(tag_address);

        if (tag_value == 0) {
            break;
        }

        if (tag_value == tag) {
            return wow64 ? read_u64(tag_address + register_size)
                         : read_u32(tag_address + register_size);
        }

        address += register_size * 2;
    }

    return std::nullopt;
}

std::optional<u64> ProcessHandle::get_elf_address(u64 offset, u64 tag) {
    u64 first_entry = read_u32(offset + ELF_PROGRAM_HEADER_OFFSET) + offset;

    auto pht_entry_size = read_u16(offset + ELF_PROGRAM_HEADER_ENTRY_SIZE);

    for (size_t i = 0; i < read_u16(offset + ELF_PROGRAM_HEADER_NUM_ENTRIES);
         i++) {
        u64 entry = first_entry + i * pht_entry_size;
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
    auto mem = dump_module(module_offset);
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
    u64 rip_address = read_i32(instruction + offset);
    return (u64)(instruction + instruction_size + rip_address);
}

std::optional<u64> ProcessHandle::get_interface_offset(
    u64 lib_address, std::string interface_name) {
    auto interface_export = get_module_export(lib_address, "CreateInterface");
    if (!interface_export.has_value()) {
        log("failed to get CreateInterface export");
        return 0;
    }

    auto export_address =
        get_relative_address(interface_export.value(), 0x01, 0x05) + 0x10;

    auto interface_entry =
        read_u64(export_address + 0x07 + read_u32(export_address + 0x03));
    auto name_length = interface_name.length();

    while (true) {
        auto interface_name_address = read_u64((interface_entry + 8));
        auto name_bytes = read_bytes(interface_name_address, name_length);

        auto name = std::string(name_bytes.begin(), name_bytes.end());
        name.push_back(0x00);
        // find first occurrence of null byte
        auto null_byte = name.find('\0');
        name = name.substr(0, null_byte);
        if (interface_name == name) {
            auto vfunc_address = read_u64(interface_entry);
            auto address =
                read_u32(vfunc_address + 0x03) + vfunc_address + 0x07;
            return address;
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
    auto objects = read_u64(convar_offset + 64);
    auto name_length = convar_name.length();

    for (size_t i = 0; i < read_i32(convar_offset + 160); i++) {
        auto object = read_u64(objects + i * 16);
        if (object == 0) {
            break;
        }

        auto name_bytes = read_bytes(read_u64(object), name_length);

        auto name = std::string(name_bytes.begin(), name_bytes.end());
        name.push_back(0x00);
        // find first occurrence of null byte
        auto null_byte = name.find('\0');
        name = name.substr(0, null_byte);
        if (convar_name == name) {
            return object;
        }
    }

    return std::nullopt;
}

void ProcessHandle::discard() {
    close(memory);
}

i8 read_i8_from_vector(u8* bytes, u64 address) {
    return *(i8*)(bytes + address);
}

i16 read_i16_from_vector(u8* bytes, u64 address) {
    return *(i16*)(bytes + address);
}

i32 read_i32_from_vector(u8* bytes, u64 address) {
    return *(i32*)(bytes + address);
}

i64 read_i64_from_vector(u8* bytes, u64 address) {
    return *(i64*)(bytes + address);
}

u8 read_u8_from_vector(u8* bytes, u64 address) {
    return *(u8*)(bytes + address);
}

u16 read_u16_from_vector(u8* bytes, u64 address) {
    return *(u16*)(bytes + address);
}

u32 read_u32_from_vector(u8* bytes, u64 address) {
    return *(u32*)(bytes + address);
}

u64 read_u64_from_vector(u8* bytes, u64 address) {
    return *(u64*)(bytes + address);
}

f32 read_f32_from_vector(u8* bytes, u64 address) {
    return *(f32*)(bytes + address);
}

f64 read_f64_from_vector(u8* bytes, u64 address) {
    return *(f64*)(bytes + address);
}
