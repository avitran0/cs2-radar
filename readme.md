# Linux CS2 Radar

this is a web-based, linux-only radar for counter-strike 2

## Setup

1. THIS ONLY WORKS ON LINUX!
2. install the rust toolchain from [here](https://rustup.rs/) and nodejs from your package manager
3. clone via git: `git clone github.com/avitran0/radar`
4. change to correct directory: `cd radar`
5. run with `./run.sh`
6. access in a browser [here](https://avitrano.com/radar)

## Building your own interface

the server sends out an array of all players every 50ms (configurable), do with that what you want

```ts
interface Player {
    name: string;
    health: number;
    armor: number;
    money: number;
    team: number;
    life_state: number;
    weapon: string;
    weapons: string[];
    has_defuser: boolean;
    has_helmet: boolean;
    color: number;
    position: Vec3;
    rotation: number;
    ping: number;
    steam_id: number;
    active_player: boolean;
}

interface Vec3 {
    x: number;
    y: number;
    z: number;
}
```
