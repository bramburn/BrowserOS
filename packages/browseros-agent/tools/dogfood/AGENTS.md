# `tools/dogfood/` — `browseros-dogfood` alpha-testing CLI

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

The `browseros-dogfood` Go CLI (module `browseros-dogfood`, Go 1.25.7). It
runs the current checkout of a *separate* BrowserOS repo against a **copy** of
your real profile, so you can alpha-test `dev` with your real data without
touching your installed browser. It builds the extension, starts the local
server, launches the installed BrowserOS app with
`--browseros-dock-icon=alpha`, and keeps all state under
`~/.browseros-dogfood` and `~/.config/browseros-dogfood/`. A background mode
(`start-background`) runs the whole thing as a daemon controlled over a Unix
socket.

It is **not** the shipped user CLI — that is `apps/cli` (`browseros-cli`).

## Contents

```
tools/dogfood/
├── main.go, go.mod, go.sum   ← module browseros-dogfood; cobra, fatih/color, yaml.v3
├── Makefile                  ← build / install ($HOME/bin, ad-hoc codesign) / test / clean
├── README.md                 ← the full user-facing guide
├── cmd/                      ← init, start, start-background, daemon, status, stop, restart, pull, logs, config, refresh-profile, source-profile
├── config/                   ← config.yaml load/save/validate, path helpers, prod-env defaults
├── profile/                  ← allowlist-based profile import + Local State profile discovery
├── pipeline/                 ← setup.sh + wxt build, .env.production writing, git helpers
├── proc/                     ← managed processes, port resolution, log files, line streaming
├── runlog/                   ← JSONL run log writer/reader
├── runtime/                  ← flock single-instance lock + RunState
├── ipc/                      ← Unix-socket JSON control protocol (status/stop/restart)
├── browser/                  ← launch args (alpha dock icon) + CDP readiness
└── internal/fspath/          ← IsSameOrChild containment guard
```

## Rules

**DF1 — The user's real profile is read-only input.** `start --refresh-profile`
copies *from* `source_user_data_dir` *into* the dogfood dev profile. The copy
uses an explicit allowlist (`Extensions`, `Cookies`, `Login Data`,
`Preferences`, `Bookmarks`, `History`, `IndexedDB/chrome-extension_*`, …) —
caches and broad site storage are deliberately excluded. Never widen the
allowlist to "copy everything".

**DF2 — Containment is enforced, not assumed.** `profile.Import` refuses to
run when the dev user-data dir is the same as, or inside, the source
user-data dir, using `internal/fspath.IsSameOrChild`. Keep that guard.

**DF3 — One instance at a time.** `runtime.AcquireLock` takes an exclusive
`flock` and returns `ErrAlreadyRunning`; both `start` and `start-background`
use it.

**DF4 — State paths are fixed and user-scoped.** BrowserOS state lives in
`~/.browseros-dogfood`; config and the imported profile live in
`~/.config/browseros-dogfood/` (honouring `XDG_CONFIG_HOME`). Config `Ports`
default to `{9015, 9115, 9315}` — deliberately distinct from the dev CLI's
9000/9100/9300 so the two never collide.

**DF5 — `start` never auto-pulls.** Updating the checkout is an explicit
`pull` or `restart --pull`; the README and `cmd/pull.go` both say so.

**DF6 — Keep build steps in `pipeline/`.** `Build` is exactly
`./tools/dev/setup.sh` then
`bun --cwd apps/agent --env-file=.env.development wxt build --mode development`.
`WriteProductionEnvFiles` writes `apps/server/.env.production` and
`apps/cli/.env.production` from config, keys sorted.

**DF7 — `daemon` is hidden.** It's the background worker behind
`start-background`; users get `status`/`stop`/`restart`/`logs`, not `daemon`.
Register user-facing verbs with a cobra `GroupID`; hidden commands don't need
one.

## Workflows

**First run:** `browseros-dogfood init` → answers repo path, BrowserOS
binary, source profile. Then `browseros-dogfood start` (foreground, Ctrl+C
stops) or `start-background` + `status` / `logs tail` / `stop`.

**Background loop:** `browseros-dogfood start-background` →
`status`, `pull` (ff-only), `restart [--pull] [--force]`, `logs`,
`logs tail`, `stop`. `cmd/control.go` polls the daemon over the IPC socket and
can follow the JSONL run log.

**Re-importing the profile:** `browseros-dogfood start --refresh-profile` (or
the standalone `refresh-profile` command). If the source profile is locked by
a running browser, the CLI asks you to quit BrowserOS; type `continue` to
override on stale locks.

**Adding a command:** 1. New `cmd/<name>.go` with `Use`/`Short`/`GroupID`
(`groupSetup`, `groupRun`, `groupInspect`). 2. `init()` → `rootCmd.AddCommand`.
3. Add `cmd/<name>_test.go`. 4. Document it in `README.md`.

**Running the Go tests:** `make -C tools/dogfood test` (`go test ./...`).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tools/` scope (rules TL1–TL6).
- [`README.md`](README.md) — the user-facing guide.
- [`../dev/AGENTS.md`](../dev/AGENTS.md) — the sibling dev orchestrator.
- [`../../package.json`](../../package.json) — `install:browseros-dogfood` → `make -C tools/dogfood install`.
- [`../../apps/cli/AGENTS.md`](../../apps/cli/AGENTS.md) — the real shipped Go CLI.
