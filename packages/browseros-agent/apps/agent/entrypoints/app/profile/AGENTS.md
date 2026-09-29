# `entrypoints/app/profile/` — profile page (`/profile`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

A single page for the signed-in user's first name, last name and avatar. It is
rendered inside `AuthLayout` (no sidebar) and is reachable directly from the hash
route `#/profile`. The form is react-hook-form + zod; reads and writes go through
`useGraphqlQuery` / `useGraphqlMutation` against the remote backend.

Avatar upload is a two-step flow: pick a local file, preview it, then upload and
store the resulting URL.

## Contents

```
profile/
├── ProfilePage.tsx      ← form state, avatar preview/upload, ProfileState machine
└── graphql/
    └── profileDocument.ts ← GetProfileByUserId + UpdateProfileByUserId
```

## Rules

- **PF1 — The user id comes from the session, not the URL.**
  `useSessionInfo()` from `@/lib/auth/sessionStorage`; `userId` may be `undefined`
  while signed out, and the page renders the signed-out state from that.
- **PF2 — Validation is zod, enforced through `zodResolver`.** `formSchema` requires
  both names. Add fields to `formSchema` and `FormValues` together; the type is
  inferred from the schema.
- **PF3 — Cache invalidation uses `getQueryKeyFromDocument`.** Never hand-build a
  query key in this page.
- **PF4 — The GraphQL patch shape is server-defined.** `UpdateProfileByUserId` takes
  `ProfilePatch!`; add fields in `apps/agent/schema/schema.graphql` before selecting
  or sending them.
- **PF5 — Keep the `ProfileState` union honest.** It is
  `'idle' | 'loading' | 'success' | 'error'`; the error string is stored separately in
  `error`. Do not collapse them.
- **PF6 — Avatar uploads show a local preview before upload.** Clearing or replacing
  the preview must not leave a stale remote URL in the form.

## Workflows

**Adding a profile field**
1. Add the field to `ProfilePatch` in `apps/agent/schema/schema.graphql`.
2. Add it to the query selection and to `updateProfileByUserId`'s patch in
   `graphql/profileDocument.ts`.
3. Run `bun run codegen` from `packages/browseros-agent`.
4. Extend `formSchema` in `ProfilePage.tsx`, add the `FormField`, and widen
   `ProfileState` handling if the save can fail differently.
5. Invalidate the query via `getQueryKeyFromDocument(GetProfileByUserIdDocument)`.

**Debugging a save that appears to do nothing**
1. Confirm a session exists (`useSessionInfo()`).
2. Check the mutation variables match `ProfilePatch` after codegen.
3. Verify the query key invalidation actually targets the list key used by the page.

**Verifying visually**
1. `bun scripts/dev/inspect-ui.ts open-app`
2. Navigate to `app.html#/profile` and `snapshot`/`fill`/`click` the form fields.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table.
- [`./graphql/AGENTS.md`](./graphql/AGENTS.md) — documents in this folder.
- [`../layout/AGENTS.md`](../layout/AGENTS.md) — `AuthLayout`, which wraps `/profile`.
- [`../../../../lib/auth/sessionStorage.ts`](../../../lib/auth/sessionStorage.ts) — `useSessionInfo`.
- [`../../../../lib/graphql/`](../../../lib/graphql/) — query/mutation hooks and key derivation.
