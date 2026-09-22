# BrowserOS — Build Pipeline

> Companion to [`AGENTS-architecture.md`](AGENTS-architecture.md). This file
> documents **how the binary is produced end to end** — both the Chromium
> fork (Python CLI) and the Bun MCP server / extension (TS). Strategic
> context in [`AGENTS.md`](AGENTS.md); release surface in
> [`docs/CI_AND_RELEASES.md`](docs/CI_AND_RELEASES.md).
>
> **Captured 2026-09-22** from `bramburn/BrowserOS` fork at Chromium
> `148.0.7778.97` (`6b3fa66a92`) and `packages/browseros-agent` at MCP
> server `0.0.165`.

## TL;DR

Two parallel build surfaces:

| Surface | What it builds | Entry | Tooling | Output |
|---|---|---|---|---|
| **Chromium fork** | `BrowserOS.exe` (signed installer) | `packages/browseros/build/browseros.py` (Python CLI) | gclient → gn → ninja → sign → package | `BrowserOS_v<ver>_win-x64.exe` (Windows); `.deb` + AppImage (Linux dev) |
| **Bun monorepo** | `browseros-server.exe` + extension CRX | `packages/browseros-agent/apps/server/src/index.ts` + WXT | Bun compiler + Biome + WXT | Standalone MCP executable + WXT extension bundle |
| **Updates** | Omaha-4 XML + Sparkle RSS + JSON pointer | `tools/release/generate_update_manifests.py` + `.github/workflows/update-manifest.yml` | Python + GitHub Actions + R2 | `update_check.xml`, `appcast.xml`, `latest.json` on `cdn.bramburn.com` |

## Chromium fork — `packages/browseros/`

### Pipeline (5 phases)

```
$ browseros build --setup --prep --build --sign --package
```

| Phase | CLI invocation | Internally | Wall time (projected) |
|---|---|---|---|
| 1 setup | `browseros build --setup` | `clean` + `git_setup` + `sparkle_setup` | 30–90 min (DEPS sync is rate-limited on chromium.g.o) |
| 2 prep | `browseros build --prep` (or `--modules=…` if no R2 creds) | `resources` + `chromium_replace` + `string_replaces` + `patches` + `configure` | 5–15 min |
| 3 build | `browseros build --build -t release -a x64` | `compile` (autoninja) | 12–24 h on i7-3615QM; 7–13 h on a modern Windows desktop |
| 4 sign | `browseros build --sign` | `sign_windows` / `sign_macos` / `sign_linux` (linux is officially a no-op per `--list`) | 2–5 min on Windows; seconds on Linux |
| 5 package | `browseros build --package` | `package_windows` (mini_installer.exe) / `package_macos` (DMG) / `package_linux` (AppImage + .deb) | 1–3 min |

Module authority: `packages/browseros/build/modules/<area>/<step>.py`
under `setup/`, `patches/`, `apply/`, `extract/`, `compile/`, `feature/`,
`resources/`, `package/`, `sign/`, `ota/`, `release/`, `storage/`,
`extensions/`, `annotate/`.

### Orchestrators (per host)

| Host | Orchestrator | Detach |
|---|---|---|
| Windows | `tools/bramburn-build.ps1` (PowerShell) | `Start-Process -WindowStyle Hidden` |
| Ubuntu LAN box (`192.168.0.45`) | `tools/ubuntu-build.sh` (bash) | `setsid + nohup` via `tools/ubuntu-launch.sh` |
| GitHub Actions (hosted Ubuntu) | `.github/workflows/release-windows.yml` (self-hosted runner for the long build, ubuntu-latest for tests) | Native CI |

### Patch system — opinions

Patches are layered onto the vanilla Chromium checkout **on top of**
`packages/browseros/CHROMIUM_VERSION`:

1. **`chromium_patches/`** — 342 files mirroring the Chromium source tree
 path. Each file is a *complete replacement* for the corresponding
 Chromium file. Listed in `packages/browseros/build/features.yaml`
 under one or more feature blocks.
2. **`chromium_files/`** — Brand-new files dropped into the tree.
 Listed in `features.yaml` under feature blocks; no replacement of an
 existing file.
3. **`series_patches/`** — GNU-Quilt-ordered sequenced patches. Run
 separately: `browseros build -m series_patches`.
4. **`resources/`** — Icons, branding, entitlements, BROWSEROS_VERSION
 — *not* patched into tree, copied at build time.
5. **`chromium_replace/`** — Special module that replaces select files
 wholesale before patching (used for files where in-place patching is
 unreliable).

### Feature-flag manifest — `features.yaml`

`features.yaml` is the canonical manifest of what gets patched and why.
Each top-level feature block:

