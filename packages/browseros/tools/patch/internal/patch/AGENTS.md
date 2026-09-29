# `tools/patch/internal/patch/` — the patch-set model

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`tools/patch/internal/`](../AGENTS.md).

## What's here

The data model at the centre of `browseros-patch`: what a patch *is*, how a
git diff becomes a `FilePatch`, how a directory of patch files is read and
written, how a checkout's working tree becomes a comparable `PatchSet`, and
how two patch sets are compared.

Five files, one model, and the convention that makes the whole tool work:
a patch file's path in `chromium_patches/` **is** its path in the Chromium
checkout, minus a suffix that encodes the operation.

## Contents

| File | Contents |
|---|---|
| `types.go` | `FileOp` (`ADD`/`MODIFY`/`DELETE`/`RENAME`/`COPY`/`BINARY`), `FilePatch`, `PatchSet`, `DeltaKind` (`needs_apply`/`needs_update`/`up_to_date`/`orphaned`), `Delta`, `NormalizeChromiumPath`, `PathMatches`, `IsInternalPath` |
| `parser.go` | `ParseDiffOutput` — `diff --git` header plus `new file mode` / `deleted file mode` / `rename from` / `similarity index` / `Binary files` detection |
| `repo.go` | `LoadRepoPatchSet` (walk `chromium_patches/`), `WriteRepoPatchSet` (write back), `loadPatchFile`, `patchWriteTarget`, `removePatchVariants`, `parseRenameMarker` |
| `workspace.go` | `BuildWorkingTreePatchSet`, `BuildCommitPatchSet`, `BuildRangePatchSet`, `buildBaseScopedSet`, `filterSet`, `ScopeFromSet`, `RejectPath`, `syntheticAddPatch` |
| `compare.go` | `Compare` and `signature` |

## Contents in detail — the four file-name variants

A patch file under `chromium_patches/` is named after the Chromium path it
touches, with one optional suffix that replaces the diff body:

| Suffix | Meaning | Body |
|---|---|---|
| *(none)* | a text diff | the `git diff` output |
| `.deleted` | delete the file | the literal text `File deleted in patch` |
| `.binary` | binary file, not directly applicable | the literal text `Binary file` |
| `.rename` | pure rename | `Renamed from: <old>` + `Similarity: <n>%` |

`chromium_patches/chrome/browser/browser.cc` therefore means "this is the
diff for that file", and `removePatchVariants` deletes all four variants
before writing so a file never lingers in two forms.

## Rules

**PT1 — The repo path equals the Chromium path.** `LoadRepoPatchSet` keys
each entry by its path relative to `chromium_patches/`, and `engine` matches
those keys directly against paths in the checkout. Renaming or nesting a
patch file breaks the correspondence with no error.

**PT2 — Always normalise with `NormalizeChromiumPath`.** `filepath.ToSlash`
+ `path.Clean` + strip a leading `./`. Without it, a Windows checkout
produces `chrome\browser\...` keys that match nothing (IN5 in the parent).

**PT3 — `PathMatches` also filters out `.browseros-patch/`.** It calls
`IsInternalPath` first, so the tool's own state directory can never appear
in a filter result. Do not add a name check for it elsewhere.

**PT4 — `WriteRepoPatchSet` scope decides what gets deleted.** With an empty
`scope`, every path already present *and* every path in the incoming set is
in scope, so a missing file is deleted from the repo. With a non-empty
`scope`, only those paths are touched. Getting the scope wrong in a new
caller silently deletes patch files.

**PT5 — A written patch body always ends in `\n`.** `WriteRepoPatchSet`
appends one if missing. Hand-rolled writers that skip this produce a
trailing-newline diff on the next `git status`.

**PT6 — `Compare` is symmetric and sorted.** It reports every repo path
(`NeedsApply` when absent locally, `UpToDate` or `NeedsUpdate` when present)
and then every local-only path as `Orphaned`, sorted by path. Callers rely
on the sort for stable `--json` output.

**PT7 — `signature` normalises CRLF, `index ` lines and trailing
whitespace.** That is what makes a Windows-extracted patch compare equal to
a Linux one. Do not add a comparison that reads raw bytes.

**PT8 — Binary markers are load-bearing but not applicable.** A `.binary`
file parses to `OpBinary` with empty content, and `engine.applySingleOperation`
returns an explicit error for it. Do not "support" it by writing a fake
diff body.

## Workflows

**Adding a new patch-file variant (e.g. a symlink marker)**
1. Add the suffix to `loadPatchFile` (detect) and `patchWriteTarget` (write).
2. Add it to `removePatchVariants` so the old form is cleaned up.
3. Add a case to `Compare.signature` so two equivalent markers compare equal.
4. Cover it in `patch_test.go`.

**Extracting only part of a checkout**
1. Pass the paths after `--`; `cmd`'s `splitWorkspaceAndFilters` turns them
   into `Filters`.
2. `PathMatches` treats a filter as a prefix match on directory boundaries,
   so `chrome/browser` matches everything beneath it and nothing else.
3. In `working-tree` mode the filters double as the write scope, so only
   those paths are written or deleted in `chromium_patches/`.

**Debugging "empty patch file"**
1. `loadPatchFile` errors when a suffixed-less file parses to zero
   `FilePatch`s — usually a diff with no `diff --git` header (a raw unified
   diff, or an empty file).
2. `syntheticAddPatch` is what `buildBaseScopedSet` produces for an
   `A`-status change when a base is given; a hand-written equivalent will
   differ in the `index` line and compare as `NeedsUpdate`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — layering (IN5, IN6).
- [`../engine/AGENTS.md`](../engine/AGENTS.md) — the main consumer of
  `Compare` and `ScopeFromSet`.
- [`../git/AGENTS.md`](../git/AGENTS.md) — the diff invocations parsed here.
- [`../../../../chromium_patches/AGENTS.md`](../../../../chromium_patches/AGENTS.md) —
  the tree whose on-disk layout this package defines.
