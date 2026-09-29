# `tools/dogfood/config/` — dogfood config.yaml

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

The on-disk configuration for the dogfood CLI, stored as YAML at
`~/.config/browseros-dogfood/config.yaml` (or `$XDG_CONFIG_HOME/browseros-dogfood/`).
`config.go` defines the `Config` struct, loads and saves it, resolves derived
paths, validates it, and supplies the production-env defaults the pipeline
writes into the checkout.

## Contents

```
tools/dogfood/config/
├── config.go       ← Config, Ports, ProductionEnv; Path, DefaultConfigDir, Defaults, Load,
│                     Save, Resolve, Validate, AgentRoot, SourceProfilePath, DevProfilePath,
│                     LogDir, LogPath, ExpandTilde, DefaultProductionEnv, FillProductionEnvDefaults
└── config_test.go  ← round-trip and validation tests
```

## Rules

**CC1 — Config lives in XDG, not the home dir directly.** `DefaultConfigDir`
prefers `$XDG_CONFIG_HOME/browseros-dogfood`, else
`~/.config/browseros-dogfood`. BrowserOS *state* (`~/.browseros-dogfood`) is
a different directory owned by this package's callers, not the config.

**CC2 — Default ports are 9015/9115/9315.** They must not collide with the
dev CLI's 9000/9100/9300 (`../../dev/proc/ports.go`) or with
`DEFAULT_PORTS` in the shared package.

**CC3 — `Validate()` is the gate.** It checks the repo path really is a
BrowserOS checkout (`validateRepo`) before any command proceeds; `Load` +
`Validate` is the pair every command uses, and `loadConfigWithoutValidation`
is only for read-only commands like `logs`.

**CC4 — `FillProductionEnvDefaults` must run before writing env files.**
`../pipeline/env.go` calls it first, then writes `apps/server/.env.production`
and `apps/cli/.env.production` with sorted keys.

**CC5 — `ExpandTilde` handles `~` in user-supplied paths** so
`source_user_data_dir: ~/Library/…` in the YAML resolves. New path fields must
go through it.

**CC6 — `LogDir`/`LogPath` are derived, not configured.** Log locations follow
the config dir; don't add a `log_dir` YAML key.

**CC7 — Adding a YAML field:** add it to `Config` with a `yaml:"snake_case"`
tag, add a default in `Defaults(home)`, and extend `Validate` if it needs one.
An untagged field is silently ignored on load.

## Workflows

**Editing config by hand:** `browseros-dogfood config edit` opens
`config.Path()` in `$EDITOR`. Keep the field names and defaults in
`Defaults` and `config_test.go` in sync with what the README documents.

**Pointing dogfood at a different checkout:** edit `repo_path` (or re-run
`init`). `AgentRoot()` derives the `packages/browseros-agent` sub-path from
it, and `validateRepo` rejects anything that isn't the expected layout.

**Adding a port:** extend `Ports` with a `yaml:"…"` tag, default it in
`Defaults`, and update `ResolvePorts` in `../proc/ports.go` and
`../browser/args.go` so the flag is actually passed to the browser.

**Debugging "repo path is not a BrowserOS checkout":** run
`validateRepo`'s check manually — it looks for the agent package layout under
`repo_path`, not just a `.git` directory.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope (rule DF4 covers paths).
- [`config.go`](config.go) — the whole config surface.
- [`../pipeline/AGENTS.md`](../pipeline/AGENTS.md) — writes `.env.production` from `ProductionEnv`.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `init` / `config edit` callers.
- [`../README.md`](../README.md) — the Config section for users.
