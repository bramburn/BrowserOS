# `components/payments/core/` — payment pref registration

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory. Chromium keeps the payment prefs in
`components/payments/core/payment_prefs.cc`; the patch overlay reproduces that
nesting so the lookup path matches.

## Contents

```
core/
└── payment_prefs.cc   ← kCanMakePaymentEnabled default false
```

## Rules

**PYC1 — Do not create `components/payments/<file>` shortcuts.** The mirrored
path is authoritative.

**PYC2 — `payment_prefs.cc` is the only patched file in Chromium's
`components/payments/`.** Anything else here is a new patch.

## Workflows

**Adding a payment-related patch**
1. Edit the file in `<chromium_src>/components/payments/core/`.
2. Extract to the mirrored path under `chromium_patches/`.
3. Register it in `features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/payments/` rules.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview, rule F2.
