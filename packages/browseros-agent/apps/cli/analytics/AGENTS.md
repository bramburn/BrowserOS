# `analytics/` — PostHog command telemetry

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> the Go CLI. Optional, off by default, and entirely absent from local
> builds.

## What's here

Two files. `analytics.go` wraps the PostHog Go client behind three
package functions — `Init(version)`, `Track(command, success,
duration)`, `Close()` — and resolves a stable anonymous distinct ID.
The PostHog API key is **not** in the source: it is a package-level
empty string that the `Makefile` fills at link time. There is one
event, sent once per invocation.

## Contents

```
analytics/
├── analytics.go      ← Init, Track, Close, resolveDistinctID,
│                       loadBrowserosID, loadOrCreateInstallID, generateUUID
└── analytics_test.go ← UUID shape, server.json parsing, install_id reuse
```

## Rules

### AN1 — `posthogAPIKey` is a `-ldflags -X` target and must stay empty in source
`Makefile` passes
`-X browseros-cli/analytics.posthogAPIKey=$(POSTHOG_API_KEY)`. A plain
`make` leaves `POSTHOG_API_KEY` unset, the var stays `""`, and `Init`
returns immediately — local and dev builds send nothing. Never commit a
real key, and never move the key into `config.yaml` or an env var read
at runtime; that would ship it to every user.

### AN2 — Every exported function must be a no-op when `svc == nil`
`Init` returns early on empty key, unresolvable ID, or client error.
`Track` and `Close` check `svc == nil` first. This is what makes the
whole package safe to call unconditionally from `cmd/root.go`'s
`Execute()`, which wraps every command. Preserve the nil checks when
adding a function.

### AN3 — The event schema is fixed: one event, four properties
`Track` sends `browseros.cli.command_executed` with `command` (the cobra
`CommandPath`, e.g. `browseros-cli bookmark search`), `success`
(bool), and `duration_ms`. `DefaultEventProperties` in `Init` adds
`cli_version`, `os`, and `arch` to every event. The payload sets
`$process_person_profile: false` — this is anonymous, per-invocation
telemetry, not a person profile. Don't add a second event, a new
default property, or a person profile without a deliberate decision.

### AN4 — `command` is a command path, never an argument
`commandName(os.Args[1:])` resolves through `rootCmd.Find` and returns
`"unknown"` for an unresolvable invocation. Raw user arguments, URLs,
element IDs, and page contents are never sent. Keep it that way: if you
find yourself wanting a more specific event, add a *field on the
command*, not the argument string.

### AN5 — Distinct ID resolution order is fixed
`resolveDistinctID` prefers `~/.browseros/server.json`'s
`browseros_id` (the BrowserOS install identity) and falls back to a
generated UUID v4 cached at `config.Dir()/install_id`. Don't introduce
a third source, and don't derive anything from the server URL or
hostname — the ID must not be correlatable to a machine name.

## Workflows

### "I need a new event"
1. Re-read AN3 first — one event per invocation is the current design.
2. If it is genuinely needed, add a `Track*` function alongside `Track`
   that starts with the `svc == nil` guard.
3. Add the function to the one call site in `cmd/root.go`'s
   `Execute()`, wrapped so the result is still reported if it fails.
4. Keep the `browseros.cli.` prefix from `eventPrefix`.

### "Nothing is being reported in my build"
1. Confirm the key was injected: `make POSTHOG_API_KEY=<key>`. Without
   it `Init` returns at AN1's first line.
2. Confirm the version is a release version — that's checked in
   `update/`, not here, but a `"dev"` build is a common cause of "the
   CLI is quiet".
3. Confirm a distinct ID resolved: if neither `~/.browseros/server.json`
   nor `config.Dir()/install_id` is writable, `Init` bails at AN5.

### "Adding a test"
1. Stay in `package analytics` — the tests exercise unexported
   functions (`generateUUID`, `loadBrowserosID`).
2. Redirect the filesystem with `t.Setenv("HOME", t.TempDir())` for the
   `server.json` path and `t.Setenv("XDG_CONFIG_HOME", t.TempDir())`
   for the `install_id` path.
3. Never construct a `service` with a real key; the tests never call
   `Init` with a key.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the Go CLI as a whole.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `Execute()` is the only caller (`Init`, `Track`, `Close`).
- [`../config/AGENTS.md`](../config/AGENTS.md) — `Dir()`, where `install_id` is stored.
- [`../Makefile`](../Makefile) — the `POSTHOG_API_KEY` ldflags wiring.
