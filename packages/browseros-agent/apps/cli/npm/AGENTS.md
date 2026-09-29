# `npm/` — npm distribution wrapper

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> the Go CLI. This directory is the published npm package; the Go code
> it ships lives one level up.

## What's here

The `browseros-cli` npm package: manifest, a launcher shim, a
postinstall downloader, and the package README. It is **not** a Bun
workspace member — the monorepo `workspaces` field is
`["apps/*", "packages/*"]` and this is `apps/cli/npm`, two levels deep,
so `bun.lock` and the root `biome.json` do not apply to it. Published
from here by `make npm-publish`.

## Contents

```
npm/
├── package.json            ← name: browseros-cli, version: 0.2.0
├── README.md               ← the npm landing page (docs.browseros.com style)
├── .npmignore              ← excludes .binary/ and node_modules/
├── bin/
│   └── browseros-cli.js    ← launcher shim (see bin/AGENTS.md)
└── scripts/
    └── postinstall.js      ← release downloader (see scripts/AGENTS.md)
```

`package.json` is deliberately hand-maintained: `files` ships only
`bin/`, `scripts/`, and `README.md`; `os`/`cpu` restrict installs to
darwin/linux/win32 on x64/arm64; `engines.node` is `>=18`. There are no
dependencies and no dev dependencies — the wrapper uses `node:https`,
`node:fs`, `node:crypto`, and `node:child_process` only.

## Rules

### NPM1 — This package downloads a released binary; it never builds one
`postinstall.js` fetches
`releases/download/browseros-cli-v${VERSION}/browseros-cli_${VERSION}_${platform}_${arch}.{zip,tar.gz}`
and verifies it against that release's `checksums.txt`. The npm version
in `package.json` **is** the CLI version — `make npm-version VERSION=x`
rewrites it and `make npm-publish` depends on that target. A version
skew between here and the release tag breaks every install.

### NPM2 — The Go side and this side must agree on artefact names
The archive name is built from the same components as `../Makefile`'s
`release` target: `browseros-cli` + version + platform + arch +
`.zip` on Windows / `.tar.gz` elsewhere. If you change
`BINARY` or `PLATFORMS` in the `Makefile`, change the format string here
and in `../scripts/install.sh` and `install.ps1` in the same change.

### NPM3 — The wrapper sets `BROWSEROS_INSTALL_METHOD=npm`
`bin/browseros-cli.js` injects that env var into the child process.
`update/manager.go` reads it to disable automatic self-update for npm
users and to print `npm update -g browseros-cli` in the notice. Do not
rename the variable.

### NPM4 — CI must not download during install
`postinstall.js` exits 0 immediately when `process.env.CI` is set unless
`BROWSEROS_NPM_FORCE=1`. The launcher sets that flag when it needs to
self-heal a missing binary at runtime. Keep both branches.

### NPM5 — `.npmignore` is the release gate
It excludes `.binary/` and `node_modules/`. `package.json` `files` is
belt-and-braces. A downloaded binary must never end up in a published
tarball — if you add a scratch directory, add it here too.

## Workflows

### "Cutting a release"
1. `make release VERSION=x.y.z` — builds, packages, and writes
   `dist/checksums.txt` for all six platforms.
2. Publish the GitHub release at tag `browseros-cli-vx.y.z` with the
   archives from `dist/`.
3. `make npm-version VERSION=x.y.z` — rewrites `npm/package.json`.
4. `make npm-publish` (depends on `npm-version`).
5. Confirm `npm/package.json` version == the release tag == the version
   injected by `-ldflags -X main.version`.

### "Adding a platform or architecture"
1. Extend `PLATFORMS` in `../Makefile`.
2. Add the mapping to `PLATFORM_MAP` / `ARCH_MAP` in
   [`scripts/postinstall.js`](scripts/postinstall.js) (note: Node names
   `x64` → Go `amd64`).
3. Add it to `os` / `cpu` in `package.json` if it's a new OS.
4. Add the same platform handling to `../scripts/install.sh` and
   `../scripts/install.ps1`.
5. Add it to `PlatformKey`'s allow-list in [`../update/manifest.go`](../update/manifest.go).

### "Testing the wrapper locally"
`node npm/scripts/postinstall.js` with `BROWSEROS_NPM_FORCE=1` (and
`CI` unset) downloads into `npm/.binary/`. `node npm/bin/browseros-cli.js
--version` launches the shim and should print the same version. Both are
covered by `.npmignore` so nothing leaks into the tarball.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the Go CLI as a whole.
- [`../Makefile`](../Makefile) — `release`, `npm-version`, `npm-publish`.
- [`../README.md`](../README.md) — the Go-side install instructions this mirrors.
- [`../scripts/AGENTS.md`](../scripts/AGENTS.md) — the curl/PowerShell installer alternative.
- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
