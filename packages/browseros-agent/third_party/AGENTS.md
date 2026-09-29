# `third_party/` — vendored third-party binaries

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

Vendored, unmodified third-party build tooling. It holds exactly one thing:
`bin/`, containing a single Windows executable, `rcedit-x64.exe` (~1.36 MB) —
the Electron `rcedit` tool, used to stamp Windows version-resource metadata
(ProductName, CompanyName, LegalCopyright, …) into a built `.exe`.

This is a checked-in binary dependency, not source. No Bun script compiles
anything here, Biome does not read it, and no test asserts on it. Its only
consumer is [`../scripts/patch-windows-exe.ts`](../scripts/patch-windows-exe.ts).

## Contents

```
third_party/
└── bin/
    └── rcedit-x64.exe    ← 1.36 MB, Windows x64 rcedit binary
```

## Rules

**TP1 — Do not modify, replace, or re-upload anything in this directory.**
These are upstream binaries vendored verbatim. There is no source here to
patch and no build step to regenerate them from.

**TP2 — Do not delete it.** The only consumer,
[`../scripts/patch-windows-exe.ts`](../scripts/patch-windows-exe.ts),
resolves `third_party/bin/rcedit-x64.exe` relative to itself and hard-fails
with `Error: rcedit binary not found at: …` if it is missing. A Windows
release build breaks at the metadata-patching step.

**TP3 — Adding a new vendored binary requires a consumer.** Put it under
`third_party/bin/`, name it with the platform and architecture
(`<name>-<arch>.exe`), and wire it from a script that resolves the path the
same way — `path.resolve(__dirname, '..', 'third_party', 'bin', …)`.

**TP4 — This is not the same `third_party` as the server resources tree.**
`CLAUDE.md` and `packages/build-tools/README.md` refer to
`resources/bin/third_party/{lima,bun}/…` — a *staged runtime* layout built by
`packages/build-tools` (resolved in code by
`apps/server/src/lib/vm/paths.ts` and `apps/server/src/lib/agents/bundled-bun.ts`).
Those paths are not this directory, and a binary added here is not
automatically shipped in the server resource bundle.

**TP5 — Non-Windows hosts need Wine.** `patch-windows-exe.ts` runs
`rcedit-x64.exe` directly on `win32` and through `wine` elsewhere; on macOS
that means `brew install --cask wine-stable`. Don't assume a Linux CI runner
can execute it directly.

**TP6 — Bump it by re-vendoring, with provenance noted in the PR.** Record the
upstream project and version in the commit message. A binary diff with no
provenance is unreviewable.

## Workflows

**Patching a built Windows executable**
1. `bun scripts/patch-windows-exe.ts <path-to-exe>`.
2. The script sets `ProductName: BrowserOS Agent`, `FileDescription`,
   `CompanyName: BrowserOS`, `LegalCopyright: Copyright (C) 2025 BrowserOS`,
   `InternalName: browseros-server`, `OriginalFilename: <basename>`.
3. On non-Windows it invokes `wine` with `WINEDEBUG=-all`; a missing Wine is
   reported as `Error: Wine is not installed`.
4. Success prints `✓ Successfully patched Windows executable metadata`.

**Debugging "rcedit binary not found"**
1. Confirm `third_party/bin/rcedit-x64.exe` is present (~1.36 MB — a stub is a
   corrupted checkout, not a valid file).
2. Confirm the build is not running from a packaged/copied location where
   `../third_party/bin/` was not staged.
3. Confirm the process has execute permission on Windows (Antivirus /
   Mark-of-the-Web will block a freshly downloaded binary).

**Vendoring a new tool**
1. Place it in `third_party/bin/` with a `<name>-<arch>` suffix.
2. Update or add a script that resolves it from `__dirname/../third_party/bin/`.
3. State upstream project + version in the commit message.

## Cross-references

- [`bin/AGENTS.md`](bin/AGENTS.md) — the single vendored binary.
- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`../scripts/patch-windows-exe.ts`](../scripts/patch-windows-exe.ts) — the
  only consumer of `rcedit-x64.exe`.
- [`../CLAUDE.md`](../CLAUDE.md) — § "Release gating" describes the
  `resources/bin/third_party/` runtime staging (a different tree; see TP4).
- [`../packages/build-tools/README.md`](../packages/build-tools/README.md) — the resource-staging manifest keys.
