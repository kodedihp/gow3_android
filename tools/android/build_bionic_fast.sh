#!/usr/bin/env bash
# Fast path for the RE7 boot/import loop.
#
# Reuses libgow3cpu.so, libgow3gpu.so and libatrac9.a from a previous Bionic build
# and only rebuilds gow3-probe. That is enough to test src/runtime*.c, src/probe.c
# and src/hle/re7_playable.cpp changes. If a cached artifact is missing,
# build_bionic.sh automatically falls back to building it.
#
# Do not use this when cpu/**, gpu/** or third_party/LibAtrac9/** changed; run
# tools/android/build_bionic.sh normally in that case.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."

export GOW3_REUSE_CPU_GPU=${GOW3_REUSE_CPU_GPU:-1}
export GOW3_REUSE_ATRAC9=${GOW3_REUSE_ATRAC9:-1}
export GOW3_SKIP_FRAMEGEN=${GOW3_SKIP_FRAMEGEN:-1}

exec bash tools/android/build_bionic.sh "$@"
