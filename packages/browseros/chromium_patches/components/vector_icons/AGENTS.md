# `components/vector_icons/` — BrowserOS toolbar icons

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

Two BrowserOS toolbar icons and the GN registration that makes the build pick
them up. `chat_orange.icon` backs the LLM chat side panel entry;
`clash_of_gpts.icon` backs the "Clash of GPTs" panel. Both are Skia tangent
command files in Chromium's `.icon` format (not SVG, not PNG), delivered as
`new file mode` unified diffs.

Feature block: **`branding`**.

## Contents

```
vector_icons/
├── BUILD.gn              ← + "chat_orange.icon" and + "clash_of_gpts.icon" in sources
├── chat_orange.icon      ← new-file diff, 48 lines of Skia path commands
└── clash_of_gpts.icon    ← new-file diff
```

Both entries go into the `sources` list of the `//components/vector_icons`
target, kept in the file's alphabetical ordering that Chromium enforces:
`chat_orange.icon` between `chat.icon` and `chat_spark.icon`,
`clash_of_gpts.icon` between `checklist.icon` and `close.icon`.

## Rules

**VI1 — The `.icon` file is a diff, not an asset.** The content you see is
`+++`-prefixed. Editing it as if it were the raw icon file will corrupt the
patch. Regenerate via `browseros dev extract` after editing the real file in
`<chromium_src>/components/vector_icons/`.

**VI2 — An unregistered icon is dead weight.** The `BUILD.gn` hunk is what puts
the file in `//components/vector_icons`. Skipping it produces a patch that
applies cleanly, compiles cleanly, and shows no icon.

**VI3 — Respect alphabetical ordering.** Chromium's vector icon build has a
PRESUBMIT that checks sorted source lists. Insert at the right position, not at
the end.

**VI4 — Canvas size is uniform.** Chromium icons are authored on a
`CANVAS_DIMENSIONS, 960,` grid with `FILL_RULE_NONZERO`. Deviating produces
misaligned toolbar glyphs that are tedious to debug visually.

**VI5 — Icons are branding.** They appear in the shipped toolbar and in
`chrome://flags` descriptions. Any change is a product/design decision, not a
build fix.

**VI6 — Renaming an icon is a three-file change:** the `.icon` file path, the
`BUILD.gn` entry, and every `kResourceId` / `icon::` reference under
`chrome/browser/ui/`. Update all three or the toolbar renders a fallback.

## Workflows

**Adding a new toolbar icon**
1. Author the `.icon` at `<chromium_src>/components/vector_icons/<name>.icon`
   (960×960, `FILL_RULE_NONZERO`, alphabetical-safe name).
2. Extract → produces a `new file mode` diff at this path.
3. Add the filename to `sources` in
   `<chromium_src>/components/vector_icons/BUILD.gn` in sorted position, and
   extract that too.
4. Add both paths to the `branding` feature in `features.yaml`.
5. Reference it from the toolbar button under `chrome/browser/ui/views/toolbar/`.

**Debugging a missing icon**
1. Confirm the file exists in `<chromium_src>/components/vector_icons/`.
2. Confirm it is listed in that directory's `BUILD.gn` `sources`.
3. Confirm the `chrome://flags`-style resource id in
   `components/vector_icons/vector_icons.cc` (unpatched upstream file) resolves.
4. Re-extract if step 1 or 2 was done only in the patch.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/` subtree rules.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — the `branding` block.
