use std::{thread::sleep, time::Duration};

use config::REFRESH_RATE;
use constants::PROCESS_NAME;
use cs2::{find_offsets, get_player_info};
use memory::{get_pid, open_process, validate_pid};
use offsets::Offsets;
use process_handle::ProcessHandle;

mod config;
mod constants;
mod cs2;
mod math;
mod memory;
mod offsets;
mod process_handle;

fn main() {
    #[cfg(not(target_os = "linux"))]
    compile_error!("only linux is supported.");
    loop {
        init();
        sleep(Duration::from_secs(5));
    }
}

fn run(process: &ProcessHandle, offsets: &Offsets) {
    let players = get_player_info(process, offsets);
    eprintln!("{}", serde_json::to_string(&players).unwrap());
}

fn init() {
    let pid = get_pid(PROCESS_NAME);
    if pid.is_none() {
        return;
    }
    let pid = pid.unwrap();
    println!("pid: {}", pid);

    let process = open_process(pid);
    if process.is_none() {
        return;
    }
    let process = process.unwrap();
    println!("process opened");

    let offsets = find_offsets(&process);
    if offsets.is_none() {
        return;
    }
    let offsets = offsets.unwrap();
    println!("offsets found");

    loop {
        if !validate_pid(pid) {
            println!("closed game");
            break;
        }
        run(&process, &offsets);
        sleep(Duration::from_millis((1000.0 / REFRESH_RATE) as u64));
    }
}
