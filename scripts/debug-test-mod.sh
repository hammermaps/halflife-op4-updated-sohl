#!/usr/bin/env bash
# Launch the deployed test mod under gdb.
#
# Run this from a real terminal (not backgrounded/detached) so the process
# inherits the full Steam session environment — launching it via `&`/`setsid`
# from a non-interactive shell breaks SteamAPI_Init() (auth fails) on this
# setup. The current Half-Life engine build's hl.sh no longer has a built-in
# DEBUGGER= wrapper, so this script sets LD_LIBRARY_PATH itself (same as
# hl.sh does) and invokes gdb directly with --args.
#
# Usage:
#   scripts/debug-test-mod.sh                 # interactive gdb, release build
#   scripts/debug-test-mod.sh debug           # interactive gdb, debug build
#   scripts/debug-test-mod.sh release --batch # non-interactive: run, on crash
#                                              # print bt + registers, quit —
#                                              # writes to debug-test-mod.log
#
# Env overrides:
#   MOD_DIR   - mod install dir (default: halflife_op4_updated_ki under the
#               default Steam Half-Life library path)
#   STEAM_HL  - Half-Life game root containing hl_linux/hl.sh (default:
#               parent directory of MOD_DIR)
#   GAME_ARGS - extra args appended to the engine command line
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CFG="${1:-release}"
MODE="${2:-interactive}"

MOD_DIR="${MOD_DIR:-$HOME/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common/Half-Life/halflife_op4_updated_ki}"
STEAM_HL="${STEAM_HL:-$(dirname "$MOD_DIR")}"
MOD_NAME="$(basename "$MOD_DIR")"

if [ ! -x "$STEAM_HL/hl_linux" ]; then
  echo "Engine binary not found: $STEAM_HL/hl_linux" >&2
  echo "Set STEAM_HL=/path/to/Half-Life if your install lives elsewhere." >&2
  exit 1
fi

if [ ! -f "$MOD_DIR/dlls/hl.so" ] || [ ! -f "$MOD_DIR/cl_dlls/client.so" ]; then
  echo "No deployed build found in $MOD_DIR — run scripts/deploy-test-mod.sh $CFG first." >&2
  exit 1
fi

export LD_LIBRARY_PATH="$STEAM_HL:${LD_LIBRARY_PATH:-}"

# shellcheck disable=SC2206 # intentional word-splitting of user-supplied extra args
EXTRA_ARGS=(${GAME_ARGS:-})
ENGINE_ARGS=(-game "$MOD_NAME" -dev 2 -console -nojoy -novid "${EXTRA_ARGS[@]}")

cd "$STEAM_HL"

if [ "$MODE" = "--batch" ] || [ "$MODE" = "batch" ]; then
  LOG_FILE="$REPO_ROOT/debug-test-mod.log"
  echo "Batch mode: running under gdb, logging to $LOG_FILE"
  gdb -q -batch \
    -ex "set pagination off" \
    -ex "run" \
    -ex "bt" \
    -ex "info registers" \
    -ex "quit" \
    --args ./hl_linux "${ENGINE_ARGS[@]}" \
    2>&1 | tee "$LOG_FILE"
else
  echo "Interactive gdb — type 'run' to start, 'bt' after a crash for a backtrace."
  gdb -q \
    -ex "set pagination off" \
    -ex "set breakpoint pending on" \
    --args ./hl_linux "${ENGINE_ARGS[@]}"
fi
