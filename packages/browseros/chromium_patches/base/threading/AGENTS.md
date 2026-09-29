# `base/threading/` — server manager thread-restriction friends

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

One file, one diff. `thread_restrictions.h` is Chromium's enforcement point for
"you may not block the thread / take base sync primitives here". The patch
registers `browseros::BrowserOSServerManager` as a `friend` of two guard
classes so the bundled MCP server manager can do the blocking I/O it needs
while starting, health-checking, and restarting `browseros_server.exe`.

`features.yaml` assigns this file to the **`server`** feature block (not
`branding`).

## Contents

```
threading/
└── thread_restrictions.h   ← +forward decl, +2 friend lines
```

Three hunks, all additions:

| Hunk | Change |
|---|---|
| ~line 205 | `namespace browseros { class BrowserOSServerManager; }` forward declaration |
| ~line 617 | `friend class browseros::BrowserOSServerManager;` in `ScopedAllowBlocking` |
| ~line 767 | `friend class browseros::BrowserOSServerManager;` in `ScopedAllowBaseSyncPrimitives` |

## Rules

**BT1 — The forward declaration must precede the friend lines.** The header
declares `friend class browseros::BrowserOSServerManager;` in two places;
removing the `namespace browseros` block breaks both at once.

**BT2 — Both guards need the friend, or only half the block is legal.** Adding
it to `ScopedAllowBlocking` alone leaves the base-sync-primitive check still
failing at runtime (a CHECK, not a compile error) — which is worse than a
build break because it only shows under load.

**BT3 — This is a per-file diff, not a directory convention.** Adding a
sibling file here means it is a new `chromium_patches` entry, and the mirror
path must be exactly `base/threading/<name>`.

**BT4 — Don't widen the exemption.** The friend is scoped to one concrete
class. Promoting it to a base class or adding `namespace browseros` wholesale
grants the exemption to the entire MCP server codebase.

## Workflows

**Blocking call fails a `ScopedAllowBlocking` CHECK at runtime**
1. Confirm the stack shows `BrowserOSServerManager` in the blocked scope.
2. If yes, the cause is elsewhere in the server code — do **not** add a new
   friend here. Fix the blocking call in
   `chrome/browser/browseros/server/process_controller_impl.cc`.
3. Only if a genuinely new class needs the exemption: edit
   `<chromium_src>/base/threading/thread_restrictions.h`, add its forward
   declaration plus a `friend class` line to the specific guard, and extract.

**Verifying this patch still applies**
- `browseros dev apply --dry-run` will report drift against
  `BASE_COMMIT`; the friend list is re-ordered by upstream CLs fairly often.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `base/` subtree rules.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — the `server` block.
