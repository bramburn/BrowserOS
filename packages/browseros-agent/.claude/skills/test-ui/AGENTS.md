# `.claude/skills/test-ui/` — `/test-ui` extension UI testing

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

The largest skill in this package (10.9 KB) and a complete, opinionated
runbook for visually testing the `apps/agent/` extension UI. It starts the dev
environment, discovers CDP targets, then drives the two testable surfaces —
the new-tab page (`app.html`, left sidebar) and the side panel
(`sidepanel.html`, chat) — through a strict
`snapshot → identify IDs → click/fill → snapshot` loop using
`bun scripts/dev/inspect-ui.ts`. It also carries a troubleshooting table, the
full command reference, the known `app.html` routes, and a "gotchas learned
from real testing" list.

## Contents

```
test-ui/
└── SKILL.md    ← 10.9 KB: name, description, argument-hint: [what to test]
```

## Rules

**TU1 — Always re-snapshot after navigation or HMR.** React re-renders change
`backendDOMNodeId`-backed element IDs. Using stale IDs fails silently or
clicks the wrong thing. This is called out twice in the file — once as a
`CRITICAL` note in step 4, once in the gotchas.

**TU2 — Prefer `snapshot` over `screenshot`.** Snapshot is text, fast, and
answers structural questions. Reserve screenshots for layout, CSS, colours,
images, and a single final confirmation before commit. The file includes a
decision table for which to use.

**TU3 — The CDP port is randomised; read it from the output.** `bun run
dev:watch -- --new` prints `[info] Ports: CDP=… Server=… Extension=…`. Never
assume a port. On this Windows host set it as `$env:BROWSEROS_CDP_PORT`.

**TU4 — Wait for both readiness lines** — `[server] CDP ready` and
`[server] HTTP server listening` — before touching targets.

**TU5 — A fresh profile lands on onboarding.** Navigate to `#/home` first;
known routes are `#/home`, `#/settings`, `#/scheduled-tasks`, `#/onboarding`.
Hash routing is faster than clicking.

**TU6 — The side panel starts disabled** in a fresh profile. Use
`open-sidepanel`, which handles the BrowserOS-specific enable + toggle API;
a bare CDP open will not work.

**TU7 — Prerequisites are macOS-shaped.** The file asks for Go via `brew` and
`/Applications/BrowserOS.app/`. The `inspect-ui.ts` commands themselves are
portable, but the setup steps need translating on Windows/other hosts. Don't
copy the `brew` line verbatim.

**TU8 — Diagnostics live in the dev-server output and in `eval`.** Look at the
`[agent]` / `[server]` / `[build]` prefixes for build errors, and use
`eval <target> "…"` for JS errors and a `fetch('/health')` reachability check
when the UI does not render.

## Workflows

**Testing a change in `apps/agent/`**
1. `bun run dev:watch -- --new` in the background; read the CDP port.
2. `bun scripts/dev/inspect-ui.ts targets` → confirm `app.html` / `sidepanel.html` exist.
3. `eval app.html "window.location.hash = '#/home'"`, then `snapshot app.html`.
4. Interact: `click` / `fill` / `press_key` / `hover` / `select_option`, then
   **re-snapshot** after every interaction.
5. `screenshot <target> /tmp/x.png` for visual checks only; one final
   screenshot before commit.
6. Iterate: edit in `apps/agent/`, wait for WXT HMR, re-snapshot, verify.

**Chat-panel smoke test**
1. `open-sidepanel`, wait, `snapshot sidepanel`.
2. `fill sidepanel <textbox-id> "Hello world"` then `press_key sidepanel Enter`.
3. `wait_for sidepanel text "…"` and `snapshot` to confirm the response.

**When a target will not respond**
1. Symptom table in the file: blank page → JS error via `eval`; wrong element
   IDs → re-snapshot; `open-sidepanel` fails → wait longer; click does nothing
   → `scroll` first.

## Cross-references

- [`SKILL.md`](SKILL.md) — the runbook.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — the skills-tree overview and the portability caveat.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — § "Self-Testing UI Changes",
  the shorter version of this same CDP loop.
- [`../../../apps/agent/AGENTS.md`](../../../apps/agent/AGENTS.md) — the
  extension this skill tests.
- [`../../../scripts/dev/inspect-ui.ts`](../../../scripts/dev/inspect-ui.ts) — the CDP driver.
