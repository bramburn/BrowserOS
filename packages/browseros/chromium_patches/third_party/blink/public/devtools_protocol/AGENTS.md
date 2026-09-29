# `third_party/blink/public/devtools_protocol/` — CDP root schema

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The two files that make a CDP domain part of the protocol at all. A `.pdl`
file under `domains/` does nothing until the build is told about it in two
places: the `domains` list in this directory's `BUILD.gn`, and the `include`
block in `browser_protocol.pdl`.

BrowserOS adds two domains this way — `Bookmarks` and `History`.

## Contents

```
devtools_protocol/
├── BUILD.gn            ← + "domains/Bookmarks.pdl", + "domains/History.pdl" in domains list
├── browser_protocol.pdl ← + include domains/Bookmarks.pdl, + include domains/History.pdl
└── domains/            ← the domain schemas themselves
```

Both lists are the upstream ones with one line inserted each. The
`Bookmarks.pdl` include sits between `Autofill.pdl` and `BackgroundService.pdl`,
which is **not** the alphabetical slot Chromium would pick (`Bookmarks` sorts
after `BluetoothEmulation`, before `Browser`); the `History.pdl` include does sit
in its alphabetical position. Keep the placement as-is for re-extracts, and do
not "fix" one without fixing the other.

## Rules

**DV1 — Two registrations or the domain is dead.** A `.pdl` in `domains/`
without a `BUILD.gn` entry is not compiled; without the `browser_protocol.pdl`
include it is not part of the generated protocol. Always add both.

**DV2 — Match where each existing line sits, not an imagined sort order.** The
`domains` list and the `include` block are both upstream-ordered, and
Chromium PRESUBMITs sorted GN string lists — but the existing `Bookmarks.pdl`
line is not in its sorted position. When you add a line, place it next to the
same anchor the current extraction used, then expect a future conflict.

**DV3 — `BUILD.gn` is the generator input, not a build target.** It feeds
`tools/gn/generate_protocol_completion_data.py` and the protocol generator. Do
not add `deps`, `sources` beyond the list, or a `group()` here.

**DV4 — Every new domain needs a handler before it is useful.** Schema plus
registration yields a domain the client can call but the browser cannot serve.
Handlers live under `chrome/browser/devtools/protocol/` (Chrome-level domains)
and `content/browser/devtools/protocol/` (target-level domains).

## Workflows

**Registering a new domain**
1. Create `domains/<Name>.pdl`.
2. Add `"domains/<Name>.pdl"` to the `domains` list in `BUILD.gn`, sorted.
3. Add `include domains/<Name>.pdl` to `browser_protocol.pdl`, sorted.
4. Implement the handler under `chrome/browser/devtools/protocol/`.
5. Extract all three touched files to `chromium_patches/`; keep under `cdp-api`.

**Diagnosing "my new domain isn't in the protocol JSON"**
1. Is the file in the `domains` list in `BUILD.gn`?
2. Is the `include` present in `browser_protocol.pdl`?
3. Did the protocol generator rerun — check
   `out/Default/.../protocol.json` for the domain name.
4. Does the handler exist and is it registered with the agent host?

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `blink/public/` rules.
- [`domains/AGENTS.md`](domains/AGENTS.md) — the domain schemas.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.
