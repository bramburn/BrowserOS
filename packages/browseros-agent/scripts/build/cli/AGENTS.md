# `scripts/build/cli/` — BrowserOS CLI release uploader

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/scripts/build`.

## What's here

The internals behind `scripts/build/cli.ts`, which is wired to
`bun run upload:cli-installers`. It publishes the BrowserOS CLI to the CDN at
`https://cdn.browseros.com` on Cloudflare R2: either just the two installer
scripts, or a full release (every `browseros-cli_<version>_<os>_<arch>`
archive, a checksum file, and a JSON release manifest). It reuses
`../server/r2.ts` for the S3 client and object-key joining, and `../log.ts`
for output.

## Contents

```
scripts/build/cli/
├── upload.ts       ← runCliInstallerUpload, runCliRelease, buildCliReleaseManifest,
│                     parseCliArchiveFilename, parseCliChecksums, CliReleaseManifest types
├── config.ts       ← loadCliUploadConfig: R2 creds from apps/cli/.env.production (or CI process env)
└── upload.test.ts  ← bun test: checksum parsing, archive-filename parsing, manifest build
```

## Rules

**CL1 — The archive filename is a hard contract.**
`CLI_ARCHIVE_PATTERN` is
`/^browseros-cli_(?<version>[^_]+)_(?<os>darwin|linux|windows)_(?<arch>amd64|arm64)\.(?<ext>tar\.gz|zip)$/`.
Renaming artifacts in the release build breaks this regex, and
`upload.test.ts` locks the shape.

**CL2 — Two upload modes, chosen by the `--release` flag.** Without it,
`runCliInstallerUpload()` publishes only `install.sh` and `install.ps1` from
`apps/cli/scripts/`. With it, `--version` and `--binaries-dir` are both
mandatory and `runCliRelease` runs.

**CL3 — `config.ts` tolerates a missing `.env.production`.**
`loadProdEnv` returns `{}` when the file is absent so CI can supply
everything through `process.env`; it does *not* do that for the server build
config. Any required var still throws.

**CL4 — `uploadPrefix` defaults to `cli`.** `R2_UPLOAD_PREFIX` overrides it;
`downloadPrefix` is hard-coded to `''` for this uploader.

**CL5 — Checksum parsing is strict.** `parseCliChecksums` throws
`Invalid checksum line` on anything that is not
`<hex>[ *]<two spaces><filename>`; a malformed line aborts the release rather
than silently skipping a binary.

**CL6 — This is the `apps/cli` release path, not the Go CLI's own updater.**
`apps/cli` builds its own archives and this script publishes them; nothing in
`apps/cli` calls back into this folder.

## Workflows

**Publishing installers only:** `bun run upload:cli-installers` — uploads
`install.sh` (`text/x-shellscript`) and `install.ps1` (`text/plain`).

**Publishing a full release:** `bun run upload:cli-installers -- --release
--version 1.2.3 --binaries-dir dist/cli`. The script resolves the monorepo
root, `chdir`s there, enumerates the archives, verifies checksums, builds the
release manifest, and uploads.

**Adding a new platform triple:** 1. Add it to `CLI_ARCHIVE_PATTERN` and to
the `archive_format` handling. 2. Add a case to `upload.test.ts`. 3. Confirm
the Go release build actually produces that archive name.

**Debugging a skipped binary:** read the `parseCliArchiveFilename` error in the
output — the filename must match the pattern exactly, including the `_`
separators and the `.tar.gz` / `.zip` extension.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `scripts/build/` scope (rules SB1–SB8).
- [`upload.ts`](upload.ts) — the release logic.
- [`../server/AGENTS.md`](../server/AGENTS.md) — the shared `r2.ts` client.
- [`../../../../apps/cli/AGENTS.md`](../../../apps/cli/AGENTS.md) — the Go CLI being published.
- [`../../../../package.json`](../../../package.json) — the `upload:cli-installers` script.
