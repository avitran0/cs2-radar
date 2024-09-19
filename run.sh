#!/bin/bash

cargo build --release >/dev/null 2>&1
npm install >/dev/null 2>&1
npx tsx index.ts