```yaml
features:
 <feature-id>:
 description: "<git-commit-style message>"
 files:
 - chrome/browser/browseros/<area>/<file>.cc
 - chrome/browser/browseros/<area>/<file>.h
 - ...
```

When `browseros dev annotate` runs, it groups by feature and creates
one git commit per feature block in the chromium source tree.

**Opinion**: when you add a patch, put it in the right feature block
immediately. Don't dump everything under `browseros-core` — features
should be atomic enough to revert individually.

### Where BrowserOS lives in the source tree

```
<chromium_src>/
├── chrome/browser/browseros/        ← BrowserOS C++ sources (added by chromium_patches/)
│ ├── BUILD.gn
│ ├── core/                          ← constants, prefs, switches
│ ├── extensions/                    ← bundled_ext loader
│ ├── metrics/                       ← BRA-like telemetry
│ └── server/                        ← bundled MCP server manager + IPC
├── chrome/browser/browsing_data/    ← BrowserOS-specific browsing data patches
├── components/update_client/        ← patches that point update_client at our R2
└── ...                                  ← 342 patches total
```

### Bundled MCP server — the gap

The actual MCP server binary (`browseros_server.exe`, versioned at
`server.json` in the Chrome-side checkout) is **precompiled** and pulled
from R2 at build time (`download_resources` module). The fork's
`packages/browseros/chromium_patches/chrome/browser/browseros/server/`
files just provide the C++ wrapper that:
- Spawns the subprocess at browser startup.
- Proxies IPC.
- Runs health checks + restart.

To replace `browseros_server.exe` itself, you need access to the
BrowserOS-private repo that builds it. The TS-based MCP server at
`packages/browseros-agent/apps/server` is the open-source answer; the
two protocols are similar but not identical.

### Build outcomes (what lands where)

```
~/browseros-build/
├── src/out/Default/                 ← autoninja output (~100 GB after build)
│ ├── chrome                        ← runnable binary
│ ├── mini_installer.exe           ← Windows installer (sign+package)
│ ├── *.dmg                        ← macOS
│ └── *.deb, *.AppImage            ← Linux
├── src/                            ← ~50 GB Chromium source + DEPS + patches
└── build.log, build-state.json     ← orchestrator artefacts
```

R2 layout after release:
```
browseros/
 <version>/
 BrowserOS_v<version>_win-x64.exe
 windows/
 update_check.xml                   ← Omaha-4 (in-browser updater)
 appcast.xml                        ← Sparkle RSS (browseros-cli)
 latest.json                        ← JSON version pointer
```

## Bun monorepo — `packages/browseros-agent/`

### Pipeline

```bash
# Install
bun install

# Develop
bun run start                     # MCP server on default port
bun run dev:watch -- --new        # MCP + extension HMR
bun run dev:server                # production server build (dev target)
bun run dev:ext                   # extension dev build

# Test
bun run test                      # full tool tests (requires BrowserOS running)
bun run test:tools                # MCP tool tests
bun run test:integration          # HTTP route integration
bun run test:sdk                  # agent SDK tests

# Lint + typecheck
bun run lint                      # Biome
bun run lint:fix
bun run typecheck

# Production
bun run dist:server               # production build, all targets
bun run dist:ext                  # production extension
bun run generate:models           # refresh models.dev data
```

### Outputs

```
packages/browseros-agent/apps/server/dist/
├── browseros-server-linux-x64/
├── browseros-server-linux-arm64/
├── browseros-server-darwin-x64/
├── browseros-server-darwin-arm64/
├── browseros-server-windows-x64/
└── browseros-server-windows-arm64/

packages/browseros-agent/apps/agent/.output/
├── chrome-mv3/                   ← WXT extension output
├── firefox-mv3/
└── safari-mv3/
```

The server output is uploaded to R2 as `artifacts/server/latest/`
plus versioned `artifacts/server/<version>/` paths.

## Update pipeline — `.github/workflows/update-manifest.yml`

After a Windows release PR closes:

1. The release workflow uploads `BrowserOS_v<ver>_win-x64.exe` to R2 +
 creates a GitHub Release tag `browseros-windows-v<ver>`.
