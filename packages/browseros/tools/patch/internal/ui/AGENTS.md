# `tools/patch/internal/ui/` — terminal styling

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/internal/`](../AGENTS.md).

## What's here

One file, `ui.go`, 84 lines: the only package in the module that imports
`charmbracelet/lipgloss`. It exposes a fixed palette as package-level styles,
a small set of one-line accessor functions, and one table helper.

It has no dependencies of its own beyond lipgloss and `fmt`/`strings`.

## Contents

| Group | Symbols |
|---|---|
| Colours | `clrCyan`, `clrBlue`, `clrGreen`, `clrHiGreen`, `clrYellow`, `clrRed`, `clrGray` — raw ANSI 256 codes |
| Styles | `TitleStyle`, `HeaderStyle`, `CommandStyle`, `AliasStyle`, `HintStyle`, `SuccessStyle`, `WarningStyle`, `ErrorStyle`, `InfoStyle`, `MutedStyle` |
| Accessors | `Title`, `Header`, `Command`, `Aliases`, `Hint`, `Success`, `Warning`, `Error`, `Info`, `Muted` |
| Table | `RenderTable(headers, rows)` — hidden border, bold-faint header row, two-space right padding |

Consumers: `cmd/root.go` (the custom help template),
`cmd/apply.go` / `cmd/status.go` / `cmd/diff.go` / `cmd/sync.go` /
`cmd/extract.go` / `cmd/list.go` (human render functions), and
`cmd/common.go` (progress prefix).

## Rules

**UI1 — This is the only lipgloss importer.** `engine` and the rest of
`cmd` must not import lipgloss directly (IN4 in the parent). Adding a style
here is the supported path; importing the library elsewhere is not.

**UI2 — Styles are package-level vars, not functions.** They are built
once at init. A function that builds a `lipgloss.Style` on every call
re-parses ANSI sequences per call and shows up in table rendering of a
300-file patch set.

**UI3 — Semantic names, not colours.** The accessors are `Success`,
`Warning`, `Error`, `Info`, `Muted`, `Hint`, `Title`, `Header`. A caller
picks the meaning; the mapping to a colour is this package's business.
Adding `Green()` or `Red()` accessors breaks that.

**UI4 — Aliases are always parenthesised and suffixed.**
`ui.Aliases` renders `(aliases: a, b)`. The help template relies on that
shape for its column alignment (`%-14s` for the command name).

**UI5 — `RenderTable` takes `[]string` headers and `[][]string` rows.**
Callers convert, this package does not. It applies a hidden border and a
fixed two-space right padding per cell; do not add per-cell formatting
here, because the table has no notion of a column's type.

**UI6 — Styling is optional output, never load-bearing.** Everything routed
through this package appears only on the human render path. `--json` output
must never pass through `ui`, or the JSON stream picks up escape codes.

## Workflows

**Adding a semantic style**
1. Pick an ANSI colour constant alongside the existing ones.
2. Add a `…Style` var with the same `.Bold(true)` convention as its peers.
3. Add a one-line accessor returning `style.Render(s)`.
4. Use it from a `cmd/` human render function.

**Debugging escape codes in `--json` output**
1. A `ui.` call in an `engine` result string, or a `fmt.Println` of a
   styled value outside `renderResult`'s human branch (UI6).
2. `renderResult` must be the only path that chooses between JSON and
   styled text.

**Restyling help output**
1. `cmd/root.go` builds its template from the accessor functions
   (`helpHeader`, `helpCmdCol`, `helpHint`, `helpAliases`), not from raw
   styles.
2. Change the accessor, not the template, so `--help` and error hints stay
   consistent.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — layering (IN4).
- [`../../cmd/AGENTS.md`](../../cmd/AGENTS.md) — rule CM5 (`renderResult`
  is the only sanctioned printer).
- [`../engine/AGENTS.md`](../engine/AGENTS.md) — rule EN2 (engine stays
  styling-free).
