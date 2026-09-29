"""Confirm the 147 'absent' paths are new-file patches (expected) and list the
one genuine base mismatch."""
import os
import re

SRC = r"D:\browseros-build\src"
ROOT = r"D:\BrowserOs\packages\browseros\chromium_patches"

patches = []
for dirpath, _d, filenames in os.walk(ROOT):
    for name in filenames:
        if name.endswith((".deleted", ".binary", ".rename")) or name.startswith("."):
            continue
        patches.append(os.path.join(dirpath, name))
patches.sort()
print("patch files (os.walk, same filters as find_patch_files): %d" % len(patches))
print("  of which AGENTS.md            : %d" % sum(
    1 for p in patches if os.path.basename(p) == "AGENTS.md"))
print("  of which look like real diffs : %d" % sum(
    1 for p in patches if os.path.basename(p) != "AGENTS.md"))
print()

newfile, notnew, missing = 0, [], []
for p in patches:
    rel = os.path.relpath(p, ROOT).replace("\\", "/")
    if os.path.basename(p) == "AGENTS.md":
        continue
    txt = open(p, encoding="utf-8", errors="replace").read()
    paths = re.findall(r"^diff --git a/(.+?) b/(.+?)\s*$", txt, re.M)
    for _a, b in paths:
        full = os.path.join(SRC, b.replace("/", os.sep))
        if os.path.exists(full):
            continue
        if "new file mode" in txt:
            newfile += 1
        else:
            notnew.append((b, rel))
print("absent-from-tree diffs that ARE 'new file mode' : %d" % newfile)
print("absent-from-tree diffs that are NOT new files  : %d" % len(notnew))
for b, rel in notnew[:40]:
    print("   %-60s <- %s" % (b, rel))
