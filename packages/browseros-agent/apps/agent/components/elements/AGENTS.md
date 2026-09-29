# `components/elements/` — First-party product widgets

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/components`.

## What's here

The extension's own composite widgets — the pieces that are neither
generic shadcn (`../ui/`) nor AI-message primitives (`../ai-elements/`).
They cover the three picker surfaces (browser tabs, MCP apps, workspace
folders), the theme switcher, and two small motion effects. Unlike the
vendored folders, this is the right place to put new product UI.

## Contents

```
elements/
├── tab-picker-popover.tsx   ← searchable tab picker, two variants:
│                              @-mention popover (anchored to the composer)
│                              and a standalone PopoverTrigger form
├── tab-list-item.tsx        ← one row of the tab list (favicon, title, globe
│                              badge for non-http pages, check when selected)
├── use-available-tabs.ts    ← chrome.tabs.query hook: current-window http
│                              tabs sorted by lastAccessed + text filter
├── AppSelector.tsx          ← MCP / "connect app" picker; bridges
│                              lib/mcp/ state to the connect-mcp dialogs
├── workspace-selector.tsx   ← recent workspace folders via lib/workspace,
│                              plus chrome.browserOS.choosePath
├── theme-toggle.tsx         ← light/dark/system radio menu over
│                              components/theme-provider
├── pill-indicator.tsx       ← small pulsing "working…" pill
├── laser.tsx                ← motion/react beam that targets an element by id
└── glowing-border.tsx       ← motion/react animated SVG stroke around a target
```

## Rules

- **ELM1 — This is where new product UI goes**, not `ui/`. If it isn't a
  generic primitive and isn't an AI-message part, it belongs in a feature
  subfolder here.
- **ELM2 — Pickers take their data from a `lib/` hook.** `use-available-tabs`
  and `useWorkspace` are the model: the component renders, the hook queries.
  `workspace-selector.tsx` also calls `getBrowserOSAdapter().choosePath()`
  directly — an accepted, isolated exception.
- **ELM3 — `use-available-tabs.ts` is kebab-case**, unlike the `useXyz.ts`
  convention in `lib/`. Keep the existing name; don't rename it in a drive-by
  change.
- **ELM4 — Motion components must clean up.** Both `laser.tsx` and
  `glowing-border.tsx` drive `requestAnimationFrame`/`useAnimationFrame`;
  they must cancel on unmount or the side panel leaks frames.
- **ELM5 — Filter text is lowercased once in the component**, then matched
  against title/url — follow that shape in new pickers.

## Workflows

**Adding a new picker**
1. Put the query hook beside it as `<name>.ts` (kebab-case, like
   `use-available-tabs.ts`).
2. Build the popover from `ui/popover` + `ui/command` — that pairing is the
   established searchable-picker pattern.
3. Wrap the trigger with `PropsWithChildren` so callers own the anchor.

**Adding a motion flourish**
1. Use `motion/react` (`motion`, `useAnimationFrame`, `useMotionValue`,
   `useMotionTemplate`, `useTransform`) as `laser.tsx` does.
2. Take colours from CSS custom properties (`var(--accent-orange)`), not hex.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — component-tree rules (CMP2, CMP3).
- [`ui/AGENTS.md`](../ui/AGENTS.md) — `popover`, `command`, `dropdown-menu` bases.
- [`../theme-provider.tsx`](../theme-provider.tsx) — what `theme-toggle.tsx` drives.
- [`../../lib/workspace/AGENTS.md`](../../lib/workspace/AGENTS.md) — `useWorkspace`.
- [`../../lib/mcp/AGENTS.md`](../../lib/mcp/AGENTS.md) — `useMcpServers`, `useSyncRemoteIntegrations`.