2. `update-manifest.yml` triggers on `workflow_run`.
3. It downloads the .exe, computes SHA-256, runs
 `tools/release/generate_update_manifests.py` to emit:
 - `update_check.xml` — Omaha-4 (Chromium's update_client understands this).
 - `appcast.xml` — Sparkle-style RSS (`browseros-cli` and external tools read this).
 - `latest.json` — `{latest_version, ...}` (lightweight CLI checks).
4. Uploads all three to R2 under `browseros/windows/`.
5. The Chromium-side `components/update_client/configurator.cc` patch
 hard-codes the URLs at compile time — so a URL change requires a new
 release + rebuild.

Configurable via GitHub repo variables: `FORK_R2_PREFIX`, `FORK_CDN_BASE`,
`BROWSEROS_APP_ID`, `BROWSEROS_REPO_PATH`, etc. See
[`docs/CI_AND_RELEASES.md`](docs/CI_AND_RELEASES.md) for full list.

## Opinionated workflow — adding a build artifact

This is the meat. Each path describes the **minimum set of edits** required
to land a new component through every layer. Use these as checklists; open
the corresponding PR labelled `area/build/<topic>`.

### "I want to add a Chromium feature"

1. **Pick where it lives** — usually `chrome/browser/browseros/<area>/`
 (a folder under `chromium_patches/chrome/browser/browseros/`).
2. **Create the patch file(s)** with full replacement source.
3. **Add an entry to `packages/browseros/build/features.yaml`** under
 the right feature block (or create a new feature block). The `files:`
 list is the **only** way the patch lands.
4. **If it's a NEW file** (i.e. `<file>` doesn't exist in stock Chromium),
 also add it to `chromium_files/` and reference it from `features.yaml`.
5. **If it needs new Chromium flags / prefs**, add to
 `chromium_patches/chrome/browser/browseros/core/` and reference.
6. **Add a `BUILD.gn` entry** at `chromium_patches/chrome/browser/browseros/BUILD.gn`
 and any per-area `BUILD.gn`.
7. **Test build**: `tools/ubuntu-launch.sh --phase 2 3` (applies patches +
 incremental rebuild). Skip phase 1 (already done).
8. **Verify patch is applied**: `cd src && git log --oneline -- chrome/browser/browseros/<file>.cc`.
9. **Cross-link in `features.yaml` description** so the diff is human-readable.

### "I want to add an MCP tool"

1. **Create `apps/server/src/tools/<tool>.ts`** exporting `tool_<name>`
 (or `get_<name>` etc — see existing tools for naming).
2. **Tool framework**: extend `ToolDefinition` (`src/tools/framework.ts`).
 Use `src/tools/snapshot.ts` or `src/tools/dom.ts` as templates.
3. **Register** the import + export in `src/tools/registry.ts`.
 That's the canonical list — *Rule R10*. If your tool is in a family
 (e.g. filesystem), put it in `src/tools/<family>/<tool>.ts` and
 import via the family aggregator.
4. **Add fixtures** under `apps/server/tests/tools/__fixtures__/<tool>/`
 (HTML, JSON responses, etc.).
5. **Add a test** `apps/server/tests/tools/<tool>.test.ts`.
6. **Add a captured JSON schema** at
 `apps/server/tests/tools/schemas/<tool>.json` (auto-generated by
 `bun run test:tools`).
7. **Update `apps/server/tests/tools/__fixtures__/<tool>/index.ts`** if
 your tool needs a special harness.
8. **Run** `bun run test:tools` — must pass CI.
9. **Add an ACL scorer entry** if your tool is in `src/tools/acl/` —
 file naming is `acl-<feature>.ts`.
10. **Cross-reference** in `apps/agent/components/mcp-settings/` so the
 user sees the new tool in the extension.

### "I want to add an HTTP route"

1. **Create `apps/server/src/api/routes/<route>.ts`** exporting a Hono
 handler function `register<X>Routes(app: Hono)`.
2. **Register** in `apps/server/src/api/server.ts` — that's the
 canonical route catalog.
3. **Auth check**: use a helper from `apps/server/src/api/utils/request-auth.ts`.
4. **Validation**: Zod schemas at the top of the route file.
5. **Integration test**: `apps/server/tests/integration/<route>.integration.test.ts`.
6. **Run** `bun run test:integration`.

### "I want to add an LLM provider"

1. **Pick the SDK** — usually `@ai-sdk/*` (the Vercel AI SDK family).
2. **Create `apps/server/src/agent/provider-<name>.ts`** exporting a
 factory function `create<Name>Provider(config)`.
3. **Register in `apps/server/src/agent/provider-factory.ts`**.
4. **Add a config key** to `packages/browseros-agent/config.sample.json` and
 `packages/browseros-agent/apps/server/src/config.ts` `ServerConfigSchema`.
5. **Add to `apps/server/src/agent/models-dev.ts`** cache refresh
 (run `bun run generate:models`).
6. **Add UI** to `apps/agent/components/ai-settings/` so users can pick
 the new provider.

### "I want to add a DB schema change"

1. **Edit `apps/server/src/lib/db/schema/<table>.ts`** with the new
 table or column.
2. **Run** `bunx drizzle-kit generate` — produces
 `apps/server/src/lib/db/migrations/0001_<name>.sql` automatically.
