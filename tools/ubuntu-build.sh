#!/usr/bin/env bash
# BrowserOS build orchestrator — Linux edition.
#
# Mirrors tools/bramburn-build.ps1 (the Windows orchestrator) phase-by-phase.
# Designed for detached invocation from SSH on the local Ubuntu LAN box
# (`192.168.0.45`, hostname `macmini2024`). For 12-24 h phase-3 builds,
# wrap in tmux — see AGENTS-ubuntu-dev.md §7.2.
#
# Phases:
#   1. setup   — `browseros build --setup`   (gclient sync + clean)
#   2. prep    — `browseros build --prep`    (configure + patches + resources)
#   3. build   — `browseros build --build`   (autoninja — the long one)
#   4. sign    — `browseros build --sign`    (no-op on Linux dev; surfaces clearly)
#   5. package — `browseros build --package` (.deb + tar.gz of out/Default)
#
# Usage (run from the box, not over an interactive SSH session):
#   ./tools/ubuntu-build.sh                        # all 5 phases
#   ./tools/ubuntu-build.sh --stop-after-phase 3   # stop after phase 3 (build)
#   ./tools/ubuntu-build.sh --phase 1 2            # only phases 1 and 2
#   ./tools/ubuntu-build.sh --dry-run              # print command, do nothing
#   ./tools/ubuntu-build.sh --help
#
# Env overrides:
#   BROWSEROS_CHROMIUM_SRC   default $HOME/browseros-build/src
#   BROWSEROS_BUILD_ROOT     default $HOME/browseros-build
#   BROWSEROS_LOG_DIR        default $BROWSEROS_BUILD_ROOT
#   BROWSEROS_BROWSEROS_EXE  default $HOME/.local/bin/browseros
#   BROWSEROS_FORK_ROOT      default $HOME/browersos-src      (the fork clone)
#   BROWSEROS_JOBS           default $(nproc)
#   BROWSEROS_AOSP_USER      default empty — if 'bramburn', defers to
#                            aosp_sync.sh to release the disk before phase 3

set -euo pipefail
IFS=$'\n\t'

# ───── Defaults ─────────────────────────────────────────────────────────────
FORK_ROOT="${BROWSEROS_FORK_ROOT:-$HOME/browersos-src}"
CHROMIUM_SRC="${BROWSEROS_CHROMIUM_SRC:-$HOME/browseros-build/src}"
BUILD_ROOT="${BROWSEROS_BUILD_ROOT:-$HOME/browseros-build}"
LOG_DIR="${BROWSEROS_LOG_DIR:-$BUILD_ROOT}"
LOG_FILE="${LOG_DIR}/build.log"
STATE_FILE="${LOG_DIR}/build-state.json"
BROWSEROS_EXE="${BROWSEROS_BROWSEROS_EXE:-$HOME/.local/bin/browseros}"
JOBS="${BROWSEROS_JOBS:-$(nproc)}"
STOP_AFTER_PHASE=0
DRY_RUN=0
PHASES_TO_RUN=(1 2 3 4 5)   # default = all

# ───── Helpers ──────────────────────────────────────────────────────────────
ts() { date -u +"%Y-%m-%dT%H:%M:%SZ"; }
iso_now() { date -Iseconds; }

log() {
  local level="${1:-INFO}"; shift || true
  local line="[$(ts)] [${level}] $*"
  mkdir -p "$LOG_DIR"
  printf '%s\n' "$line" >>"$LOG_FILE"
  if [ -t 1 ]; then printf '%s\n' "$line"; fi
}

write_state() {
  local phase="$1" name="$2" status="$3" last_error="${4:-}"
  mkdir -p "$LOG_DIR"
  python3 - "$STATE_FILE" "$phase" "$name" "$status" "$last_error" "$(iso_now)" \
    "$FORK_ROOT" "$CHROMIUM_SRC" "$LOG_FILE" <<'PY' 2>/dev/null || true
import json, sys
path, phase, name, status, err, updated, fork_root, src, log_file = sys.argv[1:]
tmp = path + ".tmp"
with open(tmp, "w", encoding="utf-8") as f:
    json.dump({
        "fork_root": fork_root,
        "chromium_src": src,
        "phase": int(phase),
        "phase_name": name,
        "status": status,
        "last_error": err,
        "updated_at": updated,
        "log_file": log_file,
    }, f, indent=2)
import os; os.replace(tmp, path)
PY
}

phase_in_args() {
  local n="$1"
  for p in "${PHASES_TO_RUN[@]}"; do [ "$p" = "$n" ] && return 0; done
  return 1
}

phase_should_stop_after() {
  local n="$1"
  [ "$STOP_AFTER_PHASE" -gt 0 ] && [ "$STOP_AFTER_PHASE" -eq "$n" ]
}

run_browseros() {
  local phase="$1" name="$2"; shift 2
  log "=== START phase ${phase} (${name}) ==="
  log "browseros $*"
  write_state "$phase" "$name" "running"

  if [ "$DRY_RUN" -eq 1 ]; then
    log "dry-run: would execute browseros $* from $FORK_ROOT"
    write_state "$phase" "$name" "ok"
    return 0
  fi

  # Run synchronously so we can capture the exit code. Caller should detach.
  local out="$LOG_FILE.phase${phase}.stdout"
  local err="$LOG_FILE.phase${phase}.stderr"
  if (cd "$FORK_ROOT" && "$BROWSEROS_EXE" "$@") >"$out" 2>"$err"; then
    log "=== END phase ${phase} (${name}) ==="
    write_state "$phase" "$name" "ok"
    return 0
  else
    local rc=$?
    log "phase ${phase} (${name}) FAILED: browseros exit ${rc}" "ERROR"
    write_state "$phase" "$name" "failed" "browseros exit ${rc}"
    return "$rc"
  fi
}

