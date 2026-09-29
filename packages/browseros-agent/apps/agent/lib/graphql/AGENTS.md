# `lib/graphql/` — GraphQL execution and typed query hooks

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The extension's GraphQL layer against the BrowserOS cloud API
(`${VITE_PUBLIC_BROWSEROS_API}/graphql`). `execute.ts` is the single
POST that runs a `TypedDocumentString`; the three hooks wrap it in TanStack
Query; `QueryProvider.tsx` mounts the `QueryClient` with an IndexedDB
persister so query caches survive a service-worker restart; and
`getQueryKeyFromDocument.ts` derives a stable cache key by parsing the
operation name out of the document.

## Contents

```
graphql/
├── execute.ts                    ← POST with credentials: 'include',
│                                   Accept: application/graphql-response+json;
│                                   throws on !ok, on body.errors, and on
│                                   missing data
├── getQueryKeyFromDocument.ts    ← graphql parse() → OperationDefinition name
├── useGraphqlQuery.ts            ← useQuery, key = [opName] or [opName, vars]
├── useGraphqlMutation.ts         ← useMutation, no key
├── useGraphqlInfiniteQuery.ts    ← useInfiniteQuery + getVariables(pageParam);
│                                   React Query v5 requires initialPageParam
└── QueryProvider.tsx             ← QueryClient (gcTime 24 h) +
                                    PersistQueryClientProvider with
                                    createAsyncStoragePersister over idb-keyval
```

## Rules

- **GQ1 — Every GraphQL request goes through `execute()`** (directly or via
  the hooks). It is the only place that sets `credentials: 'include'`, which
  is what authenticates the request with the session cookie.
- **GQ2 — Documents come from codegen, not template strings.** Write
  ``graphql(`query Name { ... }`)`` from `@/generated/graphql/gql` in a
  `graphql/` subfolder next to the feature, then run `bun run codegen`.
- **GQ3 — Query keys come from the operation name.** Never hand-write a key;
  it must match what `getQueryKeyFromDocument` produces or invalidation
  (`invalidateQueries`) silently misses. The operation must therefore be
  **named** — an anonymous `{ … }` document has a `null` key.
- **GQ4 — `TVariables` is still loosely typed** (`Record<string, any>` with an
  explicit TODO). Don't propagate `any` past the hook boundary.
- **GQ5 — Don't mutate the query client directly.** Use the hooks' returned
  `refetch` / the caller's `useQueryClient().invalidateQueries({ queryKey })`.
- **GQ6 — Persisted cache lives in IndexedDB, not `chrome.storage`.**
  `QueryProvider` is a singleton; mounting it twice creates two clients.

## Workflows

**Adding a query**
1. Add the SDL field to `schema/schema.graphql`, run `bun run codegen`.
2. Create `lib/<feature>/graphql/<name>Document.ts` with a **named** operation.
3. `const { data } = useGraphqlQuery(MyQueryDocument, variables)`.
4. Invalidate with `queryClient.invalidateQueries({ queryKey: ['MyQuery'] })`.

**Adding a paginated list**
1. Use `useGraphqlInfiniteQuery(document, getVariables, { initialPageParam: '' })`.
2. `getVariables` maps the page cursor into the query's `after` argument.

**Running a one-off mutation outside React**
1. `await execute(MyMutationDocument, variables)` from a `lib/<feature>/` module.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB3).
- [`../../schema/AGENTS.md`](../../schema/AGENTS.md) — the SDL source of truth.
- [`../llm-providers/graphql/AGENTS.md`](../llm-providers/graphql/AGENTS.md) — a `graphql/` subfolder example.
- [`../conversations/graphql/AGENTS.md`](../conversations/graphql/AGENTS.md) — the other one.
- [`../env.ts`](../env.ts) — `VITE_PUBLIC_BROWSEROS_API`, the endpoint `execute()` posts to.
