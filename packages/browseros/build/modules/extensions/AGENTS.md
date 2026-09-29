# `build/modules/extensions/` — bundled CRX extensions from the CDN manifest

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

One module. It fetches a Google Update–protocol XML manifest from a
CDN URL, downloads each extension's `.crx` file, and writes a
`bundled_extensions.json` index into the Chromium tree so pre-installed
extensions are picked up at build time. Two files total.

The extension list is **not** a config file — it comes from the
remote manifest at build time. (The package-level
`AGENTS.md` still references a `config/bundled_extensions.yaml`; that
file does not exist.)

## Contents

| File | What it does |
|---|---|
| `bundled_extensions.py` | `class ExtensionInfo(NamedTuple)` (`id`, `version`, `codebase`) and `class BundledExtensionsModule(CommandModule)` — `produces = ["bundled_extensions"]`, `requires = []`, `description = "Download and bundle extensions from CDN update manifest"`. Private methods `_get_output_dir`, `_fetch_and_parse_manifest`, `_parse_manifest_xml`, `_download_extension`, `_generate_json`. |
| `__init__.py` | Re-exports `BundledExtensionsModule`. |

Output directory: `<chromium_src>/chrome/browser/browseros/bundled_extensions/`.
Manifest URL: `ctx.get_extensions_manifest_url()`.

## Rules

**EXT1 — This is a network module; `validate()` only checks the
Chromium source exists.** Reachability of the manifest is discovered at
execute time, not up front. Do not add an HTTP pre-flight to
`validate()` — it would slow every `--list` and dry run.

**EXT2 — Fail loudly on an empty manifest.** `_fetch_and_parse_manifest()`
returning no extensions raises `RuntimeError("No extensions found in
manifest")`. Do not downgrade that to a warning: a browser shipping with
no bundled extensions is a broken build, not a degraded one.

**EXT3 — `bundled_extensions.json` shape is fixed.**
`{ "<id>": { "external_crx": "<id>.crx", "external_version": "<ver>" } }`,
written with `indent=2` and a trailing newline. The Chromium-side
loader in `chrome/browser/browseros/extensions/` parses this.

**EXT4 — The manifest parser accepts both namespaced and bare XML.**
It tries `.//gupdate:app` first and falls back to `.//app` for
namespaceless manifests, doing the same for `updatecheck`. Keep both
paths when touching the parser.

**EXT5 — Downloads are streamed with a progress bar written to
`sys.stdout`.** The write is a `\r`-prefixed line, not a `log_info`
call, because it updates in place. Do not "clean it up" into the
logger.

**EXT6 — `content-length` may be absent.** When it is, the completion
line omits the size rather than dividing by zero. Preserve that branch.

## Workflows

**Running the module standalone**
1. `browseros build -m bundled_extensions`.
2. Check `<chromium_src>/chrome/browser/browseros/bundled_extensions/`
   for one `.crx` per manifest entry plus the JSON index.

**Adding an extension to the shipped set**
1. Add it to the upstream CDN manifest (this repo has no list to edit).
2. Re-run the module and confirm it appears in `bundled_extensions.json`.

**Debugging a manifest parse failure**
1. `log_info` prints the fetched URL before the request.
2. Confirm the document uses the
   `http://www.google.com/update2/response` namespace and that each
   `<app>` has an `appid` plus an `<updatecheck codebase= version=>`.
3. An entry with a `version` but no `codebase` (or vice versa) is
   silently skipped by `_parse_manifest_xml()`.

**Bumping the artifact contract**
1. Change `_generate_json()` and the Chromium loader together.
2. The JSON key set is a cross-repo contract with
   `packages/browseros/chromium_patches/chrome/browser/browseros/extensions/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../AGENTS.md`](../AGENTS.md) — where this step sits in the pipeline recipes.
- [`../../cli/build.py`](../../cli/build.py) — the `bundled_extensions` registry entry.
- [`../resources/AGENTS.md`](../resources/AGENTS.md) — the adjacent `copy_resources` mechanism.
- [`../../common/context.py`](../../common/context.py) — `get_extensions_manifest_url()`.
