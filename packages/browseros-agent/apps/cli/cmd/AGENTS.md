# `cmd/` — cobra command surface

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> which covers the Go CLI. This file covers the command layer only;
> `mcp/`, `config/`, `output/`, `analytics/` and `update/` each have
> their own.

## What's here

Every user-facing `browseros-cli` subcommand, plus the root command
that owns the global flags. Each file is a self-registering cobra
command: a package-level `init()` builds the `*cobra.Command` and
appends it to `rootCmd`. The commands are thin — parse flags, build a
`map[string]any` of MCP tool arguments, call
`mcp.Client.CallTool`, print. `root.go` is the exception: it also
holds server-URL resolution, the help template, and the automatic
update hook that runs around every command.

## Contents

```
cmd/
├── root.go        ← rootCmd, 5 global flags, grouped help template,
│                    newClient(), resolvePageID(), server-URL resolution,
│                    automatic-update wiring, analytics Init/Track
├── root_test.go   ← version, commandName, primaryCommand, URL normalisation
├── update.go      ← update / self-update / upgrade; + update_test.go
│
├── Navigate:      pages.go   pages, active, close
│                  open.go    open <url>
│                  nav.go     nav, back, forward, reload
├── Observe:       snap.go    snap            text.go    text, links
│                  dom.go     dom, dom-search eval.go    eval
│                  wait.go    wait            file_actions.go  pdf
├── Input:         click.go   click, click-at
│                  fill.go    fill, clear, key
│                  interact.go hover, focus, check, uncheck, select,
│                              drag, upload
│                  scroll.go  scroll           dialog.go dialog
│                  file_actions.go  download
├── Resources:     bookmark.go  bookmark list|create|remove|update|move|search
│                  group.go     group list|create|update|ungroup|close
│                  window.go    window list|create|close|activate
├── Integrations:  strata.go   Strata connector MCP servers and actions
├── Setup:         health.go  health, status   info.go   info [topic]
│                  init.go    init [url]       config.go config
│                  install.go install          launch.go launch
│                  update.go  update
│
├── launch_test.go ← platform detection helpers
└── update_test.go ← runUpdateCommand prompt/cancel/apply logic
```

## Rules

### CMD1 — Register with `init()` + `rootCmd.AddCommand(...)`; no central list
There is no commands registry. Go runs every file's `init()` before
`main()` calls `cmd.Execute()`, so adding a file is the whole
registration step. Don't introduce a `commands.go` slice.

### CMD2 — Every command must set `Annotations: map[string]string{"group": "..."}`
`groupedHelp` in `root.go` buckets the help output by this annotation
and a command with no group silently lands in `"Setup:"`. The only
valid values are the six entries in `groupOrder`: `Navigate:`,
`Observe:`, `Input:`, `Resources:`, `Integrations:`, `Setup:`.

### CMD3 — Exit codes are a contract: 1 runtime, 2 page, 3 argument
`output.Error` / `output.Errorf` write to stderr and `os.Exit`.
Every call site follows the same three codes: `1` = server/connection/
environment failure, `2` = `resolvePageID` failed, `3` = the user passed
a malformed argument. Don't introduce a fourth code without updating
`README.md`, and don't `log.Fatal` or panic.

### CMD4 — Prefer `Run`; use `RunE` only when there is nothing to format
`config.go` is the one file using `RunE` (it shells out to `$EDITOR` and
has no user-facing error text). Everything else handles its own errors so
it can pick the right exit code. `rootCmd` sets `SilenceUsage` and
`SilenceErrors`, so a returned error reaches the user bare.

### CMD5 — Page targeting goes through `resolvePageID(c)`, never `--page` directly
`resolvePageID` in `root.go` layers flag-changed → `BROWSEROS_PAGE` env →
`get_active_page` fallback. A command that reads the `pageFlag` var
bypasses the env var. Commands that don't need a page (`pages` (except
`close` with no argument), `bookmark`, `window`, `group`, `strata`,
`health`, `open`, `config`, `init`, `install`, `launch`, `update`) must
not call it. `pages.go` is the model for the conditional case: it only
resolves when no explicit ID was given.

