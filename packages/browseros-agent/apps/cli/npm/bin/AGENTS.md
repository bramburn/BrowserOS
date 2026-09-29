# `npm/bin/` — launcher shim

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> the Go CLI. One file: the executable npm installs as
> `browseros-cli`.

## What's here

`browseros-cli.js` is a ~30-line CommonJS shim registered as the npm
`bin` entry in [`../package.json`](../package.json). It resolves the
platform-specific binary at `<npm>/.binary/browseros-cli[.exe]`, and
if it is missing, re-runs `scripts/postinstall.js` with
`BROWSEROS_NPM_FORCE=1` to download it. Otherwise it `spawnSync`s the
binary with the user's argv, `stdio: 'inherit'`, and exits with the
child's status.

## Rules

### BIN1 — Zero dependencies; use `node:` builtins only
`node:child_process`, `node:path`, `node:fs`. The published package has
no `dependencies` and must not gain any — a broken transitive dep
breaks every `npx browseros-cli`.
### BIN2 — Set `BROWSEROS_INSTALL_METHOD=npm` on the child
That env var is what `../update/manager.go` reads to disable
self-update for npm users and to print the correct upgrade command. If
it's missing, the CLI will try to replace its own binary inside
`node_modules`, which is wrong.
### BIN3 — `spawnSync`, not `execSync`
argv must be passed as an array so user arguments containing spaces or
shell metacharacters are not re-parsed by a shell. Keep `stdio:
'inherit'` so `--json` output stays the only thing on stdout.
### BIN4 — Propagate the real exit code
`process.exit(result.status ?? 1)`. The Go binary's exit codes (1/2/3 —
see `cmd/AGENTS.md`) are a documented contract; collapsing them to 1
breaks scripts.
### BIN5 — Self-heal, don't crash
A missing `.binary/` triggers one `execFileSync` of
`scripts/postinstall.js` with `BROWSEROS_NPM_FORCE=1`. On failure the
shim prints a `npm install -g browseros-cli` hint and exits 1. Keep
this path — it is what makes `npx browseros-cli` work on a cold cache.
### BIN6 — `EXT` is the only platform branch here
`process.platform === 'win32' ? '.exe' : ''`. The real platform/arch
matrix lives in `../scripts/postinstall.js`; don't duplicate it.

## Workflows

- **Verifying the shim:** `node bin/browseros-cli.js --version` should
  print the version baked in at release time. A mismatch means
  `BROWSEROS_INSTALL_METHOD` is fine but the binary in `.binary/` is
  stale — delete it and re-run the postinstall.
- **Adding a global flag:** do it here. Add it to the flags you forward
  in `../cmd/root.go` and document it in the table in `../README.md`.
- **Changing the binary name:** the `browseros-cli` literal here must
  match `BINARY` in `../../Makefile` and the `Binary` variable in
  `../../scripts/install.ps1`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the npm package.
- [`../package.json`](../package.json) — the `bin` entry that points here.
- [`../scripts/AGENTS.md`](../scripts/AGENTS.md) — the downloader this shim invokes.
- [`../../AGENTS.md`](../../AGENTS.md) — the Go CLI.
