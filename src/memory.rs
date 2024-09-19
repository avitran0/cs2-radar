use std::{
    fs::{read_dir, read_link, File, OpenOptions},
    io::{BufRead, BufReader},
    path::Path,
};

use crate::process_handle::ProcessHandle;

pub fn get_pid(process_name: &str) -> Option<u64> {
    for dir in read_dir("/proc").unwrap() {
        let entry = dir.unwrap();
        if !entry.file_type().unwrap().is_dir() {
            continue;
        }

        let pid_osstr = entry.file_name();
        let pid = pid_osstr.to_str().unwrap();

        if pid.parse::<u64>().is_err() {
            continue;
        }

        let exe_name_path = read_link(format!("/proc/{}/exe", pid));
        if exe_name_path.is_err() {
            continue;
        }

        let exe_name_p = exe_name_path.unwrap();
        let (_, exe_name) = exe_name_p.to_str().unwrap().rsplit_once('/').unwrap();

        if exe_name == process_name {
            return Some(pid.parse::<u64>().unwrap());
        }
    }
    None
}

pub fn validate_pid(pid: u64) -> bool {
    return Path::new(format!("/proc/{}", pid).as_str()).exists();
}

pub fn open_process(pid: u64) -> Option<ProcessHandle> {
    if !validate_pid(pid) {
        return None;
    }

    let memory = OpenOptions::new()
        .read(true)
        .write(true)
        .open(format!("/proc/{pid}/mem"));
    match memory {
        Ok(mem) => Some(ProcessHandle::new(pid, mem)),
        _ => None,
    }
}

pub fn get_module_base_address(process: &ProcessHandle, module_name: &str) -> Option<u64> {
    let maps = File::open(format!("/proc/{}/maps", process.pid)).unwrap();
    for line in BufReader::new(maps).lines() {
        if line.is_err() {
            continue;
        }
        let line = line.unwrap();
        if !line.contains(module_name) {
            continue;
        }
        let (address, _) = line.split_once('-').unwrap();
        return Some(u64::from_str_radix(address, 16).unwrap());
    }
    None
}

pub fn get_module_path(process: &ProcessHandle, module_name: &str) -> Option<String> {
    let maps = File::open(format!("/proc/{}/maps", process.pid)).unwrap();
    for line in BufReader::new(maps).lines() {
        if line.is_err() {
            continue;
        }
        let line = line.unwrap();
        if !line.contains(module_name) {
            continue;
        }
        let (_, path) = line.split_once('/').unwrap();
        let mut p = String::from(path);
        p.insert(0, '/');
        return Some(p);
    }
    None
}

pub fn check_elf_header(data: Vec<u8>) -> bool {
    data.len() >= 4 && data[0..4] == [0x7f, b'E', b'L', b'F']
}

#[allow(unused)]
pub fn read_i8_vec(data: &[u8], address: u64) -> i8 {
    let adr = address as usize;
    let buffer = [data[adr]];
    i8::from_ne_bytes(buffer)
}

#[allow(unused)]
pub fn read_u8_vec(data: &[u8], address: u64) -> u8 {
    let adr = address as usize;
    let buffer = [data[adr]];
    u8::from_ne_bytes(buffer)
}

#[allow(unused)]
pub fn read_i16_vec(data: &[u8], address: u64) -> i16 {
    let adr = address as usize;
    let buffer = [data[adr], data[adr + 1]];
    i16::from_ne_bytes(buffer)
}

#[allow(unused)]
pub fn read_u16_vec(data: &[u8], address: u64) -> u16 {
    let adr = address as usize;
    let buffer = [data[adr], data[adr + 1]];
    u16::from_ne_bytes(buffer)
}

#[allow(unused)]
pub fn read_i32_vec(data: &[u8], address: u64) -> i32 {
    let adr = address as usize;
    let buffer = [data[adr], data[adr + 1], data[adr + 2], data[adr + 3]];
    i32::from_ne_bytes(buffer)
}

#[allow(unused)]
pub fn read_u32_vec(data: &[u8], address: u64) -> u32 {
    let adr = address as usize;
    let buffer = [data[adr], data[adr + 1], data[adr + 2], data[adr + 3]];
    u32::from_ne_bytes(buffer)
}

#[allow(unused)]
pub fn read_i64_vec(data: &[u8], address: u64) -> i64 {
    let adr = address as usize;
    let buffer = [
        data[adr],
        data[adr + 1],
        data[adr + 2],
        data[adr + 3],
        data[adr + 4],
        data[adr + 5],
        data[adr + 6],
        data[adr + 7],
    ];
    i64::from_ne_bytes(buffer)
}

#[allow(unused)]
pub fn read_u64_vec(data: &[u8], address: u64) -> u64 {
    let adr = address as usize;
    let buffer = [
        data[adr],
        data[adr + 1],
        data[adr + 2],
        data[adr + 3],
        data[adr + 4],
        data[adr + 5],
        data[adr + 6],
        data[adr + 7],
    ];
    u64::from_ne_bytes(buffer)
}

#[allow(unused)]
pub fn read_string_vec(data: &[u8], address: u64) -> String {
    let mut string = String::new();
    let mut i = address;
    loop {
        let c = data[i as usize];
        if c == 0 {
            break;
        }
        string.push(c as char);
        i += 1;
    }
    string
}
