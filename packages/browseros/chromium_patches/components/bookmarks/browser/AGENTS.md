# `components/bookmarks/browser/` — show the bookmark bar by default

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A one-hunk unified diff against `bookmark_utils.cc`. In
`RegisterProfilePrefs`, the default for `prefs::kShowBookmarkBar` flips from
`false` to `true` so a fresh BrowserOS profile shows the bookmarks bar without
the user visiting the Appearance settings. The `SYNCABLE_PREF` annotation is
unchanged.

Feature block: **`chromium-ui-fixes`**.

## Contents

```
browser/
└── bookmark_utils.cc   ← @@ -450,7 +450,7 @@ : kShowBookmarkBar false → true
```

## Rules

**BKB1 — Change only the default argument.** The pref name, the type, and the
`SYNCABLE_PREF` flag are upstream contract. Flipping the flag would stop the
user's choice from syncing.

**BKB2 — This is a default, not an override.** Users who have already toggled
the bar have a value in their `Preferences` file; the pref default is
irrelevant to them. Do not add code to force the bar visible — that would
override the user's choice and is a different (much worse) change.

**BKB3 — Don't touch the other registrations in the same function.** The hunk
context includes `kEditBookmarksEnabled` and friends. Keeping the diff to one
line minimises conflict churn against Chromium's bookmark CLs.

**BKB4 — Related but separate: the CDP `Bookmarks` domain.** The protocol
surface for programmatic bookmark access lives in
[`third_party/blink/public/devtools_protocol/domains/Bookmarks.pdl`](../../../third_party/blink/public/devtools_protocol/domains/Bookmarks.pdl)
and its handler in `chrome/browser/devtools/protocol/bookmarks_handler.cc`.
Neither belongs in this folder; both must be changed together with the PDL.

## Workflows

**Verifying the flip took effect**
1. Build, run with a fresh `--user-data-dir`.
2. Confirm the bookmarks bar renders on the NTP without touching settings.
3. Confirm a pre-existing profile with `"show_bookmark_bar": false` is
   unaffected.

**Changing the default back or to a new value**
1. Edit the same line in `<chromium_src>/components/bookmarks/browser/bookmark_utils.cc`.
2. Extract back to this path.
3. Leave it under `chromium-ui-fixes`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/bookmarks/` rules.
- [`../AGENTS.md`](../../AGENTS.md) — `components/` subtree rules.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
