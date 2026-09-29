# `components/os_crypt/common/` — "BrowserOS Safe Storage"

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A one-hunk unified diff against `keychain_password_mac.mm`. Inside the
`#else` branch of `BUILDFLAG(IS_CHROME_BRANDING)`, the macOS keychain
constants are renamed:

| | Chromium default | BrowserOS |
|---|---|---|
| `kDefaultServiceName` | `"Chromium Safe Storage"` | `"BrowserOS Safe Storage"` |
| `kDefaultAccountName` | `"Chromium"` | `"BrowserOS"` |

Feature block: **`chrome-importer`**.

## Contents

```
common/
└── keychain_password_mac.mm   ← @@ -38,8 +38,9 @@ : two constants + one comment
```

## Rules

**OSC1 — `kDefaultServiceName` and `kDefaultAccountName` change together.**
A partial change produces a keychain namespace that matches nothing at all —
strictly worse than leaving both alone.

**OSC2 — Keep the `// BrowserOS:` marker comment.** It is the only signal in
this file that the hunk is a deliberate fork rather than upstream drift, and
`grep`-based audits use it.

**OSC3 — Never touch the `#if`/`#else` structure.** The Chrome-branded branch
above the hunk must be byte-identical to upstream.

**OSC4 — Credential impact is silent.** After this change, an existing
Chromium-branded install's keychain items are not migrated; users see the OS
password prompt again. Any PR touching this file should say so.

**OSC5 — `.mm` = Objective-C++, macOS build only.** No compile coverage on the
Windows dev host. Keep edits to string literals where possible.

## Workflows

**Reverting to the Chromium keychain name**
1. Edit `<chromium_src>/components/os_crypt/common/keychain_password_mac.mm`.
2. Restore both constants to their Chromium values, drop the `// BrowserOS:`
   comment.
3. Extract back to this path.

**Adding a third branded variant (e.g. a beta channel)**
1. Add a new `const char[]` pair next to the existing ones rather than
   parameterising the existing ones.
2. Extend the `#if` structure; do not add a runtime flag.
3. Document the credential-migration consequence.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/os_crypt/` rules.
- [`../../AGENTS.md`](../../AGENTS.md) — `components/` subtree rules.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview.
