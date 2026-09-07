#!/usr/bin/env bash
# Incremental Linux build (game + client DLL). Fast path for iterating on dlls/ or cl_dll/.
#
# Usage:
#   scripts/build.sh              # release build, g++, -j$(nproc)
#   scripts/build.sh debug        # debug build (symbols, no -O3)
#   COMPILER=clang++ scripts/build.sh
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CFG="${1:-release}"
COMPILER="${COMPILER:-g++}"
JOBS="${JOBS:-$(nproc)}"

cd "$REPO_ROOT/linux"
make COMPILER="$COMPILER" CFG="$CFG" -j"$JOBS"

echo ""
echo "Build output: linux/$CFG/"
ls -la "$CFG"/*.so 2>/dev/null || true
