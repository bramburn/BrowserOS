# `tools/patch/internal/` — implementation packages

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/`](../AGENTS.md).

## What's here

The eight packages that implement `browseros-patch`. Go's `internal/` rule
means only this module can import them; there is no exported API surface
outside `main.go` → `cmd/`.

Each package has one job, and the dependency graph is acyclic and strictly
layered.

## Contents

| Package | Files | Responsibility |
|---|---|---|
| `app/` | `app.go` | process-wide state: config, registry, cwd; `Load`, `Save`, `ResolveWorkspace`, `RepoInfo` |
| `engine/` | `apply.go`, `extract.go`, `sync.go`, `status.go`, `publish.go`, `progress.go`, `strings.go`, `engine_test.go` | the operations; one exported func per verb, each with an `Options`/`Result` pair |
| `git/` | `git.go`, `git_test.go` | typed wrappers over the `git` binary (`Run`, `DiffText`, `ApplyPatch`, `StashPush`, …) |
| `patch/` | `parser.go`, `compare.go`, `repo.go`, `types.go`, `workspace.go`, `patch_test.go` | the `FilePatch` model, diff parsing, repo patch-set I/O, drift classification |
| `repo/` | `repo.go` | find and load the patch repo (`BASE_COMMIT` + `chromium_patches/`) |
| `resolve/` | `resolve.go`, `resolve_test.go` | conflict-resume state at `.browseros-patch/resolve.json` |
| `ui/` | `ui.go` | lipgloss styles and `RenderTable` |
| `workspace/` | `config.go`, `registry.go`, `detect.go`, `state.go`, `workspace_test.go` | checkout registry, cwd detection, per-checkout `state.yaml` |

Dependency direction (arrows point at the imported package):

```
cmd → app → {repo, workspace}
cmd → engine → {git, patch, repo, resolve, workspace}
cmd → ui
engine → patch → git
resolve → {patch, workspace}
workspace → yaml only
git, patch(→git), repo, ui → stdlib only
```

## Rules

**IN1 — Respect the layering.** `workspace`, `repo`, `git` and `ui` must not
import `engine` or `cmd`. `patch` must not import `workspace` (it only needs
`git` for the diff builders). A cycle here is a design error, not a build
error to work around.

**IN2 — `engine` is the only package that composes operations.** Each verb is
one exported function taking an `Options` struct with a `Progress` field and
returning a `Result` struct with json tags. Do not add a helper that spans
two verbs.

**IN3 — `git/` is the only place that shells out to git.** Every wrapper
returns `(value, error)` and converts a non-zero exit into an `error` built
from stderr. Never invoke git from `engine` or `cmd` directly — the retry
strategy in `ApplyPatch` (plain → `--3way` → `--whitespace=fix` → `--reject`)
lives in `git.go:226-248` and must stay in one place.

**IN4 — `ui/` is the only package that imports lipgloss.** `engine` and
`cmd` produce plain strings and let `cmd` style them; this keeps `--json`
output trivially serialisable.

**IN5 — Normalise every path that crosses a package boundary with
`patch.NormalizeChromiumPath`.** Repo keys, `FilePatch.Path` and
`FileChange.Path` all use forward slashes with no `./` prefix, so a Windows
checkout and a Linux checkout produce identical `PatchSet` maps.

**IN6 — Filter out `.browseros-patch/` via `patch.IsInternalPath`, not by
name string matching in each package.** It is used by `PathMatches` (so
filters never match it) and by `engine.Extract`'s `changedScope`. New code
that enumerates workspace paths must go through the same helper.

**IN7 — Persisted files are versioned and default to `1` on load.**
`config.yaml`, `workspaces.yaml` and `state.yaml` all carry a `version`
field, and every loader coerces `0` → `1` when it is absent. New persisted
fields must be `omitempty` so an old file still parses.

**IN8 — Result structs carry json tags and no unexported state.** They are
the `--json` contract. `ApplyResult`, `SyncResult`, `ExtractResult`,
`PublishResult` and `WorkspaceStatus` are all consumed by scripts.

## Workflows

**Tracing what a command actually does**
1. Start at the verb in `../cmd/`.
2. Follow it into the matching `engine/<verb>.go`.
3. The leaf operations are in `patch/` (diff parsing) and `git/` (process
   invocation).

**Adding a persisted field to workspace state**
1. Add the field to `workspace.State` with `yaml` and `json` tags plus
   `omitempty`.
2. `LoadState` already tolerates a missing file and a missing field; do not
   add migration code for a scalar.
3. Update `markApplyComplete` / `Sync` / `Extract` if the new field is
   written on a specific path.

**Deciding where a new helper goes**
1. Wraps a git invocation → `git/`.
2. Parses or writes a diff → `patch/`.
3. Sequences two or more of the above → `engine/`.
4. Formats output → `ui/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — module overview (GO1-GO10).
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — the command layer.
- [`app/AGENTS.md`](app/AGENTS.md), [`engine/AGENTS.md`](engine/AGENTS.md),
  [`git/AGENTS.md`](git/AGENTS.md), [`patch/AGENTS.md`](patch/AGENTS.md),
  [`repo/AGENTS.md`](repo/AGENTS.md), [`resolve/AGENTS.md`](resolve/AGENTS.md),
  [`ui/AGENTS.md`](ui/AGENTS.md),
  [`workspace/AGENTS.md`](workspace/AGENTS.md) — per-package detail.
- [`../AGENTS.md`](../AGENTS.md) — the `browseros-patch` module.
