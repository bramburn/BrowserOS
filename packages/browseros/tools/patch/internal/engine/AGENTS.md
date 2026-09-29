# `tools/patch/internal/engine/` — the operations

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/internal/`](../AGENTS.md).

## What's here

The behavioural core of `browseros-patch`. Seven files, one per verb plus
two tiny helpers. Every exported function has the same shape: an `Options`
struct carrying a `workspace.Entry`, a `*repo.Info`, filters, and a
`Progress`; and a `Result` struct with json tags that is the `--json`
contract.

## Contents

| File | Exported | What it does |
|---|---|---|
| `apply.go` (12.7 KB) | `Apply`, `Continue`, `Skip`, `Abort` | apply repo patches into a checkout; resume, skip or roll back a conflict |
| `extract.go` | `Extract` | write checkout changes back into `chromium_patches/` |
| `sync.go` | `Sync` | pull the patch repo, stash divergent files, re-apply, optionally rebase |
| `status.go` | `InspectWorkspace` | classify drift (`NeedsApply` / `NeedsUpdate` / `Orphaned` / `UpToDate`) and derive a `SyncState` string |
| `publish.go` | `Publish` | stage, commit and push `chromium_patches` |
| `progress.go` | `Progress`, `ProgressFunc` | one-method progress interface, nil-safe via `reportProgress` |
| `strings.go` | `plural` | trivial singular/plural helper |

Modes: `apply` reports `reset`, `changed` or `incremental`; `extract` reports
`commit`, `range` or `working-tree`; `sync` may run a `sync-reset` fallback.

## Rules

**EN1 — One exported function per verb, `Options` in / `Result` out.** Do not
add a variadic or flag-shaped API. `cmd/` builds the Options struct; the
engine decides nothing about CLI syntax.

**EN2 — `engine` must not import `ui` or `cmd`.** Styling and flag parsing
are outer layers. `engine` imports only `git`, `patch`, `repo`, `resolve`
and `workspace`. Keeping it styling-free is what makes `--json` trivial.

**EN3 — `Progress` may be nil.** Every function must route messages through
`reportProgress(opts.Progress, …)`, which returns early on nil. Never call
`opts.Progress.Step` directly.

**EN4 — Every apply resets the target file to `BASE_COMMIT` before
patching.** `applyOperationRange` calls `git.ResetPathToCommit` for both
`OldPath` and `ChromiumPath` before applying. Skipping this turns a stale
local edit into a silent patch failure.

**EN5 — One conflict stops the run and is persisted.** On a failed
operation, `apply.go` writes a `resolve.State` with the full operation list,
the current index, and the already-resolved/skipped paths, then returns
immediately. `Continue` / `Skip` / `Abort` all read that file. Do not
continue past a conflict in the same process.

**EN6 — `Sync` refuses to run with a dirty patch repo.** It calls
`git.IsDirty` first and returns "patches repo has uncommitted changes"
before touching the remote. Keep that guard first.

**EN7 — `Sync` stashes divergent files, not everything.** The stash
pathspec is `status.NeedsUpdate + status.Orphaned`; the stash ref is
persisted as `state.PendingStash` and popped by `Abort` and by `Sync`
(under `--rebase`). A whole-tree stash here would silently hide the
divergence the user is trying to resolve.

**EN8 — Result structs are a public contract.** Every field has a json tag;
`omitempty` marks fields that are only meaningful in some modes. Renaming a
field is a breaking change for anything parsing `--json`.

**EN9 — Keep `plural` and the progress helpers private.** They exist so
messages read naturally; they are not an API.

## Workflows

**Tracing an `apply` run**
1. `Apply` → `buildApplyOperations` (loads the repo patch set, compares
   against the checkout) → `applyOperationRange` (reset + `git.ApplyPatch`
   per operation) → `markApplyComplete`.
2. A conflict short-circuits into `resolve.Save` and returns the single
   conflicting `resolve.Operation` in `result.Conflicts`.
3. `cmd/apply.go` prints the conflict paths and the "run continue" hint.

**Adding a new operation**
1. Add `<verb>.go` with `XxxOptions`, `XxxResult`, and
   `func Xxx(ctx context.Context, opts XxxOptions) (*XxxResult, error)`.
2. Reuse `patch.Compare` + `patch.ScopeFromSet` for anything that reads or
   writes the repo patch set; do not re-implement drift detection.
3. Persist a "last run" marker in `workspace.State` so the next command can
   tell what happened.

**Debugging "patches repo has uncommitted changes"**
1. `Sync` runs `git.IsDirty` on the *repo* root, not the checkout.
2. Commit or stash the repo, then re-run. `Publish` is the normal way to
   clear it.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — layering (IN2, IN8).
- [`../../cmd/AGENTS.md`](../../cmd/AGENTS.md) — the command layer that
  constructs the Options.
- [`../patch/AGENTS.md`](../patch/AGENTS.md) — `Compare`, `ScopeFromSet`,
  `PathMatches`.
- [`../resolve/AGENTS.md`](../resolve/AGENTS.md) — the conflict state this
  package writes.
- [`../workspace/AGENTS.md`](../workspace/AGENTS.md) — `markApplyComplete`
  and `PendingStash`.
- [`../git/AGENTS.md`](../git/AGENTS.md) — `ApplyPatch`'s retry ladder.
