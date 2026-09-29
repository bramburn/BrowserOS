# `build/docs/` — build-system documentation

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros/build`.

## What's here

Operational notes for people and agents running BrowserOS builds, as
opposed to reference material. There is currently one document:
`nightly-macos-ci.md`, which describes the self-hosted macOS runner
that produces signed nightly arm64 DMGs — its setup, its PATH
handling, the version-bump step, and the manual-dispatch escape
hatch.

Not here: API/architecture documentation (that belongs in the root
`AGENTS-*.md` files), CLI usage (the module docstrings and
`--help` output are the reference), or release process notes (those
live in the repo-level `docs/`).

## Contents

| File | Covers |
|---|---|
| `nightly-macos-ci.md` | The nightly macOS CI workflow: what it builds (`build/config/release.macos.arm64.yaml`), the `browseros-builder` self-hosted runner label, `launchd` service + keychain access requirements, the version bump, the `upload_to_r2=false` manual-dispatch option, and troubleshooting. |

## Rules

**DOC1 — Document the command that actually runs, verbatim.** The
canonical invocation in these files is
`uv run browseros build --config build/config/release.macos.arm64.yaml --chromium-src "$CHROMIUM_SRC"`.
If a config file is renamed or a flag changes, update the doc in the
same commit.

**DOC2 — This folder is for operations, not explanation.** "How do I
set up the runner", "why does codesign fail under `launchd`" — yes.
"What a module does" — that belongs in the module's own `AGENTS.md`.

**DOC3 — Keep secrets out.** Credentials are referenced by env var
name (`PROD_MACOS_NOTARIZATION_APPLE_ID`, `SPARKLE_PRIVATE_KEY`, …),
never by value. The GitHub registration token in the runner-setup
snippet is a placeholder for the reader to fill in.

**DOC4 — New build-system docs go here with a hyphenated name.**
`nightly-macos-ci.md` is the pattern: one workflow, one file, `.md`.

## Workflows

**A CI workflow changed its config file**
1. Edit the `.github/workflows/` file and the matching
   `../config/release.macos.arm64.yaml`.
2. Update the "What It Builds" command block in `nightly-macos-ci.md`.
3. Re-read the runner setup section; `runs-on:` labels and the
   `.path` file contents are the parts that silently rot.

**Adding docs for a new CI workflow**
1. Create `<workflow-name>.md` in this folder.
2. Cover: the exact build command, the runner it needs, the env vars
   it requires, and how to run it without publishing.
3. Link it from `../../../../AGENTS-build.md` if it is a primary entry
   point.

**Debugging a failed nightly**
1. Read the run's log.
2. Check the `<package_root>/logs/build_<timestamp>.log` file — see
   [`../../logs/AGENTS.md`](../../logs/AGENTS.md).
3. For keychain / `notarytool` failures, follow the `launchd` PATH and
   "user interaction not allowed" section of `nightly-macos-ci.md`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-system overview.
- [`../../../../AGENTS-build.md`](../../../../AGENTS-build.md) — full build pipeline narrative.
- [`../../../../AGENTS-ubuntu-dev.md`](../../../../AGENTS-ubuntu-dev.md) — the Linux LAN build host (the other CI path).
- [`../config/release.macos.arm64.yaml`](../config/release.macos.arm64.yaml) — the config this doc drives.
