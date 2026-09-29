# `components/payments/` — web payment API off by default

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A one-subdirectory branch carrying a single pref-default flip. Chromium
registers `payments::kCanMakePaymentEnabled` as `true`; the BrowserOS patch
registers it as `false`, so the Web Payment API is opt-in rather than opt-out.

Feature block: **`chromium-ui-fixes`**.

## Contents

```
payments/
└── core/
    └── payment_prefs.cc   ← kCanMakePaymentEnabled true → false
```

## Rules

**PY1 — One line, real consequence.** The Payment Request API stops being
offered to sites until a user (or the `chrome://settings` payments toggle)
enables it. Any site depending on it degrades to its fallback checkout path.

**PY2 — Leave `kPaymentsFirstTransactionCompleted` alone.** It is the first
registration in the same hunk; only the second one changes.

**PY3 — Keep the `SYNCABLE_PREF` flag** so an explicit user opt-in syncs.

**PY4 — Related but separate: the `browserOs.choosePath` extension API.** That
lives in `chrome/common/extensions/api/browser_os.idl` and its handler under
`chrome/browser/extensions/api/browser_os/`. This pref is not related to it.

## Workflows

**Re-enabling the payment API by default**
1. Edit `<chromium_src>/components/payments/core/payment_prefs.cc`.
2. Flip the second `RegisterBooleanPref` default back to `true`.
3. Extract back to this path; keep it under `chromium-ui-fixes`.

**Verifying**
1. Fresh profile; visit a checkout using Payment Request.
2. Expect the fallback form, not the native sheet.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/` subtree rules.
- [`core/AGENTS.md`](core/AGENTS.md) — the leaf folder.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
