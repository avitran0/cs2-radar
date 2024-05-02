import express from "express";
import { spawn } from "child_process";
import { dirname } from "path";
import { fileURLToPath } from "url";

const app = express();
const port = 5173;

app.get("/api", (req, res) => {
    res.send("Hello from Express");
});

app.listen(port, () => {
    console.log(`Server listening on port ${port}`);
});

const radar = spawn("./static/cs2-radar");

// data is a Buffer
radar.stdout.on("data", (data) => {
    console.log(`stdout: ${data}`);
});

radar.stderr.on("data", (data) => {
    const str = data.toString();
    console.error(JSON.parse(str));
});

radar.on("close", (code) => {
    console.log(`child process exited with code ${code}`);
    process.exit(1);
});

radar.on("error", (err) => {
    console.error(err);
    process.exit(1);
});
