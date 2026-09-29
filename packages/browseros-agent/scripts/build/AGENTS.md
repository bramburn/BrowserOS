# `scripts/build/` — compiled-server & CLI release pipeline

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

Two independent build entry points plus shared plumbing. `server.ts` runs the
production server artifact pipeline (bundle `apps/server/src/index.ts`, compile
per-target Bun binaries, stage resources, zip, upload to R2). `cli.ts` uploads
BrowserOS CLI installers and a full CLI release (binaries + `install.sh` /
`install.ps1` + checksums + release manifest) to the CDN. `log.ts` provides
the shared picocolors logger; `plugins/` holds the Bun WASM bundler plugin;
`config/` holds the resource manifest; `server/` and `cli/` hold the two
pipelines' internals.

## Contents

```
scripts/build/
├── server.ts              ← entry: runProdResourceBuild(process.argv.slice(2))
├── cli.ts                 ← entry: commander CLI (--release, --version, --binaries-dir)
├── log.ts                 ← log.{header,step,success,warn,error,info,done,fail}
├── config/
│   └── server-prod-resources.json  ← resource rules: {name, source{r2|local}, destination, os/arch, executable, recursive}
├── plugins/
│   └── wasm-binary.ts     ← Bun plugin: inlines `*.wasm?binary` as Uint8Array
├── server/
│   ├── orchestrator.ts    ← runProdResourceBuild: parse → config → compile → manifest → stage → archive → upload
│   ├── cli.ts             ← parseBuildArgs (--target, --manifest, --upload/--no-upload, --ci)
│   ├── targets.ts         ← 5 TargetIds → bunTarget + server binary name
│   ├── types.ts           ← BuildTarget, BuildArgs, R2Config, ResourceRule, StagedArtifact, UploadResult
│   ├── config.ts          ← loadBuildConfig: version + inlined env from apps/server/.env.production
│   ├── compile.ts         ← Bun.build + `bun build --compile` per target into dist/prod/server
│   ├── manifest.ts        ← loadManifest / getTargetRules (os + arch filtering)
│   ├── stage.ts           ← copy binary to resources/bin, apply manifest rules, write metadata
│   ├── metadata.ts        ← sha256 + size manifest for staged files
│   ├── archive.ts         ← zip via the `zip` binary; archiveAndUploadArtifacts
│   ├── r2.ts              ← S3Client, joinObjectKey, uploadFileToObject, downloadObjectToFile
│   ├── command.ts         ← runCommand spawn wrapper (stdio inherit, non-zero → throw)
│   └── stage.test.ts      ← bun test: manifest parsing + staging against a FakeS3Client
└── cli/
    ├── upload.ts          ← runCliInstallerUpload, runCliRelease, release manifest + checksums
    ├── config.ts          ← loadCliUploadConfig from apps/cli/.env.production
    └── upload.test.ts     ← bun test: parseCliChecksums, parseCliArchiveFilename, manifest build
```

## Rules

**SB1 — Two entry scripts, two purposes.** `bun run build:server*` →
`server.ts`; `bun run upload:cli-installers` → `cli.ts`. They share
`log.ts` and `server/r2.ts` and nothing else.

**SB2 — `--ci` and `--upload` are mutually exclusive.** `parseBuildArgs`
throws on the combination. `--ci` builds local zips only, requires no R2
config and no production secrets; a full build throws
`R2 configuration is required for full builds` without it.

**SB3 — Targets are a closed set.** `linux-x64`, `linux-arm64`,
`windows-x64`, `darwin-arm64`, `darwin-x64`, or `all`. Adding a platform
means adding a `TargetId` union member *and* a `TARGETS` entry, otherwise
typecheck fails.

**SB4 — Manifest rules are validated, not trusted.** `loadManifest` throws on
a missing `resources` array, an unsupported `source.type`, a rule with no
name/source/destination, or `recursive` on a non-`local` source.

**SB5 — Env comes from `apps/server/.env.production`.** `loadBuildConfig`
requires `BROWSEROS_CONFIG_URL`, `CODEGEN_SERVICE_URL`, `POSTHOG_API_KEY`,
`SENTRY_DSN` (process env wins over the file) and inlines them at build time
via `Bun.build`'s `define`. Never add a required secret without updating
`.env.production.example`.

**SB6 — Always `client.destroy()` in `finally`.** Both orchestrators do this;
a leaked S3 client hangs the build.

**SB7 — `zip` is a hard dependency of the archive step.** `archive.ts` shells
out to `zip -r -q` rather than using a library.

**SB8 — Don't hand-edit generated output.** `wasm-binary.ts` rewrites
`*.wasm?binary` imports into inline `Uint8Array` literals at bundle time; a
new WASM consumer must use the `?binary` suffix or it won't compile into the
binary.

## Workflows

**Building all server artifacts:** `bun run build:server`. Full path:
parse args → load config → `Bun.build` the server entry (minified, `define`d
env, `node-pty` external, wasm plugin) → `bun build --compile` per target →
load manifest → stage each target → zip → upload (unless `--no-upload`).

**CI build (no secrets, no R2):** `bun run build:server:ci` — same pipeline
but only `local` source rules are applied and nothing is uploaded.

**Adding a bundled runtime file:** 1. Upload the object to R2 via
`../../packages/build-tools` (`bun --filter @browseros/build-tools run upload`).
2. Add a rule with `source: {type: 'r2', key: '<relative key>'}` and the final
`destination` under `resources/bin/...` to `config/server-prod-resources.json`.
3. Add `os`/`arch` filters so it only lands in the artifacts that need it.
4. Confirm `R2_DOWNLOAD_PREFIX` matches the upload prefix.

**Publishing a CLI release:** `bun run upload:cli-installers -- --release
--version <v> --binaries-dir <dir>`. Without `--release` it uploads just the
installers. The archive pattern is
`browseros-cli_<version>_<os>_<arch>.(tar.gz|zip)` and is enforced by
`cli/upload.test.ts`.

**Adding a test for a new manifest behaviour:** 1. Add an `it(...)` to
`server/stage.test.ts` using `mkdtemp` + `afterEach` cleanup. 2. Use the
existing `FakeS3Client`; never hit the network.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `scripts/` scope (rules SR1–SR6).
- [`../package.json`](../../package.json) — the `build:server*` and `upload:cli-installers` scripts.
- [`server/AGENTS.md`](server/AGENTS.md) — the server pipeline internals.
- [`cli/AGENTS.md`](cli/AGENTS.md) — the CLI uploader internals.
- [`../../packages/build-tools/AGENTS.md`](../../packages/build-tools/AGENTS.md) — R2 upload client + Lima template.
- [`../../apps/server/AGENTS.md`](../../apps/server/AGENTS.md) — the build entrypoint `apps/server/src/index.ts`.
