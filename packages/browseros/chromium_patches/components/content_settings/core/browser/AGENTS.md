# `components/content_settings/core/browser/` — block third-party cookies

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A one-hunk unified diff against `cookie_settings.cc`. In
`CookieSettings::RegisterProfilePrefs`, the default for `prefs::kCookieControlsMode`
changes from `CookieControlsMode::kIncognitoOnly` (Chromium's default — the
3P cookie setting only applies in incognito) to `CookieControlsMode::kBlockThirdParty`
(third-party cookies blocked in all profiles).

Feature block: **`chromium-ui-fixes`**.

## Contents

```
browser/
└── cookie_settings.cc   ← @@ -83,7 +83,7 @@ : kCookieControlsMode kIncognitoOnly → kBlockThirdParty
```

## Rules

**CSCB1 — `kBlockThirdParty` is the strict mode; do not soften it without an
explicit product decision.** It changes login flows, embedded widgets, and ads
for every new profile.

**CSCB2 — Keep the `SYNCABLE_PREF` annotation.** Users who change the
setting in `chrome://settings/content/cookies` sync it; removing the flag
decouples the choice from the account.

**CSCB3 — The pref default does not override an existing profile.** Profiles
that already recorded `kCookieControlsMode` keep their value. Do not add
migration code here to force the new default — that would silently override
user choice on upgrade.

**CSCB4 — Third-party cookies also affect CDP-driven automation.** The bundled
MCP server drives a real browser; login flows that rely on 3P cookies will
behave differently under this default. Treat "works in the agent" vs "works
in a fresh profile" as two separate test cases.

## Workflows

**Verifying the new default**
1. Build and launch with a fresh `--user-data-dir`.
2. Check `chrome://settings/content/cookies` shows the strict setting without
   a manual change.
3. Sanity-check a login flow on a site that uses a third-party iframe.

**Changing the mode**
1. Edit `<chromium_src>/components/content_settings/core/browser/cookie_settings.cc`.
2. Extract back to this path.
3. Leave it under `chromium-ui-fixes`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `core/` path rules.
- [`../../AGENTS.md`](../../AGENTS.md) — `content_settings/` path rules.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `components/` subtree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.
