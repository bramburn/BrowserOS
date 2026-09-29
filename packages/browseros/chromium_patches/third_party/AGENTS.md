# `third_party/` — vendored dependency patches

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../chromium_patches/AGENTS.md`](../../chromium_patches/AGENTS.md).

## What's here

Three vendored dependency trees, three unrelated reasons to touch them.
Everything under `third_party/` is the most merge-fragile area of the overlay:
vendored code is updated by automated import CLs, so upstream churn lands here
first and hardest.

| Subtree | What changes | Feature |
|---|---|---|
| `blink/` | DevTools Protocol `.pdl` definitions — BrowserOS adds `Bookmarks`, `History`, and extends `Browser`/`Target` | `cdp-api` |
| `libxml/` | One line: `//chrome/browser/browseros/server:server` added to `xml_reader`'s `visibility` | `server` |
| `sparkle/` | A `new file` BUILD.gn wiring the macOS Sparkle framework | `sparkle-third-party` |

## Contents

```
third_party/
├── blink/
│   ├── public/devtools_protocol/
│   │   ├── BUILD.gn
│   │   ├── browser_protocol.pdl
│   │   └── domains/{Bookmarks,Browser,History,Target}.pdl
│   └── renderer/core/frame/navigator.cc
├── libxml/
│   └── BUILD.gn
└── sparkle/
    └── BUILD.gn
```

## Rules

**T3P1 — `third_party/` patches are the first to break on a version bump.**
Vendored import CLs rewrite these files wholesale. When a patch conflicts,
reset the file to `BASE_COMMIT` and re-apply by hand — do not hand-edit hunk
offsets, which silently misapplies.

**T3P2 — `third_party/blink/public/devtools_protocol/domains/Bookmarks.pdl`
and `History.pdl` are BrowserOS-authored files.** They are `new file mode`
diffs, not edits to upstream domains, so they have no upstream counterpart to
drift against. Treat them as BrowserOS source: readable, documented, owned.

**T3P3 — The `visibility` line in `libxml/BUILD.gn` is a security control.**
`xml_reader` is deliberately not widely visible because it parses untrusted
input. Adding a target to `visibility` grants it the ability to parse
attacker-controlled XML. The only sanctioned addition is
`//chrome/browser/browseros/server:server`, and only because that server parses
trusted local files. Treat any further addition as a security review item.

**T3P4 — `third_party/sparkle/BUILD.gn` is a `new file` diff.** The
`Sparkle.framework` binary it references is downloaded at build time by the
`mac-sparkle-updater` / `common/sparkle.py` machinery — it is **not** in this
repo. The BUILD.gn failing to find the framework is a missing-download problem,
not a patch problem.

**T3P5 — Never edit vendored code beyond the BrowserOS need.** A large
divergence from upstream in `third_party/` multiplies the cost of every future
Chromium bump. Keep each hunk to the minimum that delivers a feature.

**T3P6 — `.pdl` changes require the generated bindings and the handler.** A
`.pdl` edit alone ships a protocol that no client can use; the handler lives in
`chrome/browser/devtools/protocol/` and in
`content/browser/devtools/protocol/target_handler.cc`.

## Workflows

**Adding a CDP domain**
1. Create `domains/<Name>.pdl` in `<chromium_src>/third_party/blink/public/devtools_protocol/domains/`.
2. Register it in the `domains` list in that directory's `BUILD.gn`.
3. Add `include domains/<Name>.pdl` to `browser_protocol.pdl`, alphabetically.
4. Implement the handler in `chrome/browser/devtools/protocol/<name>_handler.{h,cc}`.
5. Extract steps 1–3 to this overlay; keep them all under `cdp-api`.

**Adding a BrowserOS field to an existing domain**
1. Edit `domains/<Name>.pdl` — mark new members `experimental` and `optional`
   so existing clients keep working.
2. Update the handler.
3. Re-extract both.

**A vendored file conflicts after a Chromium bump**
1. `git checkout <BASE_COMMIT> -- third_party/<path>` inside `<chromium_src>`.
2. Re-apply the BrowserOS change by hand.
3. `browseros dev extract <path>` to regenerate the diff.
4. Re-run `browseros dev apply --dry-run` across the whole overlay.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — overlay conventions.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`blink/AGENTS.md`](blink/AGENTS.md) — the CDP protocol subtree.
- [`../../build/features.yaml`](../../build/features.yaml) — `cdp-api`, `server`, `sparkle-third-party`.
