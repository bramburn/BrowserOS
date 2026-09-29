# `net/` — alternate error pages off by default

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

One file, one line, one boolean. `profile_network_context_service.cc` changes
the registered default of `embedder_support::kAlternateErrorPagesEnabled` from
`true` to `false` in `ProfileNetworkContextService::RegisterProfilePrefs()`.
With it off, Chromium renders its own error page instead of the embedder
(interstitial) error pages, which is what BrowserOS wants for the side panel
and the agent-driven windows.

## Contents

```
net/
└── profile_network_context_service.cc   ← RegisterProfilePrefs():
                                             kAlternateErrorPagesEnabled: true → false
```

## Rules

**NET1 — This is a default, not a removal.** The pref is still registered; only
its default flipped. A user or policy that sets it back to `true` still gets
the embedder pages. Deleting the `RegisterBooleanPref` call would break
`chrome://settings` and any policy that targets it.

**NET2 — Neighbouring pref defaults in this function are untouched.**
`kQuicAllowed` (still `true`) and `kGloballyScopeHTTPAuthCacheEnabled` (still
`false`) sit on adjacent lines. Keep the hunk to the one value; every extra
changed line is a conflict risk on the next Chromium bump.

**NET3 — Changing it back requires understanding the side panel.**
The embedder error pages are what `//chrome/browser/ui/webui/side_panel`
overlays render. Restoring `true` will change what a user sees on a failed
navigation inside a BrowserOS side panel.

**NET4 — The pref is `SYNCABLE`-unmarked here**, so it stays profile-local and
does not propagate through the syncable-pref table in
`../sync/prefs/chrome_syncable_prefs_database.cc`. Do not add it to the
allowlist without deciding that deliberately.

## Workflows

**Flipping a network pref default**
1. Edit the value in
   `ProfileNetworkContextService::RegisterProfilePrefs()` in
   `profile_network_context_service.cc`.
2. Keep the pref registered.
3. Confirm nothing else in the browser process reads the old default at
   startup.
4. Re-extract the single-hunk diff and keep it in the same `features.yaml`
   block.

**Tracing where the pref is consumed**
1. Search for `kAlternateErrorPagesEnabled` in the Chromium tree — the network
   service reads it to decide whether to use the embedder-provided error page.
2. Check `../net/BUILD.gn`-style deps of the consuming target if the value is
   expected at build time rather than runtime.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`ui/AGENTS.md`](../ui/AGENTS.md) — the side panel that renders error pages.
- [`../prefs/AGENTS.md`](../prefs/AGENTS.md) — profile pref registration.
- [`../sync/prefs/AGENTS.md`](../sync/prefs/AGENTS.md) — the syncable-pref
  allowlist this pref is *not* in.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
