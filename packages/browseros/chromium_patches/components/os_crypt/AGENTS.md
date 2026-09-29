# `components/os_crypt/` — macOS keychain branding

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A single-subdirectory branch holding the macOS keychain naming patch. Chromium
names its keychain entries after the product ("Chrome Safe Storage" or
"Chromium Safe Storage"); BrowserOS uses its own so the login keychain shows
"BrowserOS Safe Storage" rather than a Chromium-branded entry.

Feature block: **`chrome-importer`**.

## Contents

```
os_crypt/
└── common/
    └── keychain_password_mac.mm   ← service/account name → "BrowserOS"
```

## Rules

**OS1 — This string is a migration boundary, not cosmetics.** macOS stores
encrypted credentials in the login keychain keyed by service + account name.
Changing it makes previously saved passwords invisible — the user is prompted
to re-authenticate. Do not change it casually, and never as part of a
refactor.

**OS2 — The `#if BUILDFLAG(IS_CHROME_BRANDING)` / `#else` guard must stay.**
Chrome-branded builds keep `"Chrome Safe Storage"`; only the non-Chrome branch
is overridden. Removing the guard changes Chrome's keychain namespace too.

**OS3 — `.mm` file: macOS only.** It cannot be compile-checked on this Windows
host. Keep the change syntactically minimal and verify on a mac build.

**OS4 — `components/os_crypt/common/` also has a `keychain_password_mac.h`
upstream that is *not* patched here.** If you need to change the name
programmatically, that's a new patch file, not an edit to this one.

## Workflows

**Changing the keychain namespace**
1. Confirm the product decision (this logs users out of stored credentials).
2. Edit `<chromium_src>/components/os_crypt/common/keychain_password_mac.mm`.
3. Extract back to this path; keep it under `chrome-importer`.
4. Test credential restore on macOS with a profile that predates the change.

**Verifying the current name in a built binary**
- `security find-generic-password -s "BrowserOS Safe Storage"` on a mac host
  that has run the built browser.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/` subtree rules.
- [`common/AGENTS.md`](common/AGENTS.md) — the leaf folder.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — the `chrome-importer` block.
