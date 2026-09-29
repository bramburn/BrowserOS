# `chrome/test/` — test target wiring

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/`) in
> [`packages/browseros/`](../../../AGENTS.md).

## What's here

One patch: `chrome/test/BUILD.gn`. It registers the BrowserOS server unit
tests with Chromium's `unit_tests` binary and adds the Sparkle link config
on macOS. There is no BrowserOS-specific test code at this level — unit
tests live next to the code they cover
(`chrome/browser/browseros/server/*_unittest.cc`), and the only WebUI test
data is in [`data/webui/settings/`](data/webui/settings/AGENTS.md).

## Contents

```
test/
├── BUILD.gn                     ← adds
│                                   "//chrome/browser/browseros/server:unit_tests"
│                                   to test("unit_tests") deps, and, under is_mac,
│                                   configs += [ "//third_party/sparkle:sparkle_link_test" ]
└── data/webui/settings/          ← import_data_dialog_test.ts (see its AGENTS.md)
```

## Rules

**TS1 — BrowserOS unit tests are pulled in here, not defined here.** The
`//chrome/browser/browseros/server:unit_tests` label must exist in
`chrome/browser/browseros/server/BUILD.gn`. A new BrowserOS test target is
useless until it is added to both files.

**TS2 — The Sparkle config is macOS-only and inside the `is_mac` branch.**
Moving it out breaks the Windows and Linux `unit_tests` link, because
`//third_party/sparkle:*` is not available there.

**TS3 — `unit_tests` is one giant binary.** Adding a target here compiles
every BrowserOS test in every build. Keep the label on a small
`source_set`/`test` with a focused `sources` list rather than pulling a
whole target in.

**TS4 — WebUI test data must mirror the Chromium test path.** The settings
dialog test lives at
`chrome/test/data/webui/settings/import_data_dialog_test.ts` because
Chromium's TS test runner looks for the mirrored path.

## Workflows

**Adding a BrowserOS unit test**
1. Write `*_unittest.cc` next to the code (e.g.
   `chrome/browser/browseros/server/browseros_server_utils_unittest.cc`).
2. Add it to the `sources` of the `unit_tests` target in the owning
   `BUILD.gn` (e.g. `chrome/browser/browseros/server/BUILD.gn`).
3. If it is a new target, expose a `unit_tests` label and add it to this
   file.
4. Run `out/Default/chrome --gtest_filter=*BrowserOS*`.

**Adding a WebUI TypeScript test**
1. Create
   `chrome/test/data/webui/<area>/<name>_test.ts` (mirroring the Chromium
   path).
2. Register the `.ts` in the corresponding `chrome/test/data/webui/**/BUILD.gn`
   or mojom test list in the Chromium tree.
3. Keep selectors in sync with the Lit templates the feature ships.

## Cross-references

- [`data/webui/settings/AGENTS.md`](data/webui/settings/AGENTS.md) — the
  only test data in this tree.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/` root overlay.
- [`../browser/browseros/server/AGENTS.md`](../browser/browseros/server/AGENTS.md)
  — the unit tests this file pulls in.
- [`../test/BUILD.gn`](../test/BUILD.gn) — the patched target.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `packages/browseros/`.
