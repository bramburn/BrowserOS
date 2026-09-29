# `tools/patch/internal/resolve/` — conflict-resume state

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/internal/`](../AGENTS.md).

## What's here

The state that lets a patch run survive the process boundary. When `apply`
hits a conflict it writes `resolve.json` into the checkout's
`.browseros-patch/` directory and stops. `continue`, `skip` and `abort` read
that file to resume, step past, or roll back — from a later shell, later that
day.

It is the only JSON-serialised file in the tool, and the only one whose
existence changes tool behaviour (`WorkspaceStatus.ActiveResolve`).

## Contents

`resolve.go` (98 lines) plus `resolve_test.go`:

| Symbol | Purpose |
|---|---|
| `type Operation` | `ChromiumPath`, `PatchRel`, `Op`, `OldPath`, `RejectPath`, `Message` |
| `type State` | `Workspace`, `RepoRoot`, `BaseCommit`, `RepoRev`, `Mode`, `Current`, `Operations`, `Resolved`, `Skipped` |
| `Path` / `Exists` / `Load` / `Save` / `Delete` | file handling at `<workspace>/.browseros-patch/resolve.json` |
| `FindActive(reg, cwd)` | picks the checkout with an active conflict |
| `(*State) CurrentOperation()` | bounds-checked accessor for `Operations[Current]` |

## Rules

**RS1 — The file is JSON, not YAML.** Every other persisted file in the tool
(`config.yaml`, `workspaces.yaml`, `state.yaml`) is YAML. `Save` uses
`json.MarshalIndent` with two-space indent and a trailing newline. Do not
"normalise" it to YAML.

**RS2 — `Load` returns an error when the file is absent.** Unlike
`workspace.LoadState`, there is no empty-state default — the caller is
expected to distinguish "no conflict" from "corrupt state". `Abort` and
`Continue` surface that error verbatim.

**RS3 — `Current` is an index into `Operations`, not a count.** It is
written by `engine.applyOperationRange` before returning, and read by
`CurrentOperation()`. If the operation list is rebuilt between runs, the
index must be rebuilt with it.

**RS4 — `Resolved` and `Skipped` are Chromium paths, not indices.** They
accumulate across the whole run so the final `ApplyResult` can report what
actually landed, even after several `continue` invocations.

**RS5 — `RepoRoot`, `RepoBaseCommit` and `RepoRev` are captured at conflict
time.** `Continue` reloads the repo from `State.RepoRoot` and does not
re-discover, so a moved or renamed patch repo does not silently change the
base mid-resolution. Re-run `abort` and `apply` if the repo moved.

**RS6 — `Delete` is the success path, not a cleanup step.**
`engine.Apply` removes the file when the last operation completes, and
`Abort` removes it after resetting. A leftover `resolve.json` means a run
was interrupted — do not delete it by hand to "unstick" the tool; use
`abort`.

**RS7 — `FindActive` refuses to guess when several checkouts have
conflicts.** It prefers the checkout containing cwd, falls back to the sole
active one, and otherwise errors with "run from inside the target checkout".
That ambiguity is deliberate; do not add a "pick the first" fallback.

**RS8 — `.browseros-patch/` is excluded from patch operations.**
`patch.IsInternalPath` makes `PathMatches` reject it, so this file can
never be picked up as a patch or a filter. Adding anything else to that
directory inherits the same exclusion automatically.

## Workflows

**Resolving a conflict**
1. `browseros-patch apply ch1` → stops, reports `result.Conflicts`.
2. Fix the file in the checkout by hand; a `.rej` file may sit beside it.
3. `browseros-patch continue` → `verifyResolved` rebuilds the patch set for
   just that path and requires the delta to be `UpToDate`; if it is not, it
   errors "current conflict is not resolved yet".
4. Repeat until the operation list is exhausted and `resolve.json` is
   deleted.

**Rolling back a bad run**
1. `browseros-patch abort` → resets every operation's path (and `OldPath`)
   to `State.BaseCommit`, deletes any `.rej`, removes `resolve.json`, and
   pops `state.PendingStash` if one was recorded.

**Debugging "no active conflict resolution found"**
1. `browseros-patch status ch1` shows `active_resolve` and `sync_state`.
2. `sync_state: conflicted` means the file exists; anything else means the
   run completed or was aborted.
3. The file lives at `<checkout>/.browseros-patch/resolve.json` — check that
   path directly before assuming a lost conflict.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — layering (IN7 for persisted files).
- [`../engine/AGENTS.md`](../engine/AGENTS.md) — the writer (EN5, EN6) and
  the `Continue` / `Skip` / `Abort` implementations.
- [`../workspace/AGENTS.md`](../workspace/AGENTS.md) — the
  `.browseros-patch/` directory and `state.yaml` beside this file.
- [`../patch/AGENTS.md`](../patch/AGENTS.md) — `IsInternalPath` excludes this
  directory.
