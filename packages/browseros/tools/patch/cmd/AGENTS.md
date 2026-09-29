# `tools/patch/cmd/` — cobra command definitions

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/`](../AGENTS.md).

## What's here

One file per verb, each defining a single `*cobra.Command` in an `init()`
and registering it with `rootCmd`. This package does argument parsing,
flag wiring, workspace resolution, progress routing and result rendering —
nothing else. All behaviour lives in `internal/engine`.

Thirteen commands in four help groups.

## Contents

| File | Command | Group | What it does |
|---|---|---|---|
| `root.go` | `browseros-patch` | — | root command, usage template, `--json` / `-v` / `--llm-txt`, `renderResult` |
| `add.go` | `add <name> <path>` (alias `register`) | Chromium Checkouts | register a Chromium checkout |
| `list.go` | `list` (alias `ls`) | Chromium Checkouts | table of registered checkouts |
| `remove.go` | `remove <name>` (alias `rm`) | Chromium Checkouts | deregister a checkout |
| `apply.go` | `apply [checkout] [-- files...]` | Core | apply repo patches to a checkout |
| `diff.go` | `diff [checkout]` | Core | show repo-vs-checkout drift |
| `status.go` | `status [checkout]` | Core | workspace status summary |
| `sync.go` | `sync [checkout]` | Core | pull the patch repo, re-apply, rebase |
| `extract.go` | `extract [checkout] [--range <start> <end>] [-- files...]` | Core | write checkout changes into `chromium_patches/` |
| `publish.go` | `publish [remote]` | Remote | commit + push `chromium_patches` |
| `continue.go` | `continue` | Conflict | resume after resolving the current conflict |
| `skip.go` | `skip` | Conflict | skip the current conflict and continue |
| `abort.go` | `abort` | Conflict | reset patched files to `BASE_COMMIT`, restore the stash |
| `common.go` | — | — | shared helpers (below) |
| `common_test.go` | — | — | tests for the shared helpers |

`common.go` holds `resolveWorkspace` (delegates to
`workspace.ResolveForCommand`), `splitWorkspaceAndFilters` (splits args at
`--` via `cmd.ArgsLenAtDash()`), `ensureRepoConfigured`,
`commandProgress` (returns `nil` under `--json`), and `llmTxtGuide`.

## Rules

**CM1 — `cmd/` never contains business logic.** A `RunE` body should parse
flags, resolve the workspace, call one `internal/engine` function, then
`renderResult`. Anything resembling a diff, a git call, or a file walk
belongs in `engine/` or `patch/`.

**CM2 — Register a `--src` flag on every checkout-taking command**, using
the shared `srcFlagUsage` string. `TestSrcFlagExplainsDirectCheckoutPath`
(`common_test.go:235-249`) enumerates the commands expected to have it and
asserts the usage text explains the registry bypass; a new command without it
fails the test.

**CM3 — Path filters come after `--`, never as bare arguments.**
`splitWorkspaceAndFilters` uses `cmd.ArgsLenAtDash()`; everything after the
first `--` is a path filter. `apply` additionally rejects more than one
positional argument ("expected at most one checkout name").

**CM4 — Every command needs `Annotations: map[string]string{"group": ...}`.**
Valid groups: `Chromium Checkouts:`, `Core:`, `Conflict:`, `Remote:`. The
order is fixed by `groupOrder` in `root.go`; an unrecognised group is
dropped from `--help` entirely.

**CM5 — Print results only through `renderResult(data, humanFn)`.** It
encodes `data` as indented JSON when `--json` is set, and otherwise calls
`humanFn`. Direct `fmt.Println` bypasses that contract.

**CM6 — Progress goes to stderr via `commandProgress(cmd)`.** It is
deliberately `nil` under `--json`; returning a non-nil progress there would
corrupt the JSON document on stdout.

**CM7 — Root-only flags stay root-only.** `--llm-txt` is registered with
`rootCmd.Flags()`, not `PersistentFlags()`, and `PersistentPreRunE` rejects
it on a subcommand. `TestRootLLMTxtPrintsWithoutLoadingApp`,
`TestLLMTxtRejectedWithSubcommand` and `TestLLMTxtNotShownInSubcommandHelp`
(`common_test.go:277-354`) assert it never loads app state and never appears
in subcommand help.

**CM8 — `appState` is loaded in `PersistentPreRunE` and is `nil` under
`--llm-txt`.** Any new command that touches config or the registry must go
through `appState` (or the `repoInfo()` / `resolveWorkspace()` helpers), not
by re-reading the config files.

**CM9 — `SilenceUsage: true` and `SilenceErrors: true` are set on the root.**
Return errors; do not print them yourself or call `os.Exit` from a command.

## Workflows

**Adding a new verb**
1. Create `cmd/<verb>.go` with a package-level flag var, a `&cobra.Command`
   with `Use`, `Short`, `Example`, `Args`, a `group` annotation, and a
   `RunE` that follows CM1-CM6.
2. Add the `--src` flag if it takes a checkout (CM2), and add its name to
   the list in `TestSrcFlagExplainsDirectCheckoutPath` (`common_test.go:236`).
3. Put the behaviour in `internal/engine/<verb>.go` as
   `Xxx(ctx, XxxOptions) (*XxxResult, error)`.
4. `make test` from `tools/patch/`.

**Adding a flag to an existing command**
1. Declare the var at the top of the file and pass it through an
   `engine.XxxOptions` field — do not read it in `engine`.
2. Add an entry to the command's `Example` block if it changes the
   recommended usage.

**Debugging a command that works with a name but not from another directory**
1. `resolveWorkspace` falls back to `workspace.DetectForCommand`, which
   matches cwd against registered checkout paths (longest prefix wins, with
   symlinks resolved).
2. Outside any checkout, pass the checkout name or `--src` — the error
   message already prints the registered names and a suggested invocation.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — module overview and Go rules.
- [`../internal/AGENTS.md`](../internal/AGENTS.md) — package dependency
  direction.
- [`../internal/engine/AGENTS.md`](../internal/engine/AGENTS.md) — where the
  logic lives.
- [`../internal/ui/AGENTS.md`](../internal/ui/AGENTS.md) — the styling
  helpers used in the human render functions.
- [`../internal/workspace/AGENTS.md`](../internal/workspace/AGENTS.md) —
  checkout resolution.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `packages/browseros/`.
