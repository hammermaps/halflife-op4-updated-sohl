#!/usr/bin/env bash
# Build and run the standalone Hybrid AI Core tests. No engine headers, no
# DLL link, no test framework - just dlls/ai_hybrid_core.cpp plus the test
# file, compiled as a native host binary. Also wired into `make check` in
# linux/Makefile (scripts/build.sh does not run this — it only builds the
# game/client DLLs).
#
# Usage:
#   scripts/run-ai-hybrid-core-tests.sh
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="$REPO_ROOT/linux/release/obj/tests"
BIN="$OUT_DIR/ai_hybrid_core_tests"

mkdir -p "$OUT_DIR"

g++ -std=c++17 -fno-exceptions -fno-rtti -Wall -Wextra \
	"$REPO_ROOT/dlls/ai_hybrid_core.cpp" \
	"$REPO_ROOT/tests/ai_hybrid_core_tests.cpp" \
	-o "$BIN"

"$BIN"
