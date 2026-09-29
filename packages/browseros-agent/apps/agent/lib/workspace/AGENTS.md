# `lib/workspace/` — Workspace folders

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The user's recent workspace folders and the currently selected one. The
folders are the agent's working directory for CLI-style harnesses; the
list is an MRU capped at 10 entries, and selection is a single stored
folder object. Adding a *new* folder is a browser API call
(`chrome.browserOS.choosePath`) made by the consuming component in
`components/elements/workspace-selector.tsx`; this folder only stores and
shapes the result.

## Contents

```
workspace/
├── workspace-storage.ts  ← WorkspaceFolder { id, name, path, addedAt };
│                          workspaceFoldersStorage (local:workspaceFolders)
│                          and selectedWorkspaceStorage
│                          (local:selectedWorkspace)
└── use-workspace.ts      ← useWorkspace() → { recentFolders, selectedFolder,
│                           selectFolder, addFolder, removeFolder,
│                           clearSelection }; MRU capped at MAX_RECENT_FOLDERS
│                           (10), de-duplicated by `path` (not `id`),
│                           both storage items watched
```

## Rules

- **WSP1 — File name is `use-workspace.ts` (kebab-case), not
  `useWorkspace.ts`.** Per [`../../../../CLAUDE.md`](../../../../CLAUDE.md),
  multi-word files are kebab-case. Other `lib/` hooks predate the rule
  (`useCredits.ts`, `useVoiceInput.ts`); keep their names, use kebab-case for
  new ones.
- **WSP2 — De-duplication is by `path`, not `id`.** Two entries can share a
  display name; only the path identifies a folder. Keep `filter(f => f.path
  !== folder.path)` in `selectFolder`.
- **WSP3 — The MRU list is capped at 10.** Raising it changes what every
  picker renders; update the constant, not the call site.
- **WSP4 — Browsing for a folder belongs to the component.**
  `workspace-selector.tsx` calls `getBrowserOSAdapter().choosePath()`; this
  folder receives an already-chosen `WorkspaceFolder`. Don't import the
  adapter here.
- **WSP5 — The full-path `choosePath` flow is version-gated** by
  `Feature.WORKSPACE_FOLDER_SUPPORT` (`minBrowserOSVersion 0.36.4.0`) — check
  `useCapabilities()` before offering it.

## Workflows

**Adding a folder to the MRU**
1. `const { addFolder } = useWorkspace()`
2. `addFolder({ id, name, path, addedAt: Date.now() })` — it is promoted to
   the front and the list re-capped.

**Switching the active workspace**
1. `selectFolder(folder)` — persists the selection and re-orders the MRU.
2. `clearSelection()` sets `selectedWorkspaceStorage` back to `null`.

**Showing the current workspace in the chrome**
1. `const { selectedFolder } = useWorkspace()`
2. `components/sidebar/SidebarBranding.tsx` is the existing consumer.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB7).
- [`../../components/elements/AGENTS.md`](../../components/elements/AGENTS.md) — `workspace-selector.tsx`, the picker.
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — `choosePath()` and `Feature.WORKSPACE_FOLDER_SUPPORT`.
- [`../../components/sidebar/AGENTS.md`](../../components/sidebar/AGENTS.md) — `SidebarBranding` consumer.
