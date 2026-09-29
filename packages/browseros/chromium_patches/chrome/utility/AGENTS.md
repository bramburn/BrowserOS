# `chrome/utility/` — utility process root

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/`) in
> [`packages/browseros/`](../../../AGENTS.md).

## What's here

One patch: `utility/BUILD.gn`, which adds the BrowserOS importer source set
to Chromium's `utility` static library. All the actual import code lives
in [`importer/`](importer/AGENTS.md) and
[`importer/browseros/`](importer/browseros/AGENTS.md).

## Contents

```
utility/
├── BUILD.gn              ← deps += [ "//chrome/utility/importer/browseros" ]
│                            inside the importer-enabled branch
└── importer/             ← see importer/AGENTS.md
    └── browseros/        ← see importer/browseros/AGENTS.md
```

## Rules

**UT1 — The importer is compiled in, not feature-gated at runtime.** The
`deps +=` sits inside the same `if` block as the upstream
`importer/*.cc` sources. Removing the block removes the Chrome importer
entirely, not just BrowserOS extras.

**UT2 — Platform decryptors are selected in
`importer/browseros/BUILD.gn`, not here.** `chrome_decryptor_mac.mm` is
added under `is_mac` with `Security.framework`; `chrome_decryptor_win.cc`
under `is_win` with `crypt32.lib`. Linux has no decryptor and
`chrome_decryptor.cc` is a logging stub.

**UT3 — The utility process must stay sandbox-free of Chromium UI deps.**
The `browseros` source set depends on `base`, `sql`, `crypto`, `net`,
`components/user_data_importer/*`, `components/favicon_base`,
`chrome/common/importer`, and `chrome/app:generated_resources` — nothing
from `chrome/browser/ui`.

**UT4 — Chrome import runs in the utility process on purpose.** Passwords,
cookies, and cookies' decryption must not happen in the browser process;
keep the credential-touching code under `importer/browseros/`.

## Workflows

**Adding a new data source to the Chrome importer**
1. Add `chrome_<x>_importer.{cc,h}` under
   [`importer/browseros/`](importer/browseros/AGENTS.md) in
   `namespace browseros_importer`.
2. Add the files to the `sources` list in that directory's `BUILD.gn`.
3. Wire it into `chrome_importer.cc` and the corresponding `Set<Thing>`
   channel in `chrome/common/importer/importer_bridge.h`.
4. Add the checkbox in `chrome/browser/ui/webui/settings/`.

**Changing platform credential decryption**
1. Edit only `importer/browseros/chrome_decryptor_<platform>.*`.
2. Keep `chrome_decryptor.h` platform-agnostic; it exposes the same
   `Decrypt*` entry points for every platform.
3. Re-check the `libs` / `frameworks` lists in that `BUILD.gn`.

## Cross-references

- [`importer/AGENTS.md`](importer/AGENTS.md) — Chromium importer bridge in
  the utility process.
- [`importer/browseros/AGENTS.md`](importer/browseros/AGENTS.md) — the
  BrowserOS Chrome importer itself.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/` root overlay.
- [`../common/importer/AGENTS.md`](../common/importer/AGENTS.md) — the
  browser/utility IPC contract.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `packages/browseros/`.
