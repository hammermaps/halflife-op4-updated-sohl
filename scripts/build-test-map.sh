#!/usr/bin/env bash
# Compile mapsrc/*.map into a .bsp with the VHLT compilers, then deploy it
# into the test mod's maps/ folder so it can be loaded with `map <name>`.
#
# Usage:
#   scripts/build-test-map.sh ai_test_grunt
#
# Env overrides:
#   VHLT_BIN - directory containing hlcsg/hlbsp/hlvis/hlrad (default: ~/hltools/vhlt/bin)
#   MOD_DIR  - mod install dir to copy the .bsp into (default: halflife_op4_updated_ki
#              under the default Steam Half-Life library path; same default as
#              deploy-test-mod.sh)
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MAP_NAME="${1:?Usage: $0 <map-name-without-extension>}"
MAP_SRC="$REPO_ROOT/mapsrc/$MAP_NAME.map"

VHLT_BIN="${VHLT_BIN:-$HOME/hltools/vhlt/bin}"
MOD_DIR="${MOD_DIR:-$HOME/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common/Half-Life/halflife_op4_updated_ki}"

if [ ! -f "$MAP_SRC" ]; then
  echo "Map source not found: $MAP_SRC" >&2
  exit 1
fi

for tool in hlcsg hlbsp hlvis hlrad; do
  if [ ! -x "$VHLT_BIN/$tool" ]; then
    echo "Missing compiler: $VHLT_BIN/$tool" >&2
    echo "Set VHLT_BIN=/path/to/vhlt/bin if your compilers live elsewhere." >&2
    exit 1
  fi
done

BUILD_DIR="$REPO_ROOT/mapsrc/build"
mkdir -p "$BUILD_DIR"
cp "$MAP_SRC" "$BUILD_DIR/$MAP_NAME.map"

cd "$BUILD_DIR"
"$VHLT_BIN/hlcsg" -nowadtextures "$MAP_NAME"
"$VHLT_BIN/hlbsp" "$MAP_NAME"
"$VHLT_BIN/hlvis" -fast "$MAP_NAME"
"$VHLT_BIN/hlrad" -bounce 2 "$MAP_NAME"

if [ ! -f "$BUILD_DIR/$MAP_NAME.bsp" ]; then
  echo "Compile failed - no $MAP_NAME.bsp produced. Check the compiler output above." >&2
  exit 1
fi

mkdir -p "$MOD_DIR/maps"
cp "$BUILD_DIR/$MAP_NAME.bsp" "$MOD_DIR/maps/$MAP_NAME.bsp"

echo ""
echo "Compiled and deployed: $MOD_DIR/maps/$MAP_NAME.bsp"
echo "In-game console: map $MAP_NAME"
