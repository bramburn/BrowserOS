# `lib/changelog/` — Post-update changelog notification

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

A one-shot notification that opens the BrowserOS changelog page after a
notable extension update. It is deliberately opt-in **per version**:
`CHANGELOG_VERSIONS` is a whitelist keyed by the exact manifest version,
`changelog-storage.ts` remembers which versions have already been shown,
and `changelog-notifier.ts` opens the tab (after a 5 s delay) exactly once
per listed version.

## Contents

```
changelog/
├── changelog-config.ts    ← CHANGELOG_BASE_URL (docs.browseros.com/changelog);
│                            CHANGELOG_VERSIONS: Record<version,
│                            { showChangelog: true, anchor? }> — currently
│                            '0.0.52' and '0.0.55';
│                            getChangelogUrl(version), shouldShowChangelog(version)
├── changelog-storage.ts   ← changelogShownStorage (local:changelogShownVersions,
│                            string[]); hasShownChangelog(), markChangelogShown()
└── changelog-notifier.ts  ← checkAndShowChangelog(): version gate → dedupe →
                             5 s delayed chrome.tabs.create → mark as shown
```

## Rules

- **CHL1 — Unlisted versions show nothing.** The whitelist is the gate; the
  extension ships far more versions than changelog entries. Never make it
  "show for any version above X".
- **CHL2 — One tab per version, ever.** `markChangelogShown` appends before
  the tab opens are considered done; the dedupe in
  `changelog-notifier.ts` is what stops a second tab on next launch.
- **CHL3 — Version strings are manifest versions** from
  `chrome.runtime.getManifest().version`. They come from `package.json`
  `version` — the two must match for the entry to ever fire.
- **CHL4 — The `anchor` is a docs fragment** (`#v0-37-0`), and it does not
  have to match the extension version. Read it from the published changelog
  page rather than deriving it.
- **CHL5 — Keep the 5 s delay.** It stops the changelog tab racing the panel
  or new tab the user just opened after the update.
- **CHL6 — `chrome.tabs.create` is intentional** — a background-tab-worthy
  action, not an in-panel modal. Don't convert it to a route without a
  product decision.

## Workflows

**Adding a changelog for a new release**
1. Find the version string in `package.json` `version`.
2. Add `{ '0.0.NN': { showChangelog: true, anchor: 'vX-Y-Z' } }` to
   `CHANGELOG_VERSIONS` in `changelog-config.ts`.
3. Verify the anchor exists on `https://docs.browseros.com/changelog`.

**Changing the destination**
1. Edit `CHANGELOG_BASE_URL` in `changelog-config.ts`; `getChangelogUrl`
   appends the anchor.
2. Don't hard-code a URL in `changelog-notifier.ts`.

**Suppressing the notification entirely**
1. Empty `CHANGELOG_VERSIONS` (return `{}`) — `shouldShowChangelog` returns
   `false` for every version and the notifier becomes a no-op.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB3).
- [`../../package.json`](../../package.json) — the `version` these keys match.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — where `checkAndShowChangelog()` is called on startup.
