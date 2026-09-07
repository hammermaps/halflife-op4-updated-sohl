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
