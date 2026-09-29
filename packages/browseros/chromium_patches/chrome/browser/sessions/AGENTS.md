# `sessions/` — hidden Browsers are never persisted

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

One hunk in `session_service_base.cc`. In
`SessionServiceBase::ShouldTrackBrowser()`, BrowserOS adds an early `return
false` for any `Browser` whose `is_hidden()` is true, with the comment "Hidden
Browsers are ephemeral agent workspaces; never persist them." It sits before
the existing app-popup check, so a hidden Browser is excluded from session
tracking entirely rather than from restore.

## Contents

```
sessions/
└── session_service_base.cc   ← ShouldTrackBrowser():
                                  +if (browser->GetBrowserForMigrationOnly()->is_hidden())
                                  +  return false;
```

## Rules

**SES1 — The check is `GetBrowserForMigrationOnly()->is_hidden()`.** Use the
migration accessor, not `Browser::is_hidden()` directly: the migration wrapper
is the supported path and handles the pre-migration object state. Both forms
appear in Chromium; only this one is used here.

**SES2 — Placement matters: it is a `return false`, not a flag tweak.** The
function is a chain of exclusion checks. Inserting this one anywhere other than
immediately after the existing type checks risks running it against a
`Browser` subtype that does not support `is_hidden()`.

**SES3 — Hidden ≠ new. Do not extend this to "all BrowserOS windows".**
Only `CreateParams::hidden` windows are excluded. A normal BrowserOS window
must still be tracked, or "continue where I left off" stops working.

**SES4 — Session *restore* is a separate path.** The hunk only suppresses
tracking at creation time. If a hidden window can still be restored, the
companion change lives in `SessionRestoreImpl::CreateRestoredBrowser()` — the
same function the neighbouring comment in this file already warns about.

**SES5 — This patch is not in `features.yaml`.** Unlike most files under
`chrome/browser/`, `chrome/browser/sessions/session_service_base.cc` has no
entry in any `files:` list, so `browseros dev annotate` will apply it without
labelling it. When you touch it, add it to a feature block.

## Workflows

**Adding a new Browser type that must not be persisted**
1. Add its exclusion as a `return false` in `ShouldTrackBrowser()` beside this
   one.
2. Audit `SessionRestoreImpl::CreateRestoredBrowser()` for the matching restore
   path (SES4).
3. Keep the hunk minimal — a single early return.

**Checking whether a window will survive a restart**
1. Determine whether it was created with `CreateParams::hidden`
   (see `chrome/browser/browser.h`, upstream — not vendored here).
2. If hidden, it is excluded here and will not appear in the session.
3. If not, it is tracked normally regardless of BrowserOS branding.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay; BR4 explains the
  hidden-Browser contract.
- `chrome/browser/browser.h` — `CreateParams::hidden` / `is_hidden()`
  (upstream Chromium, patched but not vendored here).
- `chrome/browser/browser_list.cc` — the user-facing enumeration
  counterpart to this exclusion (upstream Chromium, patched but not
  vendored here).
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
