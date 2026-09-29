# `components/search/` — NTP footer off by default

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A one-line flip in `ntp_features.cc`. The `kNtpFooter` `base::Feature` is
declared `FEATURE_ENABLED_BY_DEFAULT` in Chromium; BrowserOS declares it
`FEATURE_DISABLED_BY_DEFAULT`, so the new-tab-page footer (Google logo,
attribution links) is off unless a user turns it on at
`chrome://flags/#ntp-footer`.

Feature block: **`chromium-ui-fixes`**.

## Contents

```
search/
└── ntp_features.cc   ← @@ -197,7 +197,7 @@ : kNtpFooter ENABLED → DISABLED
```

## Rules

**SE1 — Flip the default, don't delete the feature.** The `BASE_FEATURE`
declaration and its `// If enabled, a footer will show on the NTP.` comment
stay. Deleting the symbol breaks `chrome://flags` and any
`base::Feature` reference elsewhere.

**SE2 — Chromium's next NTP rewrite renames or removes this file.** This is a
high-drift target; expect `--3way` conflicts on a version bump and prefer
re-extracting over hand-patching hunk headers.

**SE3 — `components/search/` has other feature declarations** in the same
file (`kNtpNextFeatures`, `kNtpOneGoogleBarAsyncBarParts`,
`kNtpTabGroupsModule`, ...). Only `kNtpFooter` changes. A diff that touches
more than one `BASE_FEATURE` line is wrong.

**SE4 — Flags UI still works.** `chrome/browser/flag_descriptions.h` (under
`chrome/`, the only flag-description file the overlay patches) describes this
flag; it is unaffected and must stay in sync.

## Workflows

**Turning the footer back on by default**
1. Edit `<chromium_src>/components/search/ntp_features.cc`.
2. Change `kNtpFooter` back to `base::FEATURE_ENABLED_BY_DEFAULT`.
3. Extract back to this path; keep it under `chromium-ui-fixes`.

**Deciding whether a new NTP feature belongs here**
1. New NTP UI belongs under `chrome/browser/resources/settings/` or
   `chrome/browser/ui/webui/new_tab_footer/` (both `chrome/`).
2. Only feature *declarations* go in `components/search/ntp_features.cc`.
3. Add the path to `features.yaml`; a new feature deserves its own block.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/` subtree rules.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — the `chromium-ui-fixes` block.
