# `components/infobars/core/` — the infobar identifier enum

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory holding the one file Chromium keeps infobar identifiers
in: `components/infobars/core/infobar_delegate.h`. The BrowserOS patch adds a
single enumerator to `InfoBarDelegate::Identifier`.

## Contents

```
core/
└── infobar_delegate.h   ← +BROWSEROS_AGENT_INSTALLING_INFOBAR_DELEGATE = 135
```

## Rules

**IFC1 — Exactly one BrowserOS enumerator exists here (135).** Adding a second
one is a new feature, not a modification of `agent-v2-infobar`.

**IFC2 — This enum is mirrored in telemetry.** Keep the
`tools/metrics/histograms/metadata/browser/enums.xml` entry in lockstep; the
`LINT.ThenChange` comment sits directly under the closing brace and PRESUBMIT
enforces it.

**IFC3 — Chromium renumbers this enum as infobars are removed.** Keep the hunk
minimal (one `+` pair adjacent to the closing brace) so the 3-way merge in
`apply_single_patch` has the smallest possible conflict surface.

**IFC4 — Do not add `#include` lines.** `infobar_delegate.h` is included by
nearly every infobar consumer; a new include here costs build time repo-wide.

## Workflows

**Verifying the patch still applies after a Chromium bump**
1. `browseros dev apply --dry-run`.
2. On conflict, reset `<chromium_src>/components/infobars/core/infobar_delegate.h`
   to `BASE_COMMIT` and re-apply manually — never hand-edit hunk line numbers
   without re-extracting.

**Adding an infobar identifier**
1. Take the next free value after 135.
2. Insert it as the last enumerator.
3. Add the XML mirror in the same commit.
4. Re-extract the diff.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/infobars/` rules.
- [`../../../tools/metrics/histograms/metadata/browser/AGENTS.md`](../../../tools/metrics/histograms/metadata/browser/AGENTS.md) — the XML mirror.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `components/` subtree rules.
