const radar = document.getElementById("radar");
const playersT = document.getElementById("players-t");
const playersCT = document.getElementById("players-ct");
const tContainer = document.getElementById("t-container");
const ctContainer = document.getElementById("ct-container");
const mapSelect = document.getElementById("map-select");
const indicator = document.getElementById("indicator");
const activate = document.getElementById("activate");
const radarTypeSelect = document.getElementById("radar-type-select");

let active = false;

activate.addEventListener("click", () => {
    if (active) {
        tContainer.style.visibility = "hidden";
        ctContainer.style.visibility = "hidden";
        activate.style.backgroundColor = "var(--color-red)";
        active = false;
    } else {
        tContainer.style.visibility = "visible";
        ctContainer.style.visibility = "visible";
        activate.style.backgroundColor = "var(--color-green)";
        active = true;
    }
});

activate.click();

const Teams = {
    None: 0,
    Spectator: 1,
    T: 2,
    CT: 3,
};

const Colors = {
    0: "var(--color-blue)",
    1: "var(--color-green)",
    2: "var(--color-yellow)",
    3: "var(--color-orange)",
    4: "var(--color-purple)",
    5: "var(--color-text)",
};

/** @type {Map<string, {x: number, y: number, scale: number,
 * rotate: boolean, zoom: number, lowerThreshold?: number}>}
 */
const mapData = {
    de_ancient: {
        x: -2953,
        y: 2164,
        scale: 5,
        rotate: false,
        zoom: 1,
    },
    de_dust2: {
        x: -2476,
        y: 3239,
        scale: 4.4,
        rotate: true,
        zoom: 1.1,
    },
    de_inferno: {
        x: -2087,
        y: 3870,
        scale: 4.9,
        rotate: false,
        zoom: 1,
    },
    de_mirage: {
        x: -3230,
        y: 1713,
        scale: 5,
        rotate: false,
        zoom: 1,
    },
    de_nuke: {
        x: -3453,
        y: 2887,
        scale: 7,
        rotate: false,
        zoom: 1,
        lowerThreshold: -495,
    },
    de_overpass: {
        x: -4831,
        y: 1781,
        scale: 5.2,
        rotate: false,
        zoom: 1,
    },
    de_vertigo: {
        x: -3168,
        y: 1762,
        scale: 4,
        rotate: false,
        zoom: 1,
        lowerThreshold: 11700,
    },
};

const MapLayers = {
    Default: 0,
    Lower: 1,
};

const RadarTypes = {
    Clean: "clean",
    Callouts: "callouts",
    Elevations: "elevations",
    Both: "both",
};

let map = mapData["de_nuke"];
let radarType = RadarTypes.Callouts;
radarTypeSelect.value = radarType;

let pingInterval;
function startWS() {
    const url = window.location.href.replace("http://", "ws://");
    const ws = new WebSocket(url);
    ws.onmessage = wsMessage;
    ws.onclose = () => {
        setTimeout(startWS, 1000);
        indicator.style.backgroundColor = "var(--color-red)";
        pingInterval = null;
    };
    pingInterval = setTimeout(() => {
        ws.send("ping");
    }, 1000);
}

function wsMessage(event) {
    indicator.style.backgroundColor = "var(--color-green)";

    /**
     * @type {{name: string, color: number, health: number, armor: number, team: number,
     * life_state: number, weapon: string, total_hits: number,
     * position: {x: number, y: number, z: number},
     * active_player: boolean}[]}
     */
    let data;
    try {
        data = JSON.parse(event.data);
    } catch (e) {
        //console.error(e);
        return;
    }

    playersT.innerHTML = "";
    playersCT.innerHTML = "";
    radar.innerHTML = "";
    if (data.length === 0 || !active) {
        return;
    }
    console.log(data);
    const active_player = data.find((player) => player.active_player);
    if (!active_player) {
        // reset to default radar layer
        setMapImage(getRadarName());
        return;
    }

    let active_player_layer = MapLayers.Default;
    // switch to lower radar layer if player is below threshold
    if (map.lowerThreshold && active_player.position.z < map.lowerThreshold) {
        setMapImage(getRadarName() + "_lower");
        active_player_layer = MapLayers.Lower;
    } else {
        setMapImage(getRadarName());
    }

    for (const player of data) {
        const element = getPlayerElement(player);
        if (player.team === Teams.T) {
            playersT.appendChild(element);
        } else if (player.team === Teams.CT) {
            playersCT.appendChild(element);
        }

        if (player.life_state !== 0) {
            continue;
        }
        const cx = radar.clientWidth;
        const cy = radar.clientHeight;
        const x = ((player.position.x - map.x) / map.scale) * (cx / 1024);
        const y = -((player.position.y - map.y) / map.scale) * (cy / 1024);

        const dot = getPlayerDot(player, x, y, player.team === active_player.team);

        const player_layer = player.position.z < map.lowerThreshold ? MapLayers.Lower : MapLayers.Default;
        if (player_layer !== active_player_layer) {
            dot.style.opacity = "0.5";
        }

        radar.appendChild(dot);
    }
}

