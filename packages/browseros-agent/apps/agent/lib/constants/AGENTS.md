# `lib/constants/` — Shared constants

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

Small, dependency-free constant modules that several features need:
external URLs, the keyboard-shortcut table, the product web host (used by
the WXT manifest for web-accessible resources), the legacy extension id,
and — by far the largest file — the analytics event-name registry. Values
here are imported everywhere; changing one is a cross-cutting edit.

## Contents

```
constants/
├── productUrls.ts             ← docsUrl, productWebUrl, productRepositoryUrl,
│                                githubOrgUrl, privacyPolicyUrl,
│                                contributorsUrl, discordUrl, slackUrl,
│                                productVideoUrl, productRepositoryShortUrl,
│                                scheduledTasksHelpUrl
├── productWebHost.ts          ← PRODUCT_WEB_HOST = 'browseros.com'
├── shortcuts.ts               ← SHORTCUTS_LIST: per-platform modifier +
│                                key + description (Toggle Agent, Toggle LLM
│                                Chat, Switch providers)
├── legacyAgentExtensionId.ts  ← LEGACY_AGENT_EXTENSION_ID
├── mediaUrls.ts               ← media/asset URLs shown in the UI
└── analyticsEvents.ts         ← ~300 exported event-name constants,
                                 namespaced ui.*, settings.*, onboarding.*, …
```

## Rules

- **CONST1 — Every export is a `const`, never a function or an object
  literal that could be mutated at runtime.** All are marked `@public`.
- **CONST2 — Analytics event names live only in `analyticsEvents.ts`.** Adding
  one means appending a `…_EVENT` constant; never inline the string at a
  `track()` or `posthog.capture()` call site. `lib/chat-actions` takes event
  names as **props** precisely so the constants stay in one place.
- **CONST3 — URLs are absolute and `https`.** No relative paths and no
  protocol switching outside `wxt.config.ts`'s commented alpha variant.
- **CONST4 — `PRODUCT_WEB_HOST` feeds `wxt.config.ts` at build time.** It is
  imported by the manifest config for `web_accessible_resources` matches;
  changing it changes what the built extension exposes.
- **CONST5 — These are not a junk drawer.** Feature-specific constants belong
  in the feature folder (`lib/credits/credit-colors.ts`,
  `lib/jtbd-popup/constants.ts`, `lib/browseros/prefs.ts`). This directory is
  for values with cross-feature consumers only.
- **CONST6 — Prefer `@browseros/shared/constants/*`** (ports, timeouts,
  limits, urls) for values shared with the server, per
  [`../../../../CLAUDE.md`](../../../../CLAUDE.md).

## Workflows

**Adding an analytics event**
1. Append to `analyticsEvents.ts` following the existing namespace
   (`ui.*`, `settings.*`, `onboarding.*`, …) and the `/** @public */` marker.
2. Import the constant at the call site and pass it to `track()` or
   `posthog.capture()`.

**Adding a shortcut**
1. Append to `SHORTCUTS_LIST` in `shortcuts.ts` with the `windows`/`mac`/
   `linux` modifier triple and a description.
2. The newtab `ShortcutsDialog` renders the list — no registration needed.

**Adding a documentation link**
1. Prefer reusing an existing `…Url` constant from `productUrls.ts`.
2. Only add a new export if the URL is referenced from more than one feature.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB5, LIB7).
- [`../analytics/AGENTS.md`](../analytics/AGENTS.md) — PostHog capture.
- [`../metrics/AGENTS.md`](../metrics/AGENTS.md) — `track()`, the other consumer of these event names.
- [`../../wxt.config.ts`](../../wxt.config.ts) — imports `PRODUCT_WEB_HOST` and `LEGACY_AGENT_EXTENSION_ID`.
- [`../../../../packages/shared/AGENTS.md`](../../../../packages/shared/AGENTS.md) — the server-shared constants.
