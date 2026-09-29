# `config/` — user config file

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> the Go CLI. This is the only on-disk state `cmd/` writes.

## What's here

One file, `config.go`, holding a single-field YAML config and the two
functions that read and write it: `Load` and `Save`. `Dir()` and
`Path()` are the only place in the CLI where a config path is computed —
everything else calls them. There is no `config_test.go`; the current
surface is small enough to be covered from `cmd/` tests.

## Contents

```
config/
└── config.go   ← Config{ServerURL}, Dir(), Path(), Load(), Save()
```

## Rules

### CFG1 — `Dir()` is the single source of truth for config paths
It returns `$XDG_CONFIG_HOME/browseros-cli` when that env var is set,
otherwise `~/.config/browseros-cli`. On Windows the XDG branch simply
never fires and the home path is used, which is the intended
behaviour — don't add a `%APPDATA%` branch. `Path()` appends
`config.yaml`. Other packages (`update/state.go`,
`analytics/analytics.go`) call `Dir()` rather than building their own
paths.

### CFG2 — The schema is one field: `server_url`
`Config` has exactly one member, `ServerURL string \`yaml:"server_url"\``.
That is deliberate — the browser port is discovered from BrowserOS, not
configured here. If you need another setting, justify it in a new
`AGENTS.md` revision rather than growing the struct by default.

### CFG3 — A missing config file is not an error
`Load` returns `&Config{}` and a `nil` error when the file doesn't
exist. This is what lets a fresh install run `browseros-cli init` without
a "no config" special case. Preserve it; only real read/parse failures
return an error.

### CFG4 — `Save` is a full rewrite, header included
`Save` creates the directory with `0755`, marshals the whole struct, and
writes `header + data` at `0644`, where the header is the two comment
lines pointing at `browseros-cli config --path`. Because it rewrites the
whole file, a key the user removed comes back as its zero value. That's
known and accepted — don't add a merge/preserve mode for one field.

### CFG5 — Errors are wrapped with `%w`, not `%v`
`Load` returns `fmt.Errorf("parsing config: %w", err)`. Keep the `%w`
so callers can inspect the underlying `*os.PathError`.

## Workflows

### "Adding a config field"
1. Add the field to `struct Config` with a `yaml:"snake_case"` tag.
2. Populate and read it in `cmd/`, not here — this package only stores
   bytes.
3. Re-run `browseros-cli config` on a machine with an old config file:
   `Save` will rewrite it and add the new key with its zero value.
4. Add a bullet to the config section of `../README.md`.

### "Adding a test for this package"
1. Create `config/config_test.go` in `package config`.
2. Point the test at a scratch dir with
   `t.Setenv("XDG_CONFIG_HOME", t.TempDir())` so `Dir()` resolves under
   `t.TempDir()` instead of the developer's real config.
3. Cover `Load`-on-missing (empty config, nil error), `Save` + `Load`
   round-trip, and the header line.

### "A user's saved URL is wrong"
`browseros-cli config --path` prints the file path without opening an
editor (see `cmd/config.go`); `browseros-cli init <url>` re-probes
`/health` and rewrites the file. Note that `defaultServerURL()` in
`cmd/root.go` intentionally ignores BrowserOS's runtime discovery file
so a saved URL is not silently overridden by another running server.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the Go CLI as a whole.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `init` and `config` commands, server-URL priority.
- [`../update/AGENTS.md`](../update/AGENTS.md) — stores `update-state.json` in the same `Dir()`.
- [`../analytics/AGENTS.md`](../analytics/AGENTS.md) — stores `install_id` in the same `Dir()`.
