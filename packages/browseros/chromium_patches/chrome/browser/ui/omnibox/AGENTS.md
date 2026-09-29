# `chrome/browser/ui/omnibox/` — virtual URL display in the omnibox

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

One patch: `chrome_omnibox_client.cc`. When the location bar holds a
`chrome-extension://` URL belonging to a BrowserOS extension, the omnibox
displays the friendly `chrome://browseros/…` virtual URL instead.

## Contents

```
omnibox/
└── chrome_omnibox_client.cc   ← in the URL-to-text path, when
                                 url.SchemeIs(extensions::kExtensionScheme):
                                 virtual_url = browseros::GetBrowserOSVirtualURL(
                                     url.host(), url.path(), url.ref());
                                 if non-empty → display it instead of url.spec()
```

## Rules

**OB1 — Display only; this does not navigate.** The patch changes what the
omnibox *shows*. Navigation is handled by the forward handler in
`chrome/browser/chrome_content_browser_client.cc`. Do not implement
navigation logic here.

**OB2 — The mapping is table-driven, not hard-coded.**
`browseros::GetBrowserOSVirtualURL()` and `kBrowserOSURLRoutes` live in
`chrome/browser/browseros/core/browseros_constants.h`. A new virtual route
is a new table row there, not a new `if` in this file.

**OB3 — Guarded by `#if BUILDFLAG(ENABLE_EXTENSIONS)`.** The whole block
compiles out on builds without extensions. Keep new code inside that guard.

**OB4 — An unmapped extension URL falls through to `url.spec()`.** An
empty return from `GetBrowserOSVirtualURL` means "not a BrowserOS route";
this file must keep displaying the real URL in that case.

**OB5 — Keep it symmetric with the copy path.** `browser/ui/browser_commands.cc`
applies the same transformation for Copy URL. Both call
`GetBrowserOSVirtualURL` with `(host, path, ref)`.

## Workflows

**Adding a `chrome://browseros/*` route**
1. Add a `BrowserOSURLRoute` row in
   `chrome/browser/browseros/core/browseros_constants.h`
   (`virtual_path`, `extension_id`, `extension_page`, `extension_hash`).
2. Nothing is needed in this directory — the display and the forward
   navigation both read that table.
3. Verify by navigating to the extension page and checking the omnibox.

**Debugging a wrong URL in the address bar**
1. Confirm the route row's `extension_hash` matches the real fragment
   (including the leading `/`).
2. Confirm the omnibox is showing the *active tab's* URL
   (`GetLocationBarModel()->GetURL()`), not a stale model.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`../browser_commands.cc`](../browser_commands.cc) — the Copy URL twin of
  this transform.
- [`../../browseros/core/AGENTS.md`](../../browseros/core/AGENTS.md) —
  `kBrowserOSURLRoutes` and the URL helpers.
- [`../../chrome_content_browser_client.cc`](../../chrome_content_browser_client.cc)
  — the forward/reverse navigation handlers.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
