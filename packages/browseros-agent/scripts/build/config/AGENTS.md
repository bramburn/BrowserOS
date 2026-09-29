# `scripts/build/config/` — server resource manifest

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/scripts/build`.

## What's here

One file: `server-prod-resources.json`, the default resource manifest for the
production server build (the `--manifest` default in
`../server/cli.ts`). It is data, not code — a list of rules that say which
files get copied into a compiled server artifact and where they land, sourced
either from Cloudflare R2 or from a local path, optionally filtered by target
OS and architecture.

## Contents

```
scripts/build/config/
└── server-prod-resources.json   ← { "resources": [ {name, source, destination, os?, arch?, executable?, recursive?} ] }
```

## Rules

**CF1 — A rule's `destination` is the path inside the artifact**, relative to
`resources/` — e.g. `resources/bin/third_party/lima/bin/limactl`. Getting this
wrong ships a file the server never looks for.

**CF2 — `source.type` is `r2` or `local`, nothing else.**
`../server/manifest.ts` `parseSource` throws
`Unsupported source type in manifest: <type>` for anything else. `local`
requires `path`; `r2` requires `key`.

**CF3 — R2 keys are relative**, not full object paths. Lima's is
`third_party/lima/limactl-darwin-arm64`; the `R2_DOWNLOAD_PREFIX`
(`artifacts/vendor`) is prepended at download time. Writing the prefix into
the manifest double-counts it.

**CF4 — `recursive: true` is only valid on `local` sources.**
`validateRule` throws
`Manifest rule <name> uses recursive with non-local source`.

**CF5 — Always set `os` and `arch` on a binary rule.** A rule with neither
applies to all five targets, and a macOS `limactl` shipped in a Linux zip is
a silent bug, not a build error.

**CF6 — `executable: true` sets the exec bit after copy** (skipped on
Windows). Mark the R2-sourced Lima binaries executable; the guest agents are
data and must not be.

**CF7 — Every rule needs a unique, human-readable `name`.** It is the only
thing in the error messages when staging fails.

## Workflows

**Adding a bundled runtime binary:** 1. Upload the object to R2
(`bun --filter @browseros/build-tools run upload -- --file … --key <key>`).
2. Add the rule with `source: {type: 'r2', key: <key>}`, the `destination`
under `resources/bin/…`, and `os`/`arch` filters. 3. Run
`bun run build:server:ci` and inspect the produced zip.

**Adding a local file to every artifact:** use `source: {type: 'local', path:
'relative/to/monorepo/root'}`, set `recursive: true` for a directory, and
leave `os`/`arch` unset.

**Validating a manifest change:** `bun test scripts/build/server/stage.test.ts`
exercises `loadManifest`; a rule the parser rejects fails there before it ever
reaches a build.

**Debugging "Manifest rule X is missing source path or destination":** the
rule has an empty `name`, or a `source` without `key`/`path`, or no
`destination`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `scripts/build/` scope (rules SB1–SB8).
- [`../server/manifest.ts`](../server/manifest.ts) — the parser and validator.
- [`../server/AGENTS.md`](../server/AGENTS.md) — how rules are applied at staging time.
- [`../../../packages/build-tools/AGENTS.md`](../../../packages/build-tools/AGENTS.md) — uploads the R2 objects this references.
