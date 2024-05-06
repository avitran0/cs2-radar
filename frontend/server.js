import express from "express";
import { spawn } from "child_process";
import { WebSocketServer } from "ws";
import { dirname } from "path";
import { fileURLToPath } from "url";

const app = express();
const port = 5173;

const dir = dirname(fileURLToPath(import.meta.url));

app.get("/", (req, res) => {
    res.sendFile(`${dir}/index.html`);
});

// static folder
app.use(express.static(`${dir}/static`));

const server = app.listen(port, () => {
    console.log(`Server listening on port ${port}`);
});

const ws = new WebSocketServer({ server });

const radar = spawn("./static/cs2-radar");

// data is a Buffer
radar.stdout.on("data", (data) => {
    console.info(`stdout: ${data}`);
});

radar.stderr.on("data", (data) => {
    console.error(`stderr: ${data}`);
    const str = data.toString();
    ws.clients.forEach((client) => {
        if (client.readyState === 1) {
            client.send(str);
        }
    });
});

radar.on("close", (code) => {
    console.warn(`child process exited with code ${code}`);
    process.exit(1);
});

radar.on("error", (err) => {
    console.error(err);
    process.exit(1);
});

server.on("close", () => {
    radar.kill();
    process.exit(0);
});
