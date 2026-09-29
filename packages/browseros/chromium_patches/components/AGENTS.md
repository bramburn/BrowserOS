# `components/` — pref defaults, branding assets, platform shims

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../chromium_patches/AGENTS.md`](../../chromium_patches/AGENTS.md).

## What's here

Nine Chromium `components/` subtrees, 13 patch files total. They split into
three unrelated concerns that happen to share a top-level directory in
Chromium:

- **Default-value changes** (`bookmarks/`, `content_settings/`, `payments/`,
  `search/`) — flip a shipped pref or `base::Feature` default away from
  Chromium's choice. All four belong to the `chromium-ui-fixes` feature.
- **New enum members** (`infobars/`, `user_data_importer/`) — extend an enum
  that must stay in lockstep with a `tools/metrics/histograms/metadata/` mirror
  or a mojom.
- **Branding + platform shims** (`os_crypt/`, `remote_cocoa/`, `vector_icons/`)
  — keychain naming, the macOS headless-window flag, and the two BrowserOS
  toolbar icons.

## Contents

```
components/
├── bookmarks/browser/bookmark_utils.cc        ← bookmark bar ON by default
├── content_settings/core/browser/cookie_settings.cc ← block 3rd-party cookies by default
├── infobars/core/infobar_delegate.h           ← +BROWSEROS_AGENT_INSTALLING_INFOBAR_DELEGATE = 135
├── os_crypt/common/keychain_password_mac.mm   ← "BrowserOS Safe Storage" / "BrowserOS"
├── payments/core/payment_prefs.cc             ← kCanMakePaymentEnabled default false
├── remote_cocoa/
│   ├── app_shim/native_widget_ns_window_bridge.mm ← honour is_headless
│   └── common/native_widget_ns_window.mojom        ← +bool is_headless
├── search/ntp_features.cc                     ← kNtpFooter off by default
├── user_data_importer/common/                 ← TYPE_CHROME, EXTENSIONS import bit
│   ├── importer_data_types.h
│   └── importer_type.h
└── vector_icons/
    ├── BUILD.gn                    ← registers the two new .icon files
    ├── chat_orange.icon            ← new-file diff, 48 lines
    └── clash_of_gpts.icon          ← new-file diff
```

Feature ownership in `features.yaml`: `chromium-ui-fixes`
(`bookmarks/`, `content_settings/`, `payments/`, `search/`), `agent-v2-infobar`
(`infobars/`), `chrome-importer` (`user_data_importer/`), `branding`
(`vector_icons/`). `os_crypt/` is `chrome-importer`; the two `remote_cocoa/`
files are **not listed in `features.yaml` at all** — see CO8.

## Rules

**CO1 — These are one-line flips with wide user impact.** Four of the thirteen
files change a single default value. Reverting one silently re-enables the
bookmark bar, third-party cookies, the NTP footer, or the web payment API for
every new profile. They look trivial; they are product decisions.

**CO2 — Enum values are append-only.** `InfoBarIdentifier` gains `= 135`;
`ImportItem` gains `EXTENSIONS = 1 << 7` and moves `ALL` from `(1 << 7) - 1` to
`(1 << 8) - 1`. `ImporterType` gains `TYPE_CHROME = 7`. Never renumber or
reuse a slot — these persist in profiles and telemetry.

**CO3 — `ALL` in `importer_data_types.h` must be re-derived when you add a
bit.** Forgetting the `ALL = (1 << 8) - 1` update means "import everything"
silently skips the new category.

**CO4 — Every `enums.xml` mirror is mandatory.** `infobars/core/infobar_delegate.h`
carries `LINT.ThenChange(//tools/metrics/histograms/metadata/browser/enums.xml:InfoBarIdentifier)`.
Ship the C++ change and
`chromium_patches/tools/metrics/histograms/metadata/browser/enums.xml` together
or PRESUBMIT fails.

**CO5 — `.icon` files here are diffs, not SVG/XML.** `chat_orange.icon` and
`clash_of_gpts.icon` are `new file mode` unified diffs containing Skia's
tangent-command format. A new icon must also be added to the `sources` list in
`components/vector_icons/BUILD.gn` or it is never compiled into the binary.

**CO6 — `os_crypt` names are user-visible and load-bearing.** Changing
`kDefaultServiceName` from `"Chromium Safe Storage"` to `"BrowserOS Safe
Storage"` orphans every existing keychain entry — users silently lose saved
passwords. It is guarded by `#else` of `BUILDFLAG(IS_CHROME_BRANDING)`, so
Chrome-branded builds keep the Chrome string. Do not remove that guard.

**CO7 — `remote_cocoa/` is macOS-only ObjC + mojom.** It cannot be compile-checked
on this Windows host. The mojom field (`is_headless`) and the bridge that reads
it must land in the same change; a mojom field with no reader is dead weight,
a reader for a non-existent field is a hard build break on mac.

**CO8 — The two `remote_cocoa/` files are unregistered in `features.yaml`.**
They still apply (every patch file is applied by path) but they will not appear
in an annotated commit. If you touch them, add them to a feature block.

## Workflows

**Flipping a shipped default**
1. Find the registration in `<chromium_src>/components/**` (e.g.
   `RegisterProfilePrefs`).
2. Change the default argument, leaving the `SYNCABLE_PREF` annotation alone.
3. Extract back to the mirrored `chromium_patches/components/**` path.
4. Keep it under `chromium-ui-fixes` unless it is branding.

**Adding a new import category**
1. Add the bit to `ImportItem` in `components/user_data_importer/common/importer_data_types.h`.
2. Re-derive `ALL`.
3. Add the `VisitSource` entry for the new source browser.
4. Add `TYPE_<BROWSER>` to `components/user_data_importer/common/importer_type.h`.
5. Wire the UI in `chrome/browser/importer/importer_list.cc` and the
   implementation in `chrome/utility/importer/browseros/` (both `chrome/`, out
   of this folder).

**Adding a toolbar icon**
1. Create the `.icon` at
   `components/vector_icons/<name>.icon` in the Chromium tree.
2. Extract → produces a `new file mode` diff.
3. Register it alphabetically in the `sources` list of
   `components/vector_icons/BUILD.gn`.
4. Add both paths to the `branding` feature in `features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — overlay conventions.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview, rule F2.
- [`../../build/features.yaml`](../../build/features.yaml) — feature ownership for every file here.
- [`../tools/metrics/AGENTS.md`](../tools/metrics/AGENTS.md) — the `enums.xml` mirrors.
- [`../ui/AGENTS.md`](../ui/AGENTS.md) — the `headless` init param these platform shims consume.
