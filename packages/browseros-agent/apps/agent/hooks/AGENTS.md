# `hooks/` — App-level React hooks

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent`.

## What's here

One file: `use-mobile.ts`, exporting `useIsMobile()`. It is the shadcn
viewport hook and reports `true` when `window.innerWidth < 768` using a
`matchMedia('(max-width: 767px)')` listener. `components.json` maps
`@/hooks` to this directory, so shadcn CLI-generated hooks land here too.

## Contents

```
hooks/
└── use-mobile.ts   ← useIsMobile(): 768px breakpoint, matchMedia-driven
```

## Rules

- **HK1 — Keep this directory tiny and generic.** Feature hooks live in
  `lib/<feature>/`; only reusable, app-wide UI hooks belong here. Compare
  `components/elements/use-available-tabs.ts`, which is colocated with its
  only consumer instead.
- **HK2 — Use the `@/hooks` alias, not a relative hop.** It is the alias
  `components.json` declares and the one shadcn CLI emits.
- **HK3 — Return a boolean, never `undefined`.** `useIsMobile()` coerces its
  initial `undefined` state with `!!isMobile` because the first client render
  happens before `useEffect` runs.
- **HK4 — Don't add a second breakpoint without checking `global.css`.** The
  `768` in `use-mobile.ts` is a literal `MOBILE_BREAKPOINT` and is not
  shared with the Tailwind theme.

## Workflows

**Adding a shared UI hook**
1. Create `hooks/<kebab-name>.ts` with no React component code.
2. Export a single `use<Name>()` hook with an explicit return interface.
3. Only promote it here if two or more `components/` call sites need it —
   otherwise co-locate it under `components/<feature>/`.

**Adding a shadcn hook via the CLI**
1. Run `bunx shadcn@latest add <component>` from
   `packages/browseros-agent/apps/agent`.
2. Confirm the generated file landed in `hooks/` per `components.json`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — extension root.
- [`../components.json`](../components.json) — declares `@/hooks`.
- [`../components/ui/AGENTS.md`](../components/ui/AGENTS.md) — the consumers of `useIsMobile`.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — file naming is kebab-case.
