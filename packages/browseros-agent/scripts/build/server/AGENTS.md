# `scripts/build/server/` — production server artifact pipeline

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/scripts/build`.

## What's here

The internals of the production server build, driven by `runProdResourceBuild`
in `orchestrator.ts`. It bundles `apps/server/src/index.ts`, compiles a Bun
single-file executable per target, copies resources into
`resources/bin/…` according to a JSON manifest, writes a sha256 metadata
file, zips each artifact, and uploads to Cloudflare R2. `stage.test.ts` is the
only test in this folder.

## Contents

```
scripts/build/server/
├── orchestrator.ts  ← runProdResourceBuild: the whole flow, `process.chdir(rootDir)` first
├── cli.ts           ← parseBuildArgs (commander, exitOverride → Error)
├── targets.ts       ← TARGETS map + resolveTargets('all' | 'a,b')
├── types.ts         ← TargetId, BuildTarget, BuildArgs, R2Config, ResourceRule/Manifest, StagedArtifact, UploadResult
├── config.ts        ← loadBuildConfig: version from apps/server/package.json, inlined env, R2 config
├── compile.ts       ← bundleServer (Bun.build) + compileTarget (`bun build --compile`)
├── manifest.ts      ← loadManifest + getTargetRules (os/arch filters), strict validation
├── stage.ts         ← stageCompiledArtifact / stageTargetArtifact: binary copy, rules, chmod 0o755
├── metadata.ts      ← recursive sha256 + size manifest
├── archive.ts       ← zipArtifactRoot (shells to `zip`), archiveAndUploadArtifacts
├── r2.ts            ← createR2Client, joinObjectKey, uploadFileToObject, downloadObjectToFile
├── command.ts       ← runCommand spawn wrapper
└── stage.test.ts    ← bun test for manifest parsing and staging
```

## Rules

**SS1 — Stage order is fixed.** Binary first (`resources/bin/browseros_server`
or `.exe`), then manifest rules, then `metadata.json`. Reordering changes the
artifact layout the Chromium-side loader expects.

**SS2 — Only non-Windows binaries get `chmod 0o755`.** `copyServerBinary`
branches on `target.os`.

**SS3 — `orchestrator.ts` chdirs to the monorepo root first**
(`resolve(import.meta.dir, '../../..')`). Every relative path below it —
`dist/prod/server`, `scripts/build/config/server-prod-resources.json` — is
relative to that root, not to this folder.

**SS4 — `--target` is parsed strictly.** `resolveTargets` throws
`Invalid target: <v>. Available: …` on anything outside the five ids plus
`all`.

**SS5 — Manifest `os` / `arch` filters are ANDed.** A rule with `os: ["macos"]`
and `arch: ["arm64"]` applies only to darwin-arm64. Omitting both means all
targets.

**SS6 — R2 keys are joined with `joinObjectKey`, not string concatenation.**
It trims leading/trailing slashes on every segment and drops empties, so
`downloadPrefix` + manifest key can't produce a double slash.

**SB5 applies here too:** required prod vars are `BROWSEROS_CONFIG_URL`,
`CODEGEN_SERVICE_URL`, `POSTHOG_API_KEY`, `SENTRY_DSN` (plus `NODE_ENV` and
`LOG_LEVEL` inlined). Missing `apps/server/.env.production` fails fast with a
pointer to `.env.production.example`.

## Workflows

**Full build for one target:**
`bun run build:server -- --target=darwin-arm64` (R2 config required; zips
upload unless `--no-upload`).

**Local-only, no secrets:** `bun run build:server:ci` — only `local` source
rules are applied, no R2 client is created, and each artifact is zipped for
inspection.

**Debugging a missing file in an artifact:** 1. Find the rule in
`../config/server-prod-resources.json`. 2. Check its `os`/`arch` filter
against `targets.ts`. 3. For `r2` sources, confirm the object exists at
`r2.downloadPrefix + source.key` (note: the key is the *relative* key, e.g.
`third_party/lima/limactl-darwin-arm64`).

**Adding a new resource field:** extend `ResourceRule` in `types.ts`, parse it
in `manifest.ts`'s `parseRule`, then use it in `stage.ts`. Every new field
needs a `stage.test.ts` case, because the manifest parser is strict.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `scripts/build/` scope (rules SB1–SB8).
- [`orchestrator.ts`](orchestrator.ts) — start here for the flow.
- [`../config/AGENTS.md`](../config/AGENTS.md) — the resource manifest.
- [`../cli/AGENTS.md`](../cli/AGENTS.md) — the CLI uploader (shares `r2.ts` and `log.ts`).
- [`../../../apps/server/AGENTS.md`](../../../apps/server/AGENTS.md) — the bundled entry point.
