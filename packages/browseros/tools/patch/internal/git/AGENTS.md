# `tools/patch/internal/git/` — typed wrappers over the `git` binary

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/internal/`](../AGENTS.md).

## What's here

The only place in the module that executes `git`. There is no Go git
library — `Run` spawns the `git` binary with `exec.CommandContext` and
returns stdout, stderr and the exit code as a value. Every other function in
this package is a typed wrapper that converts a non-zero exit into a Go
`error`.

## Contents

`git.go` (387 lines):

| Group | Functions |
|---|---|
| Core | `Run`, `HeadRev`, `CurrentBranch`, `IsDirty`, `IsDirtyPaths`, `CommitExists`, `ListUntracked` |
| Object access | `FileExistsAtCommit`, `ShowFile`, `CheckoutFiles`, `ResetPathToCommit` |
| Diffs | `DiffText`, `DiffNoIndex`, `DiffNameStatusBetween`, `DiffTreeNameStatus`, `RevListRange`, `runNameStatus` |
| Mutation | `ApplyPatch`, `StashPush`, `StashPop`, `PullRebase`, `AddPaths`, `Commit`, `Push` |
| Types | `Result{Stdout, Stderr, Code}`, `FileChange{Status, Path, OldPath}` |

Plus `git_test.go`.

## Rules

**GT1 — Never shell out to git from another package.** The retry ladder,
the error extraction and the exit-code handling live here and must stay in
one place (IN3 in the parent).

**GT2 — `Run` distinguishes process failure from a non-zero exit.**
`err != nil` with a `ProcessState` (git ran and said no) returns
`(result, nil)`; the caller inspects `result.Code`. A genuine exec failure
returns an error. Do not collapse the two — callers branch on `Code` to
build the stderr message.

**GT3 — `DiffText` always passes `--binary -M`.** Binary diffs and rename
detection are what make `patch.ParseDiffOutput` able to classify
`OpBinary` / `OpRename`. Dropping either flag silently degrades the
`PatchSet`.

**GT4 — `DiffNoIndex` treats exit code 1 as success.** `git diff
--no-index` returns 1 when the files differ. The wrapper accepts 0 and 1
and errors on anything else. Any new untracked-file handling must use
`DiffNoIndex`, not a hand-rolled read.

**GT5 — `ApplyPatch` tries four strategies in order** (`git.go:226-248`):
plain `apply`, `--3way`, `--whitespace=fix`, then `--reject`. The first
three may leave the tree dirty on failure, which is why the caller resets
to `BASE_COMMIT` before the next attempt. Adding a strategy means
re-checking that reset contract in `engine.applyOperationRange`.

**GT6 — `ResetPathToCommit` deletes files that do not exist at the ref.**
It checks `FileExistsAtCommit` and falls back to `os.RemoveAll`. Callers
rely on this to clean up a path that the repo deletes; do not replace it
with a bare `git checkout <ref> -- <path>`.

**GT7 — `StashPush` returns `""` when there was nothing to save.**
It detects the `No local changes to save` message. Callers store the ref in
`state.PendingStash`; writing an empty string there is meaningful ("no stash
outstanding") and must not be turned into a literal `""` stash reference.

**GT8 — Pathspecs are appended after `--`.** `IsDirtyPaths`,
`ListUntracked`, `DiffNameStatusBetween` and `DiffTreeNameStatus` all build
`... -- <pathspecs>`. A path that starts with `-` would otherwise be parsed
as a flag.

## Workflows

**Adding a new git operation**
1. Add a typed wrapper that calls `Run` and converts a non-zero `Code` into
   an error using `strings.TrimSpace(result.Stderr)`.
2. Keep the signature `(ctx, dir, …) (value, error)` — `dir` is always
   explicit so no wrapper can accidentally act on the process cwd.
3. If it needs a pathspec, place it after `--`.
4. Add a case to `git_test.go` if it has a non-obvious exit-code contract
   (like `DiffNoIndex` or `StashPush`).

**Debugging a patch that will not apply**
1. `ApplyPatch` returns the stderr of the *last* strategy tried, which is the
   `--reject` run — the message is a `.rej` complaint, not the original
   conflict.
2. Look for the `.rej` file next to the target in the checkout;
   `patch.RejectPath` computes its path and `verifyResolved` deletes it once
   the conflict is resolved.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — layering (IN3).
- [`../engine/AGENTS.md`](../engine/AGENTS.md) — the reset-before-apply
  contract (EN4) that depends on this package.
- [`../patch/AGENTS.md`](../patch/AGENTS.md) — the consumer of `DiffText`,
  `DiffNoIndex` and `DiffTreeNameStatus`.
