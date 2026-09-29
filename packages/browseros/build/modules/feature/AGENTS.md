# `build/modules/feature/` — `features.yaml` management

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

The read/write/validate layer around `build/features.yaml`, the
manifest that decides which patches ship and how `browseros dev
annotate` groups them into commits. `validation.py` enforces the naming
and description rules, `select.py` handles the interactive prompts
(including classifying unclassified patch files), and `feature.py` has
the CRUD entry points plus four thin `CommandModule` wrappers.

## Contents

| File | What it does |
|---|---|
| `validation.py` | `validate_description(description) -> (ok, msg)`, `validate_feature_name(name) -> (ok, msg)`, `VALID_PREFIXES` (the accepted conventional-commit prefixes for a feature description). |
| `select.py` | `load_features_yaml()`, `save_features_yaml()`, `prompt_feature_selection()`, `prompt_new_feature()`, `add_files_to_feature()`, `get_all_patch_files()`, `get_all_classified_files()`, `get_unclassified_files()`, `classify_files()`, `prompt_feature_selection_for_file()`. |
| `feature.py` | `add_or_update_feature()`, `add_feature()`, `list_features()`, `show_feature()` + `class ListFeaturesModule`, `ShowFeatureModule`, `AddUpdateFeatureModule`, `ClassifyFeaturesModule`. |
| `__init__.py` | Re-exports all of the above. |

All four module classes use `produces = []` and `requires = []`.

## Rules

**FEA1 — `features.yaml` is the source of truth for what ships.** A
patch file under `chromium_patches/` that no feature block lists will
not be applied, will not be committed by `../annotate/`, and will not
be reviewable. Always add the path.

**FEA2 — Descriptions are validated, not free text.** `validate_description()`
requires the conventional-commit prefix to appear in `VALID_PREFIXES`
(`chore:`, and the other accepted prefixes). Write the description as
you want the commit message to read — it is copied verbatim by
`../annotate/annotate.py`.

**FEA3 — Feature names are validated too.** `validate_feature_name()`
rejects characters that would break YAML keys or CLI flags; use
lowercase-hyphenated names like `browseros-core`, `llm-chat`.

**FEA4 — Use `add_or_update_feature()` / `add_files_to_feature()`,
never hand-edit the YAML for routine changes.** These helpers
deduplicate, preserve ordering, and normalise the written file. Manual
edits are how duplicate paths and inconsistent indentation get in.

**FEA5 — `classify_files()` is the fix for orphaned patches.**
`get_unclassified_files()` diffs every patch on disk against every path
already listed in a feature. Run `browseros dev feature classify`
after extracting new patches.

**FEA6 — `load_features_yaml()` / `save_features_yaml()` are the only
serialisers.** Writing the file with a bare `yaml.dump()` elsewhere
loses the layer comments that make the manifest readable.

## Workflows

**Adding a feature**
1. `browseros dev feature add-update --name <feature> --commit <sha> --description "<prefix>: text"`.
2. Confirm the description prefix is in `VALID_PREFIXES`.
3. `browseros dev feature list` and `... show <feature>` to verify.

**Attaching a patch to an existing feature**
1. `browseros dev feature add-update --name <feature> --commit <sha>` —
   it prompts for which files to include.
2. Or use the interactive `browseros dev extract` prompt, which calls
   `add_files_to_feature()` for you.

**Finding patches that belong to no feature**
1. `browseros dev feature classify`.
2. It lists unclassified patch files and prompts for a feature for
   each; `classify_files()` returns `(classified, skipped)`.

**Inspecting the manifest**
1. `browseros dev feature list` — every feature with description.
2. `browseros dev feature show <feature>` — the file list.
3. Or read `build/features.yaml` directly; it is committed and
   hand-editable for bulk reorganisations.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../../features.yaml`](../../features.yaml) — the manifest this module owns.
- [`../annotate/AGENTS.md`](../annotate/AGENTS.md) — the consumer that turns features into commits.
- [`../extract/AGENTS.md`](../extract/AGENTS.md) — where new patches come from.
- [`../../cli/dev.py`](../../cli/dev.py) — the `browseros dev feature` commands.
