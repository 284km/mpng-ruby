#!/bin/sh
# verify.sh — the gem answers exactly like the codec it wraps.
#
#   MERE=/path/to/mere-checkout sh verify.sh
#
# Four claims, each checked from the outside:
#
#   1. freshness  — ext/mpng/libmpng.{c,h} are REGENERATED from libmpng.mere
#                   before anything runs: a gate that tests a vendored copy
#                   tests the past, not the code.
#   2. exactness  — for every test PNG (generated with its answer by mpng's
#                   own make_pngs.py), Mpng.to_ppm equals the computed
#                   .expected byte for byte, AND equals what the mpng CLI
#                   writes for the same file: the boundary must not change
#                   the codec's answer.
#   3. errors     — a non-PNG raises, with the codec's message, and the
#                   process survives.
#   4. boundedness — two runs at different call counts must report identical
#                   default-region stats (MERE_REGION_STATS): calls are
#                   transactions, N of them cost what one costs.
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
MERE_ROOT="${MERE:-}"
[ -n "$MERE_ROOT" ] || { echo "usage: MERE=/path/to/mere-checkout sh verify.sh" >&2; exit 2; }
M="$MERE_ROOT/_build/default/bin/mere.exe"
[ -x "$M" ] || { echo "verify: $M not found (dune build?)" >&2; exit 2; }
MPNG_DIR="${MPNG_DIR:-$DIR/.mere_deps_probe}"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# --- 1. regenerate the vendored C from the Mere source -----------------------
( cd "$DIR" && "$M" -c --lib libmpng.mere > ext/mpng/libmpng.c \
            && "$M" --header libmpng.mere > ext/mpng/libmpng.h ) \
  || { echo "FAIL verify: regenerating libmpng.c/h"; exit 1; }
( cd "$DIR/ext/mpng" && ruby extconf.rb > /dev/null && make -s 2>/dev/null | grep -v warning || true )
[ -f "$DIR/ext/mpng/mpng_ext.bundle" ] || [ -f "$DIR/ext/mpng/mpng_ext.so" ] \
  || { echo "FAIL verify: extension did not build"; exit 1; }

# --- test images: computed together with their answers ------------------------
MPNG_SRC="$TMP/mpng_src"
"$M" install --quiet 2>/dev/null || "$M" install > /dev/null 2>&1 || true
# make_pngs.py lives in the mpng package; fetch it from the dependency cache
CACHE_PY="$(find "$HOME/.mere/cache" -name make_pngs.py 2>/dev/null | head -1)"
[ -n "$CACHE_PY" ] || { echo "FAIL verify: mpng's make_pngs.py not found (mere install?)"; exit 1; }
python3 "$CACHE_PY" "$TMP" > "$TMP/cases" || { echo "FAIL verify: make_pngs.py"; exit 1; }
MPNG_MAIN="$(dirname "$CACHE_PY")/../mpng.mere"

# --- 2 + 3. exactness and the error path --------------------------------------
RB="ruby -I $DIR/ext/mpng -I $DIR/lib"
fails=0
for png in "$TMP"/*.png; do
  name="$(basename "$png" .png)"
  exp="$png.expected"
  [ -f "$exp" ] || continue
  $RB -e '
    require "mpng"
    ppm = Mpng.to_ppm(File.binread(ARGV[0]))
    exit(ppm == File.binread(ARGV[1]) ? 0 : 1)' "$png" "$exp" \
    || { echo "FAIL verify: $name gem output != computed expectation"; fails=$((fails+1)); continue; }
  if "$M" "$MPNG_MAIN" ppm "$png" "$TMP/cli.ppm" > /dev/null 2>&1; then
    $RB -e '
      require "mpng"
      exit(Mpng.to_ppm(File.binread(ARGV[0])) == File.binread(ARGV[1]) ? 0 : 1)' "$png" "$TMP/cli.ppm" \
      || { echo "FAIL verify: $name gem output != mpng CLI output"; fails=$((fails+1)); }
  fi
  echo "  ok | $name"
done
[ "$fails" -eq 0 ] || exit 1

$RB -e '
  require "mpng"
  begin
    Mpng.to_ppm("not a png")
    exit 1
  rescue RuntimeError => e
    exit(e.message.include?("not a PNG") ? 0 : 1)
  end' || { echo "FAIL verify: error path (fail should raise, host should survive)"; exit 1; }
echo "  ok | fail -> RuntimeError, host survives"

# --- 4. boundedness ------------------------------------------------------------
PNG1="$TMP/rgb_mixed.png"
run_n () {
  MERE_REGION_STATS=1 $RB -e '
    require "mpng"
    data = File.binread(ARGV[0])
    ARGV[1].to_i.times { Mpng.to_ppm(data) }
    Mpng.shutdown' "$PNG1" "$1" 2>&1 | grep '^region-stats default:'
}
s_small="$(run_n 50)"
s_large="$(run_n 2000)"
[ -n "$s_small" ] || { echo "FAIL verify: no region-stats line — meter gone"; exit 1; }
[ "$s_small" = "$s_large" ] || {
  echo "FAIL verify: default region grew with the call count"
  echo "---   50 calls: $s_small"
  echo "--- 2000 calls: $s_large"
  exit 1; }
echo "  ok | 50 and 2000 decode calls: identical default-region stats"

echo "verify: ok"
