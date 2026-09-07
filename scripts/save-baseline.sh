#!/usr/bin/env bash
# Snapshot the current Linux build output as a reference baseline, with
# checksums, so a later build (e.g. "ai_hybrid 0 must be byte-identical to
# baseline") can be diffed against it via scripts/diff-baseline.sh.
#
# Usage:
#   scripts/build.sh                # build first
#   scripts/save-baseline.sh        # snapshot linux/release/ under a git-sha tag
#   scripts/save-baseline.sh debug  # snapshot linux/debug/ instead
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CFG="${1:-release}"
BUILD_DIR="$REPO_ROOT/linux/$CFG"
SHA="$(git -C "$REPO_ROOT" rev-parse --short HEAD)"
DIRTY="$(git -C "$REPO_ROOT" diff --quiet && git -C "$REPO_ROOT" diff --cached --quiet || echo "-dirty")"
OUT_DIR="$REPO_ROOT/reference-builds/${SHA}${DIRTY}-${CFG}"

if [ ! -d "$BUILD_DIR" ] || ! ls "$BUILD_DIR"/*.so >/dev/null 2>&1; then
  echo "No build output found in linux/$CFG/ — run scripts/build.sh $CFG first." >&2
  exit 1
fi

mkdir -p "$OUT_DIR"
cp "$BUILD_DIR"/*.so "$OUT_DIR"/
cp "$BUILD_DIR"/*.dbg "$OUT_DIR"/ 2>/dev/null || true

(
  cd "$OUT_DIR"
  sha256sum *.so > sha256sums.txt
)

echo "Baseline saved: $OUT_DIR"
cat "$OUT_DIR/sha256sums.txt"