3. **Verify the migration** — open the `.sql` file; Drizzle is good
 but double-check.
4. **Update repo methods** in `apps/server/src/lib/db/<repo>.ts`.
5. **Test** `bun run test:integration` — Drizzle applies migrations
 automatically in test setup.

### "I want to add an extension entry point (route)"

1. **Create the dir** `apps/agent/entrypoints/app/<route>/` containing
 `index.tsx` + component files.
2. **Update the entry manifest** — WXT auto-discovers, but check
 `wxt.config.ts` if any global rule applies.
3. **Add a sidebar / nav entry** in
 `apps/agent/components/sidebar/` and the relevant `entrypoints/app/layout/`.
4. **Add a use case in `lib/`** — keep side-effects in
 `lib/<feature>/` so it's reusable.
5. **Test** via the CDP inspector (`scripts/dev/inspect-ui.ts`,
 see `packages/browseros-agent/CLAUDE.md` § "Self-Testing UI Changes").

### "I want to add a feature flag"

Two scopes:

1. **Chromium-side** (compiled into the binary): add to
 `packages/browseros/build/features.yaml` under an existing feature or
 a new one. Re-run `browseros dev annotate` to convert edits to commits.
2. **Extension-side** (runtime toggle): add to
 `apps/agent/lib/llm-providers/<provider>/flags.ts`. Make it configurable
 in `components/ai-settings/`.

### "I want to change the update URL or R2 layout"

1. **Decide the new layout** — `browseros/...`, version prefixes, etc.
2. **Edit `tools/release/generate_update_manifests.py`** — `OAHAHA_*`
 constants.
3. **Update `tools/release/R2_LAYOUT.md`** (if it exists).
4. **Update Chrome-side `components/update_client/configurator.cc`
 patch** with new URLs.
5. **Trigger a full Chromium build + release** to ship the URL change.
6. **Test by shipping v1 + v2** (per AGENTS.md Priority #1 acceptance
 criteria: "ship v1 → ship v2 → restart-into-v2 with tabs intact").

## Quick reference — file paths

```
# Chromium side
packages/browseros/
├── CHROMIUM_VERSION                         ← version pin
├── BASE_COMMIT                              ← commit pin
├── features.yaml                            ← PATCHES MANIFEST
├── build/
│ ├── browseros.py                           ← Typer app entry
│ ├── modules/<area>/<step>.py               ← phase implementations
│ ├── common/context.py                      ← Context class
│ ├── common/module.py                       ← CommandModule base
│ ├── common/utils.py                        ← shared helpers (PATHs, OS check)
│ └── cli/build.py                           ← `browseros build` subcommand
├── chromium_patches/chrome/browser/browseros/
│ ├── BUILD.gn
│ ├── core/                                  ← BrowserOS core (prefs, constants)
│ ├── extensions/                            ← bundled-ext loader
│ ├── metrics/                               ← telemetry
│ └── server/                                ← bundled MCP server manager
├── chromium_files/                          ← new files on top of Chromium
├── series_patches/                          ← GNU-Quilt ordered patches
└── resources/                               ← icons, branding

# Bun side
packages/browseros-agent/
├── apps/server/src/
│ ├── index.ts                               ← bun entry
│ ├── main.ts                                ← Application class
│ ├── config.ts                              ← zod-validated config
│ ├── tools/registry.ts                      ← tool catalog (R10)
│ ├── agent/provider-factory.ts              ← LLM providers
│ ├── api/server.ts                          ← Hono app
│ ├── api/routes/                            ← HTTP routes
│ ├── lib/db/                                ← Drizzle + SQLite
│ └── browser/                               ← CDP client
├── apps/agent/
│ ├── entrypoints/<name>/                    ← extension entry points
│ ├── components/<area>/                     ← React UI
│ ├── lib/<feature>/                         ← feature modules
│ └── wxt.config.ts                          ← WXT config
└── packages/shared/src/constants/           ← R2 source-of-truth
```

## Cross-references

- [`AGENTS-architecture.md`](AGENTS-architecture.md) — overall map.
- [`AGENTS.md`](AGENTS.md) — strategic roadmap + scope.
- [`AGENTS-toolchain.md`](AGENTS-toolchain.md) — Ubuntu toolchain.
- [`AGENTS-ubuntu-dev.md`](AGENTS-ubuntu-dev.md) — dev workflow.
- [`docs/CI_AND_RELEASES.md`](docs/CI_AND_RELEASES.md) — CI + release.
- [`packages/browseros/AGENTS.md`](packages/browseros/AGENTS.md) — Chromium fork view.
- [`packages/browseros-agent/AGENTS.md`](packages/browseros-agent/AGENTS.md) — Bun monorepo view.
