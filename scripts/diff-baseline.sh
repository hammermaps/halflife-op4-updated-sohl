#!/usr/bin/env bash
# Compare the current build output against a saved baseline (see
# scripts/save-baseline.sh). Useful for verifying "ai_hybrid 0 (default)
# changes nothing" claims: if the .so is byte-identical to a pre-change
# baseline, behavior can't have changed at the binary level.
#
# Usage:
#   scripts/diff-baseline.sh reference-builds/<sha>-release
#   scripts/diff-baseline.sh reference-builds/<sha>-release debug
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BASELINE_DIR="${1:?Usage: $0 <reference-builds/baseline-dir> [cfg]}"
CFG="${2:-release}"
BUILD_DIR="$REPO_ROOT/linux/$CFG"

if [ ! -f "$BASELINE_DIR/sha256sums.txt" ]; then
  echo "No sha256sums.txt in $BASELINE_DIR — is this a directory made by scripts/save-baseline.sh?" >&2
  exit 1
fi

echo "Comparing linux/$CFG/ against $BASELINE_DIR ..."
STATUS=0
while read -r EXPECTED FILE; do
  if [ ! -f "$BUILD_DIR/$FILE" ]; then
    echo "MISSING: $FILE (not present in linux/$CFG/)"
    STATUS=1
    continue
  fi
  ACTUAL="$(sha256sum "$BUILD_DIR/$FILE" | awk '{print $1}')"
  if [ "$ACTUAL" = "$EXPECTED" ]; then
    echo "IDENTICAL: $FILE"
  else
    echo "CHANGED:   $FILE"
    STATUS=1
  fi
done < "$BASELINE_DIR/sha256sums.txt"

exit "$STATUS"
