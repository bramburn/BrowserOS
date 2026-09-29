"""Static ownership check for packages/browseros/build/features.yaml.

Reports any Chromium file claimed by more than one feature block, including
the case where one block claims a whole directory and another claims a file
inside it.
"""
import sys
from collections import defaultdict

import yaml

PATH = r"D:\BrowserOs\packages\browseros\build\features.yaml"

with open(PATH, "rb") as fh:
    data = yaml.safe_load(fh)

features = data["features"]

exact = defaultdict(list)   # path -> [block]
dirs = defaultdict(list)    # dir  -> [block]

for block, spec in features.items():
    for entry in spec.get("files") or []:
        p = entry.rstrip("/")
        if entry.endswith("/"):
            dirs[p].append(block)
        else:
            exact[p].append(block)

print("blocks: %d" % len(features))
print("file claims: %d   dir claims: %d" % (sum(len(v) for v in exact.values()),
                                            sum(len(v) for v in dirs.values())))
print()

dupes = {p: b for p, b in exact.items() if len(b) > 1}
print("=== EXACT DUPLICATE FILE CLAIMS: %d ===" % len(dupes))
for p, b in sorted(dupes.items()):
    print("  %s" % p)
    for blk in b:
        print("      <- %s" % blk)
print()

ddupes = {p: b for p, b in dirs.items() if len(b) > 1}
print("=== DUPLICATE DIRECTORY CLAIMS: %d ===" % len(ddupes))
for p, b in sorted(ddupes.items()):
    print("  %s   <- %s" % (p, ", ".join(b)))
print()

print("=== DIR CLAIM vs NESTED FILE CLAIM ===")
hits = 0
for d, dblocks in sorted(dirs.items()):
    prefix = d + "/"
    for p, pblocks in exact.items():
        if p.startswith(prefix):
            hits += 1
            print("  dir  %s  <- %s" % (d, ", ".join(dblocks)))
            print("  file %s  <- %s" % (p, ", ".join(pblocks)))
            print()
print("nested overlaps: %d" % hits)
