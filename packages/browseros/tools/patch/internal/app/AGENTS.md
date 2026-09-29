# `tools/patch/internal/app/` — process-wide state

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/internal/`](../AGENTS.md).

## What's here

One file, `app.go`, and one type: `App`. It is the object every command
touches first — created once in `cmd/root.go`'s `PersistentPreRunE` and
stored in the package-level `appState` variable.

`App` holds the four things a command cannot resolve on its own: the output
mode (`JSON`), verbosity, the working directory, and the two persisted
files (config + checkout registry).

## Contents

`app.go` (67 lines):

| Symbol | Purpose |
|---|---|
| `type App struct` | `JSON`, `Verbose`, `CWD`, `Config *workspace.Config`, `Registry *workspace.Registry` |
| `Load(jsonOut, verbose, cwd) (*App, error)` | resolves `cwd` (defaults to `os.Getwd`), then `workspace.LoadConfig()` and `workspace.LoadRegistry()` |
| `(*App) Save() error` | writes config then registry |
| `(*App) ResolveWorkspace(name, src)` | thin wrapper over `workspace.Resolve` |
| `(*App) RepoInfo() (*repo.Info, error)` | loads the patch repo, with a discovery fallback |

## Rules

**AP1 — Construct it once, in `PersistentPreRunE`.** `cmd/root.go` does
`appState, err = app.Load(jsonOut, verbose, "")`. A second `Load` in a
command re-reads both YAML files and can produce a state that disagrees
with what the root resolved.

**AP2 — `appState` is `nil` under `--llm-txt`.** `PersistentPreRunE`
returns before `Load` when the flag is set, and
`TestRootLLMTxtPrintsWithoutLoadingApp` (`cmd/common_test.go:277-305`)
asserts that path never touches state. Any new code must tolerate the nil
case rather than assuming the root always loaded it.

**AP3 — `RepoInfo()` has two paths and both matter.** If
`Config.PatchesRepo` is set, that path is used; otherwise `repo.Discover`
walks up from `cwd` looking for the repo markers. The error message tells
the user to run `browseros-patch add <name> <path> --patches-repo <repo>`
once. Commands should surface that message, not invent their own.

**AP4 — `Save()` writes config before registry.** A partial write leaves
`patches_repo` set without the checkout, which is the recoverable direction.
Reversing the order would leave checkouts pointing at an unconfigured repo.

**AP5 — Keep `App` free of logic.** It is a holder plus three thin
delegates. Anything that needs to decide something belongs in `workspace`,
`repo` or `engine`; otherwise the same decision gets made twice (once here,
once in `cmd/common.go`'s `ensureRepoConfigured`).

**AP6 — `cwd` is `filepath.Clean`ed at load time.** Detection relies on that
normalisation; passing an uncleaned path from a new caller would make
`DetectForCommand` fail to match a registered checkout.

## Workflows

**Loading state in a new command**
1. Use the `appState` variable from `cmd` — do not call `app.Load` again.
2. Reach the registry via `appState.Registry`, the config via
   `appState.Config`.
3. Persist with `appState.Save()` after any mutation; individual
   `workspace.SaveConfig` / `SaveRegistry` calls exist for finer control but
   `Save()` is the default.

**Diagnosing "patches repo is not configured"**
1. Check `~/.config/browseros-patch/config.yaml` for `patches_repo`.
2. If unset, the command must be run from inside a `packages/browseros/`
   directory, or `--patches-repo` must be passed.
3. The repo markers are a `BASE_COMMIT` file *and* a `chromium_patches/`
   directory — see [`../repo/AGENTS.md`](../repo/AGENTS.md).

**Extending the struct**
1. Add the field with a value that is safe when the config file is absent
   (loaders return an empty `Config{Version: 1}` rather than an error).
2. Do not add a field that is only meaningful to one command — pass that
   through the `engine.XxxOptions` struct instead.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — package layering (IN1).
- [`../../cmd/AGENTS.md`](../../cmd/AGENTS.md) — where `app.Load` is called
  (rule CM8).
- [`../workspace/AGENTS.md`](../workspace/AGENTS.md) — `Config` and
  `Registry`.
- [`../repo/AGENTS.md`](../repo/AGENTS.md) — `repo.Discover` / `repo.Load`.
