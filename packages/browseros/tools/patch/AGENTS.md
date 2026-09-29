# `tools/patch/` — the `browseros-patch` Go CLI

> Part of [`../AGENTS.md`](../AGENTS.md) in [`tools/`](../AGENTS.md), in
> [`packages/browseros/`](../../AGENTS.md).

## What's here

A self-contained Go module — `github.com/browseros-ai/BrowserOS/packages/browseros/tools/patch`,
Go 1.25 — that implements the `browseros-patch` command. It moves changes
between two places: the **patch repo** (this `packages/browseros/` tree,
recognised by a `BASE_COMMIT` file next to a `chromium_patches/`
directory) and one or more registered **Chromium checkouts** (`ch1`, `ch2`,
…).

Dependencies are deliberately few: `spf13/cobra` for the command tree,
`charmbracelet/lipgloss` for terminal output, `gopkg.in/yaml.v3` for state
files. There is no git library — `internal/git` shells out to the `git`
binary.

## Contents

```
patch/
├── main.go                 ← 7 lines: calls cmd.Execute()
├── go.mod / go.sum
├── Makefile                ← build / install / uninstall / test / fmt / clean
├── cmd/                    ← cobra command definitions (one file per verb)
└── internal/
    ├── app/                ← process-wide state (config, registry, cwd)
    ├── engine/             ← apply / extract / sync / status / publish logic
    ├── git/                ← typed wrappers over the git CLI
    ├── patch/              ← diff parsing, patch-set model, repo I/O
    ├── repo/               ← patch-repo discovery + BASE_COMMIT
    ├── resolve/            ← conflict resume state (.browseros-patch/resolve.json)
    ├── ui/                 ← lipgloss styles + table helper
    └── workspace/          ← checkout registry, config, per-checkout state
```

There is no README in this directory. `browseros-patch --llm-txt` prints a
concise operating guide, and `cmd/common.go:40-55` holds its source.

## Rules

**GO1 — This is Go, not Python.** Nothing here is covered by
`pyproject.toml` or `pyrightconfig.json`. Use `make build` / `make test` /
`make fmt`; the repo-root Python tooling does nothing for this module.

**GO2 — Keep `internal/` internal.** The import path contains
`/internal/`, so Go itself enforces that only this module can import these
packages. Do not create a second module that reuses them.

**GO3 — The patches repo is identified by two markers, not by path.**
`internal/repo/repo.go:55-61` requires both `BASE_COMMIT` and a
`chromium_patches/` directory. Adding a repo without `BASE_COMMIT` makes
the tool report "not a browseros patches repo".

**GO4 — Config and state live outside the repo.**
`~/.config/browseros-patch/config.yaml` (honours `XDG_CONFIG_HOME`) holds
`patches_repo`; `workspaces.yaml` holds registered checkouts. Per-checkout
state is written **into the Chromium checkout** at
`.browseros-patch/state.yaml`. Never commit that directory — it is excluded
by `patch.IsInternalPath` and by the `git ls-files --exclude-standard`
untracked scan.

**GO5 — Every command must work with `--json`.** `cmd/root.go` exposes
`--json` as a persistent flag and `renderResult` is the only sanctioned way
to print results. Direct `fmt.Println` of human text in a new command
breaks scripted use; add a `json` struct instead.

**GO6 — Every subcommand registers with an explicit help group.**
`Annotations: map[string]string{"group": ...}` drives the custom usage
template in `cmd/root.go`. Valid values are `Chromium Checkouts:`, `Core:`,
`Conflict:`, `Remote:`; the default when omitted is `Core:`. A typo'd group
silently hides the command from its section.

**GO7 — Long-running work reports through `Progress`, not stdout.**
`commandProgress(cmd)` returns `nil` under `--json` and otherwise writes to
stderr. Never print progress to stdout in `engine/` — it would corrupt the
JSON stream.

**GO8 — Any path crossing a boundary is normalised with
`patch.NormalizeChromiumPath`.** It does `filepath.ToSlash` + `path.Clean`
+ trims a leading `./`. Repo paths are stored with forward slashes so a
Windows checkout and a Linux checkout produce identical `PatchSet` keys.

**GO9 — Registration happens in `init()` via `rootCmd.AddCommand`.** Each
`cmd/*.go` file follows the same shape: declare flags, build
`&cobra.Command{...}`, register in `init()`. Do not introduce a second
registration mechanism.

**GO10 — Don't add a dependency without a reason in the PR.** Three direct
dependencies today; the remaining `require` block in `go.mod` is indirect
and pulled in by lipgloss.

## Workflows

**Building and testing**
1. `cd packages/browseros/tools/patch`
2. `make build` → `./browseros-patch`; `make test` → `go test ./...`;
   `make fmt` → `gofmt -w` over all `.go` files.
3. `make install VERSION=x.y.z` to stamp a version via `-ldflags`.

**First-time setup on a machine**
1. `browseros-patch add ch1 /path/to/chromium/src` from inside
   `packages/browseros/` (so `patches_repo` is auto-discovered), or pass
   `--patches-repo` explicitly.
2. `browseros-patch status ch1` to confirm it sees the checkout.

**Adding a new subcommand**
1. Create `cmd/<verb>.go` following the existing shape (flags → `RunE` →
   `renderResult` → `rootCmd.AddCommand`).
2. Put the logic in `internal/engine/`, not in `cmd/`. `cmd/` is
   argument-parsing and rendering only.
3. Give the command the `--src` flag with `srcFlagUsage` so it works
   without registry lookup, and add the name to the `--src` list asserted in
   `TestSrcFlagExplainsDirectCheckoutPath` (`cmd/common_test.go:235-249`).
4. Add a `group` annotation and a doc comment; run `make test`.

**Changing the output look**
1. Add a style var and accessor in `internal/ui/ui.go`.
2. Use it from `cmd/` only; `internal/engine` must stay styling-free.
3. `internal/ui` is the only package that imports lipgloss.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tools/` context and Go-vs-Python split.
- [`cmd/AGENTS.md`](cmd/AGENTS.md) — the command tree.
- [`internal/AGENTS.md`](internal/AGENTS.md) — package layout and
  dependency direction.
- [`../../AGENTS.md`](../../AGENTS.md) — `packages/browseros/` conventions
  this tool implements.
- [`../../build/features.yaml`](../../build/features.yaml) — the manifest that
  must be updated alongside any extract.
- [`../../chromium_patches/AGENTS.md`](../../chromium_patches/AGENTS.md) —
  the tree this tool writes.