function getPlayerElement(player) {
    const element = document.createElement("div");
    element.className = "player";

    const data = document.createElement("div");
    data.className = "data";

    /*const name = document.createElement("span");
                name.textContent = player.name;
                data.appendChild(name);*/

    const health = document.createElement("div");
    health.className = "health";
    const healthIcon = document.createElement("img");
    healthIcon.src = "/icons/svg/health.svg";
    const healthText = document.createElement("span");
    healthText.textContent = `${player.health}`;
    health.appendChild(healthIcon);
    health.appendChild(healthText);
    data.appendChild(health);

    const armor = document.createElement("div");
    armor.className = "armor";
    const armorIcon = document.createElement("img");
    armorIcon.src = "/icons/svg/armor.svg";
    const armorText = document.createElement("span");
    armorText.textContent = `${player.health <= 0 ? 0 : player.armor}`;
    armor.appendChild(armorIcon);
    armor.appendChild(armorText);
    data.appendChild(armor);

    const money = document.createElement("div");
    money.className = "money";
    const moneyIcon = document.createElement("img");
    moneyIcon.src = "/icons/svg/money.svg";
    const moneyText = document.createElement("span");
    moneyText.textContent = `${player.money}`;
    money.appendChild(moneyIcon);
    money.appendChild(moneyText);
    data.appendChild(money);

    if (player.life_state !== 0) {
        element.style.opacity = "0.5";
    }

    element.appendChild(data);

    // add svg for weapon
    const weaponContainer = document.createElement("div");
    weaponContainer.classList.add("weapon-container");
    const weapon = document.createElement("img");
    weapon.className = "weapon";
    if (player.weapon === "") {
        player.weapon = "unknown";
    }
    // check if the image exists
    weapon.src = `/icons/svg/${player.weapon}.svg`;

    const weaponName = document.createElement("p");
    weaponName.textContent = player.weapon;
    weaponContainer.appendChild(weapon);
    weaponContainer.appendChild(weaponName);
    element.appendChild(weaponContainer);

    element.style.borderColor = Colors[player.color] ?? "var(--color-text)";
    if (player.active_player) {
        element.style.backgroundColor = "var(--color-highlight)";
    }

    return element;
}

function getPlayerDot(player, x, y, friendly) {
    const dot = document.createElement("div");
    dot.className = "dot";
    let color;
    if (friendly) {
        dot.style.backgroundColor = Colors[player.color] ?? "var(--color-text)";
    } else {
        dot.style.backgroundColor = "var(--color-red)";
    }
    dot.style.left = `${x}px`;
    dot.style.top = `${y}px`;
    return dot;
}

mapSelect.addEventListener("change", () => {
    map = mapData[mapSelect.value];
    setMapImage(getRadarName());
});

radarTypeSelect.addEventListener("change", () => {
    radarType = radarTypeSelect.value;
    setMapImage(getRadarName());
});

function setMapImage(name) {
    radar.style.backgroundImage = `url(/radars/${name}.png)`;
}

function getRadarName() {
    return `${radarType}/${mapSelect.value}`;
}

map = mapData[mapSelect.value];
radarType = radarTypeSelect.value;
setMapImage(getRadarName());
startWS();
