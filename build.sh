#!/usr/bin/env bash
# Builds pharmacy.exe with g++. Uses g++ from PATH, or else the WinLibs copy
# that `winget install BrechtSanders.WinLibs.POSIX.UCRT` puts in AppData.
set -e
cd "$(dirname "$0")"

if ! command -v g++ >/dev/null 2>&1; then
    for dir in "$HOME"/AppData/Local/Microsoft/WinGet/Packages/BrechtSanders.WinLibs.*/mingw64/bin; do
        if [ -x "$dir/g++.exe" ]; then
            PATH="$dir:$PATH"
            break
        fi
    done
fi

if ! command -v g++ >/dev/null 2>&1; then
    echo "g++ not found. Install it from PowerShell with:"
    echo "  winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT"
    exit 1
fi

g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o pharmacy.exe
echo "Built pharmacy.exe - run it with: ./pharmacy.exe"
