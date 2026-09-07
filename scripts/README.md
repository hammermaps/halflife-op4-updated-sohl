# Dev scripts

Fast-iteration helpers for local Linux builds. All scripts wrap `linux/Makefile`
(see `BUILDING.md` / `CLAUDE.md` for the underlying `make` invocation and
requirements).

- `build.sh [release|debug]` — incremental build (default: release, `g++`, `-j$(nproc)`).
  `COMPILER=clang++ scripts/build.sh` to build with clang instead.
- `rebuild.sh [release|debug]` — same, but removes the build dir first (clean rebuild).
- `save-baseline.sh [release|debug]` — snapshots the current `linux/<cfg>/*.so` (+ `.dbg`)
  into `reference-builds/<git-sha>[-dirty]-<cfg>/` with a `sha256sums.txt` manifest.
  `reference-builds/` is gitignored — these are local reference points, not committed artifacts.
- `diff-baseline.sh <reference-builds/dir> [release|debug]` — hashes the current
  build output and reports IDENTICAL/CHANGED/MISSING against a saved baseline.
  Useful for verifying "this change shouldn't alter behavior when disabled"
  claims (e.g. `ai_hybrid 0`, per `docs/designs/hybrid-ai-core-phase-a.md`) —
  a byte-identical `.so` is strong evidence nothing changed at the binary level.

Typical flow when starting risky/behavior-sensitive work:

```bash
scripts/rebuild.sh              # clean baseline build
scripts/save-baseline.sh        # snapshot it
# ... make changes ...
scripts/build.sh                # incremental rebuild
scripts/diff-baseline.sh reference-builds/<sha>-release
```

- `deploy-test-mod.sh [release|debug]` — copies `linux/<cfg>/hl.so` and
  `client.so` (+ `.dbg` symbols) into the prepared Steam test-mod install so
  it can be launched directly from Half-Life for manual in-game verification.
  Default install: `~/.var/app/com.valvesoftware.Steam/.../halflife_op4_updated_ki`
  (override with `MOD_DIR=/other/path`). Run after `build.sh`/`rebuild.sh`:

```bash
scripts/build.sh
scripts/deploy-test-mod.sh
# then launch (see "Launching via Steam" below for why this needs a
# non-Steam shortcut on a Flatpak Steam install)
```

### Launching via Steam (Flatpak Steam caveat)

On a Flatpak Steam install, running `hl.sh`/`hl_linux` directly from a terminal
(even interactively) fails `SteamAPI_Init()` — confirmed 2026-09-08: Steam
writes its own sandbox-internal PID to `~/.steam/steam.pid`, which is
meaningless on the host, so the natively-run engine can't tell Steam is
running and aborts with "Failed to initialize authentication interface."
This reproduces even with the unmodified retail `gearbox` mod, so it's a
Steam/Flatpak environment issue, not something in this repo's code.

**Fix that works:** add the mod as a **non-Steam game** so Steam itself spawns
the process inside its own sandbox:

1. Steam Library → "+" → "Add a Non-Steam Game" → Browse to
   `~/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common/Half-Life/hl.sh`
2. Right-click the new entry → Properties → Launch Options:
   `-game halflife_op4_updated_ki -dev 2 -console -nojoy`
3. Launch from the Steam library (▶ Play) after each `deploy-test-mod.sh`.

- `debug-test-mod.sh [release|debug] [--batch]` — launches the deployed mod
  under `gdb` instead of plain `hl.sh`. **Must be run from a real interactive
  terminal**, not backgrounded/detached — launching the engine via `&`/`setsid`
  from a non-interactive shell breaks `SteamAPI_Init()` (auth fails) on at
  least this setup, because it loses env the Steam client normally injects.
  Sets `LD_LIBRARY_PATH` itself since the current engine's `hl.sh` no longer
  has a built-in `DEBUGGER=` wrapper.
  - Default (interactive): drops you into a `gdb` prompt with the engine
    loaded and args set — type `run`, and `bt` after a crash.
  - `--batch`: runs non-interactively, and on a crash automatically prints a
    backtrace + registers, logged to `debug-test-mod.log` (gitignored) at the
    repo root — useful for a quick "did this change break startup" check.

  Env overrides: `MOD_DIR` (mod path), `STEAM_HL` (Half-Life game root,
  default: parent of `MOD_DIR`), `GAME_ARGS` (extra engine args).

```bash
scripts/deploy-test-mod.sh
scripts/debug-test-mod.sh              # interactive
scripts/debug-test-mod.sh release --batch  # unattended crash triage
```
