# `tools/dev/cmd/` — `browseros-dev` commands

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dev`.

## What's here

Every cobra subcommand of the `browseros-dev` CLI. `root.go` defines the root
command (`SilenceUsage`/`SilenceErrors`, completions disabled) and
`Execute()`. The rest are the individual verbs — `setup`, `watch`, `test`,
`target`, `reset`, `cleanup`, `dogfood-stop` — each with flags declared in
`init()` and a `RunE` body, each with a `*_test.go` beside it. `style.go`
holds the shared `fatih/color` styles used for prompt output.

## Contents

```
tools/dev/cmd/
├── root.go          ← rootCmd ("browseros-dev"), Execute()
├── setup.go         ← setup [--if-needed]: bun install + agent GraphQL codegen
├── watch.go         ← watch [--new] [--manual]: agent → CDP → server supervision
├── test.go          ← test [--keep] [--headless] [-- bun test args]
├── target.go        ← target definitions: dev | dogfood | prod (ports, dirs, Lima home)
├── reset.go         ← reset: guided destructive profile + Lima VM reset
├── cleanup.go       ← cleanup [--target] [--quick] [--yes] [--only-ports|--only-temps]
├── dogfood_stop.go  ← stop the dogfood daemon over its IPC socket (10 s timeout)
├── style.go         ← headerStyle, commandStyle, successStyle, warnStyle, labelStyle, pathStyle, dimStyle
└── *_test.go        ← root, setup, watch, test, target, reset, cleanup, dogfood_stop tests
```

## Rules

**DC1 — Every command registers itself in `init()`.**
`rootCmd.AddCommand(<x>Cmd)`; there is no central list.

**DC2 — Flags are package-level vars bound in `init()`,** not locals —
`watchCmd.Flags().BoolVar(&watchNew, "new", false, …)`. Keep that pattern so
tests can construct commands without re-parsing `os.Args`.

**DC3 — `GroupID` isn't used here** (unlike dogfood). Don't add cobra command
groups to this module; dogfood's `style.go`/`root.go` pattern is separate.

**DC4 — Resolve the repo root first.** Commands that need paths call
`proc.FindMonorepoRoot()` before doing anything else and return its error.

**DC5 — Destructive verbs prompt.** `reset` and `cleanup` read from stdin
(`bufio.Scanner`) and support non-interactive flags (`--yes`, `--quick`);
`--target` selects `dev`, `dogfood`, or `prod`. Never make a destructive step
unconditional.

**DC6 — `test` passes args through verbatim.** Everything after `--` reaches
`bun test`; don't reformat or filter them.

**DC7 — Long-running commands handle SIGINT.** `watch` and `test` use
`signal.NotifyContext` and cancel the process context, which in turn stops the
`ManagedProc` goroutines. A new long-running command must do the same or it
will orphan a browser.

## Workflows

**Adding a subcommand:** 1. New `cmd/<name>.go`. 2. Declare a package-level
`cobra.Command` with `Use`/`Short`/`Long`/`RunE`. 3. Bind flags in `init()` and
`rootCmd.AddCommand`. 4. Add `cmd/<name>_test.go`. 5. Add a `bun run dev:*`
script in `../../package.json` if it should be reachable from the root.

**Wiring a new supervised process into `watch`:** register it with
`proc.StartManaged` in `runWatch`, then gate the following step on a readiness
poll from `browser/` or `server/`.

**Debugging "dev watch run is already locked":** a previous run's process is
still holding the flock in `proc/process.go`. `bun run dev:cleanup` or
`dev:reset` clears it; don't hand-delete the state file while the process is
alive.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dev` scope (rules DV1–DV7).
- [`root.go`](root.go) — command registration point.
- [`../proc/AGENTS.md`](../proc/AGENTS.md) — process supervision used by these commands.
- [`../browser/AGENTS.md`](../browser/AGENTS.md) — launch args and CDP readiness.
- [`../../../../package.json`](../../../package.json) — the `dev:*` scripts that call these.
