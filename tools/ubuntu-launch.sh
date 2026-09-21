#!/usr/bin/env bash
# Launch wrapper: starts ubuntu-build.sh in the background, detached from the
# SSH session. Uses nohup + setsid so it survives the SSH disconnect.
#
# Usage from Windows (SSH):
#   ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 \
#     '~/browseros-build/ubuntu-launch.sh --stop-after-phase 1'
#
# Then from another shell (no SSH needed):
#   ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 \
#     'tail -f ~/browseros-build/build.log'
#   # check state:
#   ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 \
#     'cat ~/browseros-build/build-state.json | jq .'
#
# Defaults to running all 5 phases; pass --stop-after-phase to truncate.
# See AGENTS-ubuntu-dev.md §7.2 for the full workflow.

set -euo pipefail

# Find the orchestrator in either of the expected locations.
CANDIDATES=(
  "$HOME/browersos-src/tools/ubuntu-build.sh"
  "$HOME/browseros-build/ubuntu-build.sh"
)
ORCHESTRATOR=""
for c in "${CANDIDATES[@]}"; do
  if [ -x "$c" ]; then
    ORCHESTRATOR="$c"
    break
  fi
done
if [ -z "$ORCHESTRATOR" ]; then
  echo "ERROR: orchestrator (ubuntu-build.sh) not found or not executable" >&2
  echo "       tried: ${CANDIDATES[*]}" >&2
  exit 1
fi

# Default env for the background process.
export BROWSEROS_CHROMIUM_SRC="${BROWSEROS_CHROMIUM_SRC:-$HOME/browseros-build/src}"
export BROWSEROS_BUILD_ROOT="${BROWSEROS_BUILD_ROOT:-$HOME/browseros-build}"
export BROWSEROS_LOG_DIR="${BROWSEROS_LOG_DIR:-$BROWSEROS_BUILD_ROOT}"
export BROWSEROS_BROWSEROS_EXE="${BROWSEROS_BROWSEROS_EXE:-$HOME/.local/bin/browseros}"
export BROWSEROS_FORK_ROOT="${BROWSEROS_FORK_ROOT:-$HOME/browersos-src}"
export PATH="$HOME/.local/bin:$HOME/depot_tools:${PATH}"
export PYTHONIOENCODING="utf-8"
export PYTHONUTF8="1"

mkdir -p "$BROWSEROS_LOG_DIR"

# Reject if already running — user must explicitly stop first.
PID_FILE="$BROWSEROS_LOG_DIR/build.pid"
if [ -f "$PID_FILE" ] && kill -0 "$(cat "$PID_FILE")" 2>/dev/null; then
  echo "ERROR: a build is already running (pid $(cat "$PID_FILE"))." >&2
  echo "       tail:  tail -f $BROWSEROS_LOG_DIR/build.log" >&2
  echo "       stop:  ssh -t ... kill \$(cat $PID_FILE)" >&2
  exit 0
fi

echo "Starting $ORCHESTRATOR $*"
echo "  log:   $BROWSEROS_LOG_DIR/build.log"
echo "  state: $BROWSEROS_LOG_DIR/build-state.json"
echo "  pid:   $PID_FILE (filled on start)"
echo

# setid + nohup so the process is fully detached from SSH session.
setsid nohup "$ORCHESTRATOR" "$@" </dev/null >"$BROWSEROS_LOG_DIR/launch.out" 2>&1 &
PID=$!
echo "$PID" > "$PID_FILE"
echo "Started (pid $PID). Tail with: tail -f $BROWSEROS_LOG_DIR/build.log"
