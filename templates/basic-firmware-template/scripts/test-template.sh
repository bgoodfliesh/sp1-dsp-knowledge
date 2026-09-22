#!/bin/sh
set -eu

if ! command -v copier >/dev/null 2>&1; then
    echo "copier is required; install it with: pipx install copier" >&2
    exit 2
fi

template_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/basic-firmware-template.XXXXXX")
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

copier copy --defaults --trust "$template_dir" "$tmp_dir/project"

test -f "$tmp_dir/project/CMakeLists.txt"
test -f "$tmp_dir/project/src/modules/example_gain.c"
test -f "$tmp_dir/project/docs/knowledge-sources.yml"
test -f "$tmp_dir/project/tests/unit/Makefile"
test -f "$tmp_dir/project/tests/SKILL.md"

make -C "$tmp_dir/project/tests/unit" run

copier copy --defaults --trust \
    --data 'dsp_modules=["Reverb (Clouds-derived, from tape-looper)"]' \
    "$template_dir" "$tmp_dir/reverb-project"
test -f "$tmp_dir/reverb-project/lib/dsp/sp1_dsp_reverb.h"
test -f "$tmp_dir/reverb-project/src/modules/reverb.c"
make -C "$tmp_dir/reverb-project/tests/unit" run

echo "Copier render smoke test passed"
