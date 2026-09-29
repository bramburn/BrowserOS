# `chrome/browser/ui/views/relaunch_notification/` — update prompt threshold

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

One patch: `relaunch_notification_controller.cc`. It reclassifies
`UPGRADE_ANNOYANCE_VERY_LOW` so that it *closes* the relaunch notification
instead of showing it, lowering the threshold at which BrowserOS nags the
user to restart.

## Contents

```
relaunch_notification/
└── relaunch_notification_controller.cc   ←
      case UPGRADE_ANNOYANCE_NONE:
    - case UPGRADE_ANNOYANCE_VERY_LOW:
    -   /* comment about levels moving back down */
          CloseRelaunchNotification(); break;
    + case UPGRADE_ANNOYANCE_VERY_LOW:
      case UPGRADE_ANNOYANCE_LOW:
      case UPGRADE_ANNOYANCE_ELEVATED:
      case UPGRADE_ANNOYANCE_GRACE:
        /* show the notification */
```

## Rules

**RN1 — The switch labels are load-bearing, not cosmetic.** The patch moves
`VERY_LOW` from the close group to the show group. Moving the label
without moving the body (or vice versa) silently inverts the behaviour.

**RN2 — `UPGRADE_ANNOYANCE_NONE` still closes the notification.** Only
`VERY_LOW` changed. Do not "fix" `NONE` as part of a related change.

**RN3 — This is the Sparkle-era behaviour.** The file is listed under the
`mac-sparkle-updater` feature in `packages/browseros/build/features.yaml`,
where the app-menu update indicator severity is also raised
(`chrome/browser/ui/toolbar/app_menu_icon_controller.cc`). Treat the two as
one policy: BrowserOS surfaces updates later but insists on the relaunch
once the annoyance level is `LOW` or above.

**RN4 — No ChromeOS/Windows equivalent is patched here.** On other
platforms the relaunch flow is driven by the platform update detector.

## Workflows

**Changing how urgently the user is asked to restart**
1. Move the `UPGRADE_ANNOYANCE_*` case labels in the `switch` in
   `OnUpgradeRecommended()`.
2. Keep `NONE` in the close branch.
3. Cross-check `chrome/browser/ui/toolbar/app_menu_icon_controller.cc` for
   the menu indicator severity.

**Verifying the notification appears**
Run the browser with a staged update in the appcast feed, or lower the
annoyance level in `chrome://flags` terms. `LOW` and above should show the
notification; `VERY_LOW` and `NONE` should not.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/`.
- [`../../toolbar/app_menu_icon_controller.cc`](../../toolbar/app_menu_icon_controller.cc) —
  the menu-side update indicator.
- [`../../webui/help/AGENTS.md`](../../webui/help/AGENTS.md) —
  the Sparkle `VersionUpdater` that feeds the detector.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
