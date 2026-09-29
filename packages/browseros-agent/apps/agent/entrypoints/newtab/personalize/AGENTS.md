# `entrypoints/newtab/personalize/` — personalise the assistant (`/home/personalize`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/newtab`.

## What's here

A small, deliberate form: three free-text sections — "About you", "What you expect
from the browser", and "Your commonly performed actions" — that write to
`usePersonalization()` in `@/lib/personalization/personalizationStorage`. Each
section ships a Markdown template and a worked example; the template can be copied
to the clipboard with a one-click button.

The page is only mounted in the alpha build of the home route (and the bare
`/personalize` hash redirects to it when alpha is on, or to `/home` when it is off).

## Contents

```
personalize/
├── Personalize.tsx   ← the three collapsible sections + clipboard copy
└── templates.ts      ← template/example Markdown per section key
```

## Rules

- **PE1 — Section keys must match `PersonalizationStorage`.** `aboutYou`,
  `expectations`, `commonActions` are storage keys; renaming one silently drops the
  user's saved text.
- **PE2 — Add a section in two files:** an entry in the `sections` array in
  `Personalize.tsx` **and** a matching block in `templates.ts`. A section without a
  template renders without the copy affordance.
- **PE3 — The editor is `MarkdownEditor` from `@/components/ui/MarkdownEditor`,**
  not a plain textarea. Content is stored as Markdown; preview rendering depends on
  that.
- **PE4 — Copy feedback is a 2 s timeout** (`setCopiedSection`, cleared after 2000 ms).
  Reuse the existing pattern for a new copy button.
- **PE5 — Entry animation is gated on `mounted`,** set in a mount effect. Keep it —
  it is what avoids a flash of unstyled layout in a full-page route.
- **PE6 — Persist through `usePersonalization`,** never `chrome.storage` directly.

## Workflows

**Adding a personalisation section**
1. Add the key to the `PersonalizationStorage` type in
   `@/lib/personalization/personalizationStorage.ts`.
2. Add a template + example entry in `templates.ts` under the same key.
3. Add the `{ key, title, description }` entry to the `sections` array in
   `Personalize.tsx`.
4. Render the editor using the same Collapsible + MarkdownEditor pattern.

**Editing a template**
1. Edit only `templates.ts` — the copy button interpolates it.
2. Keep the `[Placeholder]` bracket convention in templates and plain prose in
   examples; users can tell them apart.

**Debugging lost personalisation**
1. Check the storage key names match between this folder and the lib module.
2. Check `mounted` is flipping (a stuck false means invisible content).
3. Check the route is actually mounted — `/home/personalize` exists only in the alpha
   home branch.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the new-tab route rules.
- [`../../app/AGENTS.md`](../../app/AGENTS.md) — the route and its alpha gate.
- [`../../../../lib/personalization/personalizationStorage.ts`](../../../lib/personalization/personalizationStorage.ts) — storage and hook.
- [`../../../../components/ui/MarkdownEditor.tsx`](../../../components/ui/MarkdownEditor.tsx) — the editor used here.