### CMD6 — Branch on `jsonOut` at the very end, and both arms go through `output/`
The house pattern is:

```go
if jsonOut { output.JSON(result) } else { output.Confirm(result.TextContent()) }
```

`--json` is a machine contract: `output.JSON` prefers
`result.StructuredContent` and falls back to the full result. Never
print human text before the `jsonOut` check, or `--json` output stops
being parseable.

### CMD7 — MCP tool names live at the call site; the Go side owns no schema
Mostly `c.CallTool("click", map[string]any{...})`. When a flag picks
between two tools the name goes in a local `toolName` var — that is the
only indirection in use today, in `open.go` (`new_page` /
`new_hidden_page`), `snap.go` (`take_snapshot` /
`take_enhanced_snapshot`), and `window.go` (`create_window` /
`create_hidden_window`). `interact.go` goes one step further with an
`elementAction(toolName string)` factory that builds the
`hover`/`focus`/`check`/`uncheck` commands. Don't invent a wider
dispatch layer.

There is no client-side input validation beyond "is this an integer".
Two gotchas: tool arguments are keyed by the *server's* names
(`"page"`, `"element"`, `"clickCount"`), which do not match the JSON
output names (`pageId`, `tabId`); and structured output is untyped, so
read it via `intVal` / `strVal` / `boolVal` in `output/printer.go`.

### CMD8 — Group related commands in one file by user intent
`interact.go` holds seven commands, `bookmark.go` holds six. The file
name is the *noun or verb family* (`interact`, `bookmark`, `group`),
not the command name. One command per file is the exception, not the
default.

## Workflows

### "Adding a new subcommand"
1. Pick or create the family file in `cmd/` (`interact.go` for a hover/
   focus/select sibling, a new `thing.go` otherwise).
2. Build the command in `init()`: `Use`, `Short`, `Annotations` with a
   valid group, `Args` (`cobra.ExactArgs`/`NoArgs`/`MaximumNArgs`),
   and `Run`.
3. `c := newClient()` → `pageID, err := resolvePageID(c)` (exit `2` on
   error) → `c.CallTool("<tool_name>", map[string]any{...})` (exit `1`
   on error).
4. Finish with the `jsonOut` / `output.*` pair.
5. Register with `rootCmd.AddCommand(cmd)` at the bottom of `init()`.
6. Add a `Short` that reads well in the 14-column help column.

### "Wrapping a new MCP tool"
1. Confirm the tool exists in the server's canonical list
   (`server/src/tools/registry.ts`) — the CLI does not implement tools.
2. Add the `CallTool` in the matching family file, following the
   argument-key convention in CMD7.
3. If the result needs custom human formatting, add the formatter to
   `output/printer.go` and use it for both text and JSON.
4. Cover it in the root `integration_test.go` (`package main`,
   `-tags integration`).

### "Adding a command family (parent + subcommands)"
1. Create a parent `*cobra.Command` with no `Run` (see `bookmark.go`
   for the exact shape) and one annotation.
2. Build each child in a loop or a small helper
   (`strata.go check <server-name>` / `discover` / `details` / `exec` is
   the fullest example).
3. Add all of them in one `rootCmd.AddCommand(parentCmd, ...)` call.

### "Renaming a command"
Add the old spelling to `Aliases`, never remove it. `update` already
carries `self-update, upgrade` and `config` carries `cfg`. Aliases are
rendered in gold by `helpAliases` in the help template.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the Go CLI as a whole.
- [`../mcp/AGENTS.md`](../mcp/AGENTS.md) — the MCP client these commands call.
- [`../output/AGENTS.md`](../output/AGENTS.md) — `output.Error` / `output.JSON` contracts.
- [`../config/AGENTS.md`](../config/AGENTS.md) — where `init` and `config` persist `server_url`.
- [`../update/AGENTS.md`](../update/AGENTS.md) — the package `update.go` drives.
- [`../README.md`](../README.md) — user-facing command list and global-flag table.
- [`../../server/AGENTS.md`](../../server/AGENTS.md) — the server that implements the tools.
