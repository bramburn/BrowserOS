# `tools/patch/internal/workspace/` — checkout registry and state

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/internal/`](../AGENTS.md).

## What's here

Everything the tool remembers between invocations, in three places: the
user-level config and registry (`~/.config/browseros-patch/`), the
per-checkout state file inside each Chromium tree
(`.browseros-patch/state.yaml`), and the cwd → checkout detection that makes
commands work from any subdirectory of a registered checkout.

Four source files, all YAML. The one non-YAML persisted file in the tool is the
conflict state, which lives in `../resolve/`.

## Contents

| File | Contents |
|---|---|
| `config.go` | `Config{Version, PatchesRepo}`, `ConfigDir` (honours `XDG_CONFIG_HOME`, else `~/.config/browseros-patch`), `ConfigPath`, `RegistryPath`, `LoadConfig`, `SaveConfig` |
| `registry.go` | `Entry{Name, Path, AddedAt}`, `Registry{Version, Workspaces}`, `LoadRegistry`, `SaveRegistry`, `NormalizeWorkspacePath`, `Get`, `Add`, `Remove` |
| `detect.go` | `Detect`, `DetectForCommand`, `Resolve`, `ResolveForCommand`, `detectErrorMessage`, `canonicalPath` |
| `state.go` | `State{Version, Workspace, BaseCommit, LastApplyRev, LastSyncRev, LastExtractRev, PendingStash, LastApplyAt, LastSyncAt, LastExtractAt}`, `StateDir`, `StatePath`, `LoadState`, `SaveState` |
| `workspace_test.go` | tests for the above |

## Contents in detail — resolution order

`ResolveForCommand(reg, name, cwd, src, commandPath)` tries, in order:

1. `--src <path>` → normalised on the fly, not registered; the entry's name
   is the directory basename.
2. an explicit checkout name → `reg.Get(name)`.
3. otherwise → `DetectForCommand`, which picks the **longest** registered
   path that contains cwd, comparing both the cleaned and the
   symlink-resolved form.

## Rules

**WS1 — `NormalizeWorkspacePath` requires a real git checkout.** It rejects
a non-directory and a path without a `.git` entry, then stores the
symlink-resolved absolute path. Registering a non-checkout fails at `add`
time, not later during `apply`.

**WS2 — Names and paths are both unique.** `Add` refuses a duplicate name
*and* a duplicate resolved path, and keeps `Workspaces` sorted by name so
`list` and the error message are stable.

**WS3 — `ConfigDir` honours `XDG_CONFIG_HOME`.** On this Windows host the
effective path is `C:\Users\<user>\.config\browseros-patch`. Changing the
fallback breaks every existing installation; treat it as a compatibility
surface.

**WS4 — Both user-level files carry a `version` field defaulting to `1`.**
`LoadConfig` and `LoadRegistry` return `&Config{Version: 1}` /
`&Registry{Version: 1}` when the file is absent, and coerce `0` → `1` when
it is present but unset. `SaveConfig`/`SaveRegistry` write a one-line YAML
header comment above the body.

**WS5 — `State` lives inside the Chromium checkout, not in the user
config.** It is at `<checkout>/.browseros-patch/state.yaml` — inside the
50 GB tree, excluded from patch operations by `patch.IsInternalPath`, and
never committed. `LoadState` is the one loader that returns an empty state
on a missing file, because an unregistered checkout is a normal starting
point.

**WS6 — `PendingStash` is a git stash ref, and `""` is meaningful.** It
means "no stash outstanding". `engine.Sync` sets it, `engine.Abort` pops
it. Do not normalise it away or substitute a placeholder.

**WS7 — Detection resolves symlinks on both sides.** `canonicalPath` uses
`filepath.EvalSymlinks` and falls back to `filepath.Clean` on error. A
checkout reached through a junction or symlink only matches because of that
second comparison.

**WS8 — `workspace` imports stdlib plus `yaml.v3` only.** It must not
import `engine`, `git`, `patch` or `repo` (IN1 in the parent) — several
packages depend on it.

**WS9 — New state fields must be `omitempty` and default to zero.**
`workspace_test.go` round-trips the struct; an untagged field would be
emitted always and break that expectation.

## Workflows

**Registering a checkout**
1. `browseros-patch add ch1 /path/to/chromium/src` (alias `register`), with
   `--patches-repo` if you are not inside the patch repo.
2. `browseros-patch list` renders the name/path table.
3. `browseros-patch remove ch1` (alias `rm`) deregisters it; the directory
   and its `.browseros-patch/` state are left untouched.

**Running from inside a subdirectory of a checkout**
1. Any command with no name and no `--src` detects the containing checkout
   by longest-prefix match.
2. Outside every registered checkout, the error lists the registered names
   and their paths and suggests `browseros-patch <cmd> <name>` for the first
   one — a deliberate, copy-pasteable hint (`namedCheckoutExample`).

**Debugging a checkout that is not detected**
1. `browseros-patch list` — is it registered at all?
2. Is cwd really under the registered path, including through a symlink?
3. The registry stores the symlink-resolved path; if the checkout was moved
   or its link retargeted, re-run `add` (and `remove` the stale entry).

**Inspecting what the tool did to a checkout**
1. `<checkout>/.browseros-patch/state.yaml` — `last_apply_rev`,
   `last_sync_rev`, `last_extract_rev` and the three timestamps.
2. `<checkout>/.browseros-patch/resolve.json` — present only while a
   conflict is unresolved (see `../resolve/AGENTS.md`).
3. A `.rej` file beside a patched path indicates the last `apply` failed
   there.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — layering (IN7, IN8).
- [`../resolve/AGENTS.md`](../resolve/AGENTS.md) — `resolve.json`, the other
  file in the same directory.
- [`../engine/AGENTS.md`](../engine/AGENTS.md) — the writer of
  `LastApplyRev` / `LastSyncRev` / `LastExtractRev` / `PendingStash`.
- [`../app/AGENTS.md`](../app/AGENTS.md) — `Load`, `Save`,
  `ResolveWorkspace`.
- [`../../cmd/AGENTS.md`](../../cmd/AGENTS.md) — `resolveWorkspace` and the
  `--src` flag.
