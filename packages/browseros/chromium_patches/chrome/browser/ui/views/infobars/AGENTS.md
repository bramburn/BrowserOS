# `chrome/browser/ui/views/infobars/` — infobar shadow height

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

One patch: `infobar_container_view.cc`. In `Layout(Pass)`, the content
shadow under the infobar is given a 1 px bound instead of its preferred
height, producing a hairline separator rather than Chromium's drop shadow.

## Contents

```
infobars/
└── infobar_container_view.cc   ←
      - content_shadow_->SetBounds(0, top, width(),
                                   content_shadow_->GetPreferredSize().height());
      + content_shadow_->SetBounds(0, top, width(), 1);
```

## Rules

**IF1 — Height 1 is intentional and hard-coded.** A "restore preferred
size" change here reverts the BrowserOS hairline and is a visible product
regression, not a bug fix.

**IF2 — The comment above the call is now stale.** It explains why the
shadow is drawn outside the container bounds, which no longer explains the
1 px bound. If you edit this block, update the comment rather than leaving
it to mislead the next reader.

**IF3 — Infobar *height* is not set here.** It comes from
`ChromeLayoutProvider::GetDistanceMetric` in
[`../chrome_layout_provider.cc`](../chrome_layout_provider.cc), which BrowserOS also patches
(non-refresh infobars: `36 + 2 * 3`). Change the two independently and
verify together.

**IF4 — Owned by the `chromium-ui-fixes` feature** in
`packages/browseros/build/features.yaml`, alongside
`chrome_layout_provider.cc` and
`chrome/browser/ui/startup/infobar_utils.cc`.

## Workflows

**Changing the infobar's visual weight**
1. Shadow thickness: `infobar_container_view.cc` (this directory).
2. Vertical padding: `../chrome_layout_provider.cc`.
3. Which infobars appear: `../../startup/infobar_utils.cc`.

**Adding a BrowserOS infobar**
1. Create the delegate in
   `chrome/browser/infobars/` (new file patch).
2. Add its `IDS_` strings to `chrome/app/generated_resources.grd`.
3. Add the create call in `../../startup/infobar_utils.cc`.
4. Add both paths to a feature block in `build/features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/`.
- [`../chrome_layout_provider.cc`](../chrome_layout_provider.cc) — infobar distance metrics.
- [`../../startup/AGENTS.md`](../../startup/AGENTS.md) — which startup
  infobars are created.
- [`../../../browseros/core/browseros_switches.h`](../../../browseros/core/browseros_switches.h)
  — the agent-v2 infobar switch.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
