# `tools/patch/internal/repo/` — patch-repo discovery

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/internal/`](../AGENTS.md).

## What's here

One file, `repo.go`, 61 lines. It answers two questions: *is this directory
the BrowserOS patch repo?* and *what is in it?* The answer is a single
`Info` struct plus a discovery walk.

## Contents

`repo.go`:

| Symbol | Purpose |
|---|---|
| `type Info struct` | `Root`, `PatchesDir` (= `<root>/chromium_patches`), `BaseCommit` |
| `Discover(start) (string, error)` | walks up from `start` until `hasRepoMarkers` is true |
| `Load(root) (*Info, error)` | validates markers, reads and trims `BASE_COMMIT` |
| `hasRepoMarkers(root) bool` | requires **both** `BASE_COMMIT` (a file) and `chromium_patches/` (a directory) |

## Rules

**RP1 — Two markers, both required.** A `BASE_COMMIT` without
`chromium_patches/`, or a `chromium_patches/` without `BASE_COMMIT`, is not
a patches repo. That check is the only thing preventing the tool from
operating on some unrelated git checkout.

**RP2 — `BaseCommit` is trimmed whitespace from the file.** Every reset in
`engine.applyOperationRange` runs `git checkout <BaseCommit> -- <path>`
against the *Chromium* checkout, so a trailing newline would become part of
the ref and fail. Never write `BASE_COMMIT` with padding and expect it to
survive.

**RP3 — `PatchesDir` is derived, never configured.** It is
`filepath.Join(root, "chromium_patches")`; there is no option to point the
tool at a differently-named patch tree. A rename of `chromium_patches/`
requires changing this file and `chromium_replace`-adjacent build config
together.

**RP4 — `Discover` walks up from cwd, not down.** It resolves `abs(start)`
once, then loops `filepath.Dir` until the filesystem root. It does not
consult the git remote or the `go.mod` module path — the markers are the
only signal.

**RP5 — `Discover` never caches.** `cmd/common.go`'s `ensureRepoConfigured`
and `app.RepoInfo` both call it, then write the result into
`Config.PatchesRepo` and persist it. A one-time configuration beat the
repeated walk; keep it that way.

**RP6 — `repo` imports stdlib only** (`fmt`, `os`, `path/filepath`,
`strings`). Adding a dependency here would drag it into every command's
startup path (IN1 in the parent).

## Workflows

**Pointing the tool at a fork checkout**
1. `browseros-patch add <name> <src> --patches-repo <path>` from anywhere —
   the flag is stored in `config.yaml` and takes precedence over discovery.
2. The path must contain `BASE_COMMIT` and `chromium_patches/`.
3. `browseros-patch list` confirms the checkout; the repo path is implicit
   in the config.

**Debugging "not a browseros patches repo"**
1. Check `BASE_COMMIT` exists at the root you passed — in
   `packages/browseros/` it is the sibling of `chromium_patches/`.
2. Check `chromium_patches/` is a directory, not a symlink to a missing
   target (`os.Stat` follows symlinks, so a dangling one reports absent).
3. If running from inside the repo, `Discover` should have found it —
   otherwise cwd is not under the repo root (a symlinked path that
   `filepath.Abs` does not resolve can defeat the walk).

**Changing the base commit**
1. Edit `BASE_COMMIT` in the patch repo.
2. Every checkout's `.browseros-patch/state.yaml` now records a different
   `base_commit`; `engine.Sync` detects the mismatch and falls back to a
   full `sync-reset` apply (the `Fallback` field in `SyncResult`).
3. Expect a conflict storm on the first sync after a bump; resolve with
   `continue`/`skip` or `abort`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — layering (IN1, IN6).
- [`../app/AGENTS.md`](../app/AGENTS.md) — `RepoInfo()`, the caller that
  caches `Root` into the config.
- [`../engine/AGENTS.md`](../engine/AGENTS.md) — every `ResetPathToCommit`
  uses `Info.BaseCommit`.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `packages/browseros/`,
  where `BASE_COMMIT` and `chromium_patches/` live.
