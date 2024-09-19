import express from "express";
import { spawn } from "child_process";
import { WebSocketServer } from "ws";
import { existsSync } from "fs";
import { exit } from "process";

const app = express();
const port = 9001;

const server = app.listen(port, () => {
    console.log(`server started on port ${port}`);
});

const wss = new WebSocketServer({ server });

wss.on("connection", (socket, client) => {
    const ip = client.socket.remoteAddress?.replace("::ffff:", "") ?? "?";
    console.log(`client connected (ip: ${ip})`);

    socket.on("close", () => {
        console.log(`client disconnected (ip: ${ip})`);
    });
});

if (!existsSync("./target/release/radar")) {
    console.error("executable not found");
    exit(1);
}
const radar = spawn("./target/release/radar");

// data is a Buffer
radar.stdout.on("data", (data) => {
    process.stdout.write(`[radar] ${data}`);
});

radar.stderr.on("data", (data) => {
    //console.error(`stderr: ${data}`);
    const str = data.toString();
    wss.clients.forEach((client) => {
        if (client.readyState === 1) {
            client.send(str);
        }
    });
});

radar.on("close", (code) => {
    console.warn(`child process exited with code ${code}`);
    exit(1);
});

radar.on("error", (err) => {
    console.error(err);
    exit(1);
});

server.on("close", () => {
    radar.kill();
    exit(0);
});
