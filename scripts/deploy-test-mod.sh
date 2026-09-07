#!/usr/bin/env bash
# Deploy a local Linux build into the prepared Steam test-mod install so it
# can be launched from Half-Life directly. Copies:
#   linux/<cfg>/hl.so     -> <mod>/dlls/hl.so
#   linux/<cfg>/client.so -> <mod>/cl_dlls/client.so
# (+ matching .dbg symbol files, if present, for gdb backtraces)
#
# Usage:
#   scripts/deploy-test-mod.sh                # release, default mod dir below
#   scripts/deploy-test-mod.sh debug
#   MOD_DIR=/path/to/other/mod scripts/deploy-test-mod.sh
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CFG="${1:-release}"
BUILD_DIR="$REPO_ROOT/linux/$CFG"
MOD_DIR="${MOD_DIR:-$HOME/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common/Half-Life/halflife_op4_updated_ki}"

if [ ! -d "$MOD_DIR" ]; then
  echo "Mod directory not found: $MOD_DIR" >&2
  echo "Set MOD_DIR=/path/to/mod to point at a different install." >&2
  exit 1
fi

if [ ! -f "$BUILD_DIR/hl.so" ] || [ ! -f "$BUILD_DIR/client.so" ]; then
  echo "Missing build output in linux/$CFG/ — run scripts/build.sh $CFG first." >&2
  exit 1
fi

mkdir -p "$MOD_DIR/dlls" "$MOD_DIR/cl_dlls"
cp "$BUILD_DIR/hl.so" "$MOD_DIR/dlls/hl.so"
cp "$BUILD_DIR/client.so" "$MOD_DIR/cl_dlls/client.so"
cp "$BUILD_DIR/hl.so.dbg" "$MOD_DIR/dlls/" 2>/dev/null || true
cp "$BUILD_DIR/client.so.dbg" "$MOD_DIR/cl_dlls/" 2>/dev/null || true

echo "Deployed linux/$CFG build to: $MOD_DIR"
ls -la "$MOD_DIR/dlls/hl.so" "$MOD_DIR/cl_dlls/client.so"
echo ""
echo "Launch (Steam library -> Half-Life: Opposing Force Updated - KI), or from a terminal:"
echo "  ~/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common/Half-Life/hl.sh -game halflife_op4_updated_ki -dev -console"
