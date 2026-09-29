# `components/ui/` — Vendored shadcn/ui primitives

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/components`.

## What's here

33 files copied from shadcn/ui (`style: "new-york"`, `baseColor: neutral`,
`rsc: false`, `iconLibrary: lucide` — all declared in
[`../../components.json`](../../components.json)). They wrap Radix UI and are
built on `cva` + `cn()` from `@/lib/utils`. 32 are genuine shadcn
primitives; `MarkdownEditor.tsx` is hand-written and is the single
first-party file in this directory.

## Contents

```
ui/
├── primitives (shadcn): alert, alert-dialog, badge, button, button-group,
│   card, carousel, checkbox, collapsible, command, dialog, dropdown-menu,
│   form, hover-card, input, input-group, kbd, label, popover, progress,
│   resizable, scroll-area, select, separator, sheet, sidebar, skeleton,
│   sonner, switch, tabs, textarea, tooltip
└── first-party: MarkdownEditor.tsx  ← @mdxeditor/editor wrapper with a
                                      custom toolbar, copy button and
                                      read-only mode
```

## Rules

- **UI1 — Do not hand-edit the shadcn primitives.** Re-run
  `bunx shadcn@latest add <name>` to pull a fresh upstream copy, and layer
  local behaviour in a wrapper component in `components/elements/` or the
  feature folder instead of editing the vendored file.
- **UI2 — `MarkdownEditor.tsx` is *not* vendored.** It is maintained here and
  may be edited freely; do not regenerate or "restore" it from shadcn.
- **UI3 — Variant definitions are `cva`, never inline conditionals.** Extend
  the `buttonVariants` map rather than branching inside a component body.
- **UI4 — Always compose `cn()` with `className`.** Every primitive accepts and
  merges an incoming `className`; that is how call sites override styles.
- **UI5 — Keep shadcn's import surface intact.** The `components.json`
  aliases (`@/components`, `@/components/ui`, `@/lib/utils`) are what the CLI
  writes; a rewritten import path breaks the next `add`.
- **UI6 — `useIsMobile()` from `@/hooks` drives the `sidebar.tsx` mobile
  variant.** Don't add a second breakpoint hook for it.

## Workflows

**Adding a new shadcn primitive**
1. `cd packages/browseros-agent/apps/agent`
2. `bunx shadcn@latest add <name>` — it reads `components.json` and writes
   into this directory.
3. Do not "tidy" the generated file; it is the upstream shape.

**Customising a primitive's look**
1. Leave `ui/<name>.tsx` untouched.
2. Wrap or re-export it from `components/elements/` and pass `className` /
   variant props through.

**Editing the markdown editor**
1. Edit `MarkdownEditor.tsx` directly — it is first-party.
2. Re-verify read-only mode, which passes `editable: false` to `MDXEditor`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — component-tree rules (CMP2, CMP3).
- [`../../components.json`](../../components.json) — shadcn config for this dir.
- [`../../styles/AGENTS.md`](../../styles/AGENTS.md) — the tokens these primitives use.
- [`../ai-elements/AGENTS.md`](../ai-elements/AGENTS.md) — the other vendored set.
- [`../../lib/utils.ts`](../../lib/utils.ts) — the `cn()` helper.
