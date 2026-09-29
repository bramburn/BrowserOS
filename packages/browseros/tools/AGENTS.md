# `tools/` — standalone developer CLIs shipped with the package

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/`](../AGENTS.md).

## What's here

One subproject: `patch/`, the BrowserOS patch tool (`browseros-patch`), a
Go CLI that moves changes between the BrowserOS patch repo
(`chromium_patches/`) and one or more local Chromium checkouts.

It is the only Go code under `packages/browseros/` — the rest of the package
is the Python build CLI. Treat it as a separate build system: its own
`go.mod`, its own `Makefile`, its own `internal/` package layout, and
Go-specific rules (not the Python ones in the parent).

## Contents

```
tools/
└── patch/          ← browseros-patch: Go module, cobra CLI, own Makefile
```

## Rules

**TL1 — Go and Python rules do not mix.** Nothing in `tools/` is covered by
`pyproject.toml`, `pyrightconfig.json`, or the Typer CLI structure in
`../build/`. Do not add a Python file here expecting it to be importable, or
expect the Go module to be picked up by the Python test run.

**TL2 — `tools/` is not invoked by the Python build pipeline.** The build
phases in `../build/modules/` shell out to `git`, `gn`, `ninja`, `codesign`
— never to `browseros-patch`. The patch tool is a developer-side
counterpart, not a build step. A build will not fail if it is broken.

**TL3 — Keep the tool optional.** Nothing in the repo requires
`browseros-patch` to exist; `chromium_patches/` can be edited directly. Do
not make the build or `features.yaml` depend on it.

**TL4 — Version the binary via the `Makefile` `VERSION` variable, not a
hardcoded string.** `Makefile:12` injects it into `cmd.Version` with
`-ldflags`; a value committed in `cmd/root.go` would only apply to
`go run`.

**TL5 — Each subdirectory needs its own AGENTS.md.** `patch/`, `patch/cmd/`
and every `patch/internal/<pkg>/` are documented separately; read the
nearest one before editing.

## Workflows

**Building the tool**
1. `cd packages/browseros/tools/patch`
2. `make build` (produces `browseros-patch` in that directory) or
   `make install` (copies to `$GOBIN` / `$(go env GOPATH)/bin`).
3. `make test` runs `go test ./...`.

**Registering a Chromium checkout**
1. `browseros-patch add <name> <path>` from inside the patches repo, so
   `--patches-repo` is discovered; add `--patches-repo` explicitly
   otherwise.
2. `browseros-patch list` to confirm.

**Deciding where a change goes**
1. Edit the Chromium file in the checkout, then `browseros-patch extract`.
2. Or edit the file under `../../chromium_patches/` directly, then
   `browseros-patch apply`.
3. The parent package's `features.yaml` entry is still required either way.

## Cross-references

- [`patch/AGENTS.md`](patch/AGENTS.md) — the `browseros-patch` module.
- [`../AGENTS.md`](../AGENTS.md) — `packages/browseros/` (F1, F2: patch
  conventions the tool implements).
- [`../build/features.yaml`](../build/features.yaml) — the manifest `extract`
  output still has to be reflected in.
- [`../chromium_patches/AGENTS.md`](../chromium_patches/AGENTS.md) — the
  patch tree this tool reads and writes.
