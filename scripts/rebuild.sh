#!/usr/bin/env bash
# Full clean rebuild. Use when incremental builds act up, or before saving a
# reference baseline (scripts/save-baseline.sh) so the binary reflects a clean tree.
#
# Usage:
#   scripts/rebuild.sh              # release
#   scripts/rebuild.sh debug        # debug
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CFG="${1:-release}"
COMPILER="${COMPILER:-g++}"
JOBS="${JOBS:-$(nproc)}"

cd "$REPO_ROOT/linux"
rm -rf "$CFG"
make COMPILER="$COMPILER" CFG="$CFG" -j"$JOBS"

echo ""
echo "Build output: linux/$CFG/"
ls -la "$CFG"/*.so 2>/dev/null || true
