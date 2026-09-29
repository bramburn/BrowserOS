# `resources/settings/browseros_prefs_page/` — the BrowserOS Settings page

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The fork's own settings page, served at `chrome://settings/browseros-settings`.
Two new files (both `new file mode` patches): a Polymer element built on
`PrefsMixin` and a hand-written HTML template with inline CSS. It exposes one
preference — `browseros.show_toolbar_labels`, via a `settings-toggle-button` —
and contains a "Custom AI Providers" section that is currently
`style="display: none;"`, wired to `browseros.custom_providers` as a JSON string
with add/delete handled by a `<cr-dialog>`. The element is
`settings-browseros-prefs-page`.

## Contents

```
browseros_prefs_page/
├── browseros_prefs_page.ts     ← SettingsBrowserOSPrefsPageElement
│                                  (PolymerElement + PrefsMixin); customElements.define
│                                  in browseros_prefs_page.html.js
└── browseros_prefs_page.html   ← 336 lines: inline <style include="cr-shared-style
                                   settings-shared md-select">, page header,
                                   toolbar toggle bound to
                                   prefs.browseros.show_toolbar_labels, the hidden
                                   custom-providers section, the <cr-dialog>, and the
                                   status toast
```

## Rules

**BPP1 — Prefs are read and written by string literal, not by the
`syncable-prefs` JSON.** `this.getPref('browseros.custom_providers')` and
`this.setPrefValue('browseros.show_toolbar_labels', …)`. The constants live in
`../../../../browseros/core/browseros_prefs.h`; if one is renamed, this page
breaks silently because the string no longer resolves. Keep the literals in
sync with that header.

**BPP2 — Custom providers are stored as a JSON string in a string pref.**
`loadCustomProviders_()` does `JSON.parse(pref.value)` and
`saveCustomProviders_()` does `JSON.stringify(this.customProviders)`. The
underlying pref is `PrefType::kString` in the `settingsPrivate` allowlist —
changing it to a list type breaks both halves.

**BPP3 — The custom-providers section is hidden on purpose.** The enclosing
`.prefs-section` carries `style="display: none;"`. Do not remove the attribute
while the flow is unstable; the dialog code is still being iterated on.

**BPP4 — The element must be `customElements.define`d and exported.**
`settings.ts` re-exports `SettingsBrowserOSPrefsPageElement`, and
`settings_main/settings_main.ts` imports the module for the side effect. The
`declare global { interface HTMLElementTagNameMap { 'settings-browseros-prefs-page': … } }`
block is what makes the tag type-safe in templates — keep it.

**BPP5 — `BUILD.gn` must list the `.ts`.**
`build_webui("build")` includes
`"browseros_prefs_page/browseros_prefs_page.ts"`. There is no local
`BUILD.gn`.

**BPP6 — The template has no trailing newline in the diff.**
Both files end with `\ No newline at end of file`. Adding one is a one-line
diff churn on every extract; leave the file endings as they are unless a real
edit requires otherwise.

**BPP7 — Status messages use CSS animation, not a framework toast.**
`showStatusMessage_()` toggles a `.show` class, forces reflow via
`void statusMessage.offsetWidth`, and removes the class after 2 s. A new
message must reuse that path rather than introducing a second mechanism.

## Workflows

**Exposing a new `browseros.*` pref on this page**
1. Confirm the pref is registered in `../../prefs/browser_prefs.cc` and
   allowlisted in
   `../../extensions/api/settings_private/prefs_util.cc`.
2. Add a `settings-toggle-button` (or the appropriate control) to
   `browseros_prefs_page.html` with `pref="{{prefs.browseros.x}}"`.
3. If it is a string pref holding JSON, mirror the load/save pattern from
   `loadCustomProviders_()` / `saveCustomProviders_()`.
4. Extract both files into the same `features.yaml` block.

**Changing the page title or route**
1. The title lives in `../route.ts`
   (`new Route('/browseros-settings', 'BrowserOS Settings')`).
2. The URL must match the `view` slot check in
   `../settings_main/settings_main.html` via `routes_.BROWSEROS_PREFS`.
3. The `H1` text ("BrowserOS Preferences") is hard-coded in
   `browseros_prefs_page.html`; there is no `$i18n{}` for it.

**Debugging a pref that does not persist**
1. Check the literal in the `.ts`/`.html` against
   `browseros_prefs.h` (BPP1).
2. Check the `settingsPrivate` allowlist row exists with the matching
   `PrefType`.
3. Check the element is imported in `../settings_main/settings_main.ts` and
   exported in `../settings.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `settings/` app, routes and exports.
- [`../../../browseros/core/AGENTS.md`](../../../browseros/core/AGENTS.md)
  — the `browseros.*` pref constants and their types.
- [`../../../extensions/api/settings_private/AGENTS.md`](../../../extensions/api/settings_private/AGENTS.md)
  — the allowlist rows for `kShowToolbarLabels`, `kShowLLMChat`,
  `kShowLLMHub`, `kProviders`, `kCustomProviders`.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.
