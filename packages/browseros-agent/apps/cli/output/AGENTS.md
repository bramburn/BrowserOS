# `output/` — printing and exit

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> the Go CLI. Every byte the CLI writes to stdout or stderr goes
> through this package.

## What's here

One file, `printer.go`. It holds the four ways a command reports
success (`JSON`, `JSONRaw`, `Text`, `Confirm`), the two ways it reports
failure (`Error`, `Errorf`), and two hand-rolled formatters for
structured content (`PageList`, `ActivePage`). The `Error*` helpers are
the only place the CLI decides an exit code is needed.

## Contents

```
output/
└── printer.go   ← JSON, JSONRaw, Text, Confirm, Error, Errorf,
                   PageList, ActivePage,
                   intVal, strVal, boolVal
```

## Rules

### OUT1 — Never `fmt.Println` a result; call an `output` function
Commands print results through `output.JSON` / `output.Text` /
`output.Confirm` / `output.PageList` / `output.ActivePage`. That is what
keeps `--json` and human output mutually exclusive. The exception is
progress chatter during a command's own work (e.g. `install.go`
printing "Downloading…"), which is stdout-only and is fine.

### OUT2 — `Error` and `Errorf` exit; they don't return
Both write `"Error: <msg>"` in red to **stderr** and call `os.Exit(code)`.
Code is the argument, not a constant chosen here. Callers follow 1/2/3
(runtime / page resolution / bad argument) — see `cmd/AGENTS.md` CMD3.
Because they exit, code after the call is unreachable; don't write
`if err != nil { output.Error(...); return }` expecting the `return`
to run.

### OUT3 — `JSON` prefers `structuredContent`, `JSONRaw` is the escape hatch
`JSON(*mcp.ToolResult)` emits `result.StructuredContent` when it is
non-nil and falls back to the whole result otherwise. `JSONRaw(v any)`
marshals an arbitrary value and is what `cmd/update.go` uses for its
own payload (`applied`, `currentVersion`, `asset`, …). Use `JSONRaw`
for a hand-built struct, `JSON` for a tool result. Don't add a third
variant.

### OUT4 — Colours come from the package-level `color` vars
`errColor`, `dimColor`, `boldColor` are the three defined at the top of
the file, plus the `help*` colours in `cmd/root.go` for the usage
template. Commands that need their own colour (see `init.go`) construct
a local `color.New(...)`; that's acceptable, but a *shared* semantic
colour belongs in the var block. `github.com/fatih/color` no-ops when
stdout isn't a TTY, so no manual TTY detection is needed.

### OUT5 — Structured content is read through `intVal` / `strVal` / `boolVal`
These return zero values instead of panicking on a type assertion
mismatch. JSON numbers are `float64`, so `intVal` is what converts
`pageId`. A new formatter must use these, not raw `.(float64)`.

### OUT6 — Adding a formatter means adding it here, not in `cmd/`
`PageList` and `ActivePage` are the model: take a `*mcp.ToolResult`,
fall back to `Text(result)` when `StructuredContent` is nil or has an
unexpected shape, and never return an error. Two formatters now live in
this package rather than in the command files — keep it that way.

## Workflows

### "A command needs a different human layout"
1. Add a `func X(result *mcp.ToolResult)` to `printer.go` alongside
   `PageList`.
2. Guard with `if result.StructuredContent == nil { Text(result); return }`
   and re-check the key type before iterating.
3. Call it from the command only in the non-`jsonOut` arm.

### "A new exit code is needed"
1. Confirm no existing code fits — 1/2/3 cover runtime, page, argument.
2. Call `output.Errorf(<code>, ...)` with the new code.
3. Document it in `../README.md` and in `cmd/AGENTS.md` CMD3.

### "Testing output"
There is no `output/` test file today. The package is pure formatting
over `mcp.ToolResult`, so a new `printer_test.go` should build
`&mcp.ToolResult{StructuredContent: map[string]any{...}}` by hand and
capture stdout — no server required.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the Go CLI as a whole.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — every call site, and the exit-code contract.
- [`../mcp/AGENTS.md`](../mcp/AGENTS.md) — where `ToolResult` and `StructuredContent` come from.
