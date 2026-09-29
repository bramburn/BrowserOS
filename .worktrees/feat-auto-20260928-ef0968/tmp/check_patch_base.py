"""Do the BrowserOS patches' base blobs match the tree currently on disk?

Every patch carries `index <old-blob-sha>..<new-blob-sha> 100644`. If the tree
holds <old-blob-sha> for that path, the patch was generated against exactly
this content and `git apply` has a chance. Any mismatch means the tree is NOT
the base the patches were cut from.

Read-only: hashes the working-tree file, never writes.
"""
import os
import re
import subprocess
import sys

SRC = r"D:\browseros-build\src"
ROOT = r"D:\BrowserOs\packages\browseros\chromium_patches"

index_re = re.compile(r"^index ([0-9a-f]{7,40})\.\.([0-9a-f]{7,40})")

patches = []
for dirpath, _d, filenames in os.walk(ROOT):
    for name in filenames:
        if name.endswith((".deleted", ".binary", ".rename")) or name.startswith("."):
            continue
        patches.append(os.path.join(dirpath, name))
patches.sort()

# path -> list of (patch, expected_old_sha)
expected = {}
for p in patches:
    rel = os.path.relpath(p, ROOT).replace("\\", "/")
    cur_path = None
    with open(p, "r", encoding="utf-8", errors="replace") as fh:
        for line in fh:
            if line.startswith("diff --git "):
                m = re.match(r"^diff --git a/(.+?) b/(.+?)\s*$", line.rstrip("\n"))
                cur_path = m.group(2) if m else None
                want = None
            elif line.startswith("index ") and cur_path:
                m = index_re.match(line.rstrip("\n"))
                if m:
                    expected.setdefault(cur_path, []).append((rel, m.group(1)))

missing, mismatch, match, nodiff = [], [], 0, []
for path, entries in sorted(expected.items()):
    full = os.path.join(SRC, path.replace("/", os.sep))
    if not os.path.exists(full):
        missing.append((path, entries[0][0]))
        continue
    if os.path.isdir(full):
        nodiff.append((path, entries[0][0]))
        continue
    if path.endswith((".pdl", ".json", ".xml", ".pma")) or "new file" in "":
        pass
    r = subprocess.run(["git", "hash-object", "--", full],
                       capture_output=True, text=True)
    if r.returncode != 0:
        nodiff.append((path, entries[0][0]))
        continue
    actual = r.stdout.strip()
    for rel, want in entries:
        if actual.startswith(want) or want.startswith(actual):
            match += 1
        else:
            mismatch.append((path, rel, want, actual))

total = match + len(missing) + len(mismatch) + len(nodiff)
print("patch files scanned          : %d" % len(patches))
print("paths with an `index` header : %d" % len(expected))
print("  base blob MATCHES tree     : %d" % match)
print("  base blob MISMATCH         : %d" % len(mismatch))
print("  file absent from tree      : %d" % len(missing))
print("  not hashed (dir/other)     : %d" % len(nodiff))
print("  ---------------------------------")
print("  total index headers        : %d" % total)
print()

if mismatch:
    print("=== MISMATCHED BASE BLOBS (first 25) ===")
    for path, rel, want, actual in mismatch[:25]:
        print("  %s" % path)
        print("      patch  %s" % rel)
        print("      wants  %s" % want)
        print("      has    %s" % actual)
    if len(mismatch) > 25:
        print("  ... and %d more" % (len(mismatch) - 25))
    print()

if missing:
    print("=== PATHS ABSENT FROM TREE (first 25) ===")
    for path, rel in missing[:25]:
        print("  %-70s  <- %s" % (path, rel))
    if len(missing) > 25:
        print("  ... and %d more" % (len(missing) - 25))