phase_setup()   { run_browseros 1 "setup"   build --setup   --chromium-src "$CHROMIUM_SRC"; }
phase_prep()    { run_browseros 2 "prep"    build --prep    --chromium-src "$CHROMIUM_SRC"; }
phase_build()   { run_browseros 3 "build"   build --build   --chromium-src "$CHROMIUM_SRC" -t release -a x64; }
phase_sign()    { log "phase 4 (sign) is a no-op on Linux dev builds — Windows-only"; write_state 4 "sign" "ok"; return 0; }
phase_package() { run_browseros 5 "package" build --package --chromium-src "$CHROMIUM_SRC" --target linux-dev; }

usage() {
  cat <<'EOF'
BrowserOS build orchestrator — Linux edition.

Usage: ubuntu-build.sh [options]

Options:
  --stop-after-phase N    Run phases 1..N then exit (N in 1..5).
                          0 means run all 5 phases (default).
  --phase N [N ...]       Only run the listed phases (whitespace-separated).
  --chromium-src PATH     Override CHROMIUM_SRC (default ~/browseros-build/src).
  --fork-root PATH        Override FORK_ROOT (default ~/browersos-src).
  --build-root PATH       Override BUILD_ROOT (default ~/browseros-build).
  --browseros PATH        Override browseros CLI path.
  --jobs N                Override parallel jobs (default $(nproc)).
  --dry-run               Print commands, do not execute.
  -h, --help              This help.

Logs:    $LOG_FILE
State:   $STATE_FILE

Phases (each is a browseros CLI invocation; partial progress is preserved):
  1 setup   gclient sync + clean
  2 prep    configure + patches + chromium_replace + resources
  3 build   autoninja (the 12-24 h one)
  4 sign    no-op on Linux
  5 package tar.gz + .deb of out/Default

For long-running phase 3, wrap in tmux:
  tmux new -s browseros -d './tools/ubuntu-build.sh'
  tmux attach -t browseros
EOF
}

# ───── Parse args ───────────────────────────────────────────────────────────
while [ $# -gt 0 ]; do
  case "$1" in
    --stop-after-phase) STOP_AFTER_PHASE="$2"; shift 2 ;;
    --phase)            shift; PHASES_TO_RUN=(); while [ $# -gt 0 ] && [[ "$1" =~ ^[1-5]$ ]]; do PHASES_TO_RUN+=("$1"); shift; done ;;
    --chromium-src)     CHROMIUM_SRC="$2"; shift 2 ;;
    --fork-root)        FORK_ROOT="$2"; shift 2 ;;
    --build-root)       BUILD_ROOT="$2"; LOG_DIR="$2"; shift 2 ;;
    --browseros)        BROWSEROS_EXE="$2"; shift 2 ;;
    --jobs)             JOBS="$2"; shift 2 ;;
    --dry-run)          DRY_RUN=1; shift ;;
    -h|--help)          usage; exit 0 ;;
    *)                  echo "unknown arg: $1" >&2; usage; exit 2 ;;
  esac
done

# Apply LOG_DIR default update if BUILD_ROOT was overridden
LOG_FILE="${LOG_DIR}/build.log"
STATE_FILE="${LOG_DIR}/build-state.json"

# ───── Pre-flight ───────────────────────────────────────────────────────────
mkdir -p "$LOG_DIR" "$BUILD_ROOT"

if [ "$DRY_RUN" -eq 0 ]; then
  if [ ! -x "$BROWSEROS_EXE" ]; then
    log "browseros CLI not found at $BROWSEROS_EXE — install it first: pipx install -e $FORK_ROOT/packages/browseros (see AGENTS-ubuntu-dev.md §5.4)" "ERROR"
    write_state 0 "init" "failed" "browseros CLI missing at $BROWSEROS_EXE"
    exit 1
  fi
  if [ ! -d "$FORK_ROOT" ]; then
    log "fork root $FORK_ROOT does not exist. Clone the fork first:  gh repo clone bramburn/BrowserOS $FORK_ROOT" "ERROR"
    write_state 0 "init" "failed" "fork root missing at $FORK_ROOT"
    exit 1
  fi
fi

log "ubuntu-build.sh starting"
log "FORK_ROOT:   $FORK_ROOT"
log "CHROMIUM_SRC:$CHROMIUM_SRC"
log "BUILD_ROOT:  $BUILD_ROOT"
log "LOG_FILE:    $LOG_FILE"
log "STATE_FILE:  $STATE_FILE"
log "BROWSEROS:   $BROWSEROS_EXE"
log "JOBS:        $JOBS"
log "PHASES:      ${PHASES_TO_RUN[*]}"
log "DRY_RUN:     $DRY_RUN"

# ───── Phase dispatch ───────────────────────────────────────────────────────
RC=0

for p in 1 2 3 4 5; do
  if phase_in_args "$p"; then
    case "$p" in
      1) phase_setup   || RC=$? ;;
      2) phase_prep    || RC=$? ;;
      3) phase_build   || RC=$? ;;
      4) phase_sign    || RC=$? ;;
      5) phase_package || RC=$? ;;
    esac
    if [ "$RC" -ne 0 ]; then log "phase $p failed, aborting"; break; fi
    if phase_should_stop_after "$p"; then log "stop-after-phase=$p reached, exiting cleanly"; break; fi
  fi
done

if [ "$RC" -eq 0 ]; then
  log "ubuntu-build.sh finished (all requested phases ok)"
  write_state 5 "package" "ok"
fi
exit "$RC"
