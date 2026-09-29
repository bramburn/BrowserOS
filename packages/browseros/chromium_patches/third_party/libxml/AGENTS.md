# `third_party/libxml/` — expose `xml_reader` to the BrowserOS server

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A single-line change to Chromium's libxml build file. The `xml_reader` static
library has a deliberately narrow `visibility` list (GN's term for "which
targets may depend on this"), and the BrowserOS server target is added to it so
`chrome/browser/browseros/server/` can parse the appcast/update XML.

Feature block: **`server`**.

## Contents

```
libxml/
└── BUILD.gn   ← + "//chrome/browser/browseros/server:server" in static_library("xml_reader") { visibility }
```

The hunk context shows the neighbouring entries — `//base/test:test_support` and
`//components/policy/core/common:unit_tests` — both Chromium-internal, both
parsing trusted data.

## Rules

**TLP1 — This is a security-relevant visibility grant.** `xml_reader` parses
XML, and the upstream comment in the file ties its narrow visibility to the
Security Team's review of untrusted-XML attack surface. Adding a target means
asserting that the new consumer only parses trusted input.

**TLP2 — Only `//chrome/browser/browseros/server:server` is sanctioned.** Any
other target added to this list needs a security review, not just a build fix.

**TLP3 — Insert alphabetically / next to the sibling entries.** The list is
roughly sorted by path; appending at the end is accepted but inserting out of
order invites future merge conflicts with upstream CLs.

**TLP4 — Keep the hunk to one line.** Every extra line in this file is
additional conflict surface on a file that upstream rewrites during each
libxml bump.

**TLP5 — This is not a version-number change, a feature flag, or a metric.**
`third_party/libxml/BUILD.gn` is easy to misfile under "misc"; it belongs to
`server` because the consumer is the bundled MCP server manager.

## Workflows

**The server fails to link against `xml_reader`**
1. Confirm the dependency is declared in
   `chrome/browser/browseros/server/BUILD.gn` as `//third_party/libxml`.
2. Confirm the `visibility` entry in this file is still present and matches
   the target label exactly.
3. Re-extract if the entry was added only to the patch and not the source.

**Evaluating a request to widen visibility**
1. Identify what the new consumer parses and whether it can be attacker-controlled.
2. If yes, reject and find a different library.
3. If no, add the entry next to its siblings and note the justification in the
   feature's `description:` in `features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `third_party/` subtree rules.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — the `server` block.
