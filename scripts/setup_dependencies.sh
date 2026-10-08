#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$ROOT/external"

if [ ! -f "$ROOT/external/imgui/imgui.cpp" ]; then
    git clone --depth 1 https://github.com/ocornut/imgui.git "$ROOT/external/imgui"
fi

if [ ! -f "$ROOT/external/json/include/nlohmann/json.hpp" ]; then
    git clone --depth 1 https://github.com/nlohmann/json.git "$ROOT/external/json"
fi

echo "Dependencies are ready."
