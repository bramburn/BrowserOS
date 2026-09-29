# `lib/onboarding/` — First-run state and profile sync

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The persisted state of the first-run experience: whether onboarding was
completed, the profile the user typed in (name / role / company / free-text
description), the dismissal timestamps for the browser-import and sign-in
hints, the post-login redirect path, and the first-run confetti flag.
`syncOnboardingProfile.ts` takes that locally-typed profile and pushes it
to the cloud the first time the user is signed in, then clears it locally
so the next launch does not repeat the write.

## Contents

```
onboarding/
├── onboardingStorage.ts    ← OnboardingProfile { name, role, company,
│                             description? }; six `local:` items:
│                             onboardingCompleted, onboardingProfile,
│                             importHintDismissedAt, signInHintDismissedAt,
│                             authRedirectPath, firstRunConfettiShown
└── syncOnboardingProfile.ts
                             ← syncOnboardingProfile(userId): splits the
                             display name into first/last, maps role /
                             company / description into a `preferences` map,
                             runs UpdateProfileByUserIdDocument, then
                             removes the stored profile
```

## Rules

- **ONB1 — Split the name, don't overwrite it.** `splitName()` treats
  everything after the first token as the last name; multi-word surnames
  work, but a mononym is stored with an empty `lastName` (which is then
  omitted from the patch).
- **ONB2 — Extra fields go into `preferences`.** `role`, `company` and
  `description` are not first-class profile columns; they are string values
  in a `preferences` record. Adding a real column means changing the
  document in `entrypoints/app/profile/graphql/profileDocument.ts`.
- **ONB3 — The local profile is write-once.** `syncOnboardingProfile`
  removes it after a successful sync so the user's later edits aren't
  overwritten by the stale onboarding answer.
- **ONB4 — Dismissal hints store timestamps, not booleans.** A `number |
  null` "dismissed at" lets a hint return after N days; don't convert them
  to booleans.
- **ONB5 — `authRedirectPath` is a router path string** consumed by the login
  route. Store the path, not a route object.

## Workflows

**Adding a first-run hint**
1. Add `xxxHintDismissedAtStorage = storage.defineItem<number | null>(...)`
   in `onboardingStorage.ts`.
2. Gate rendering on `value === null`; write `Date.now()` on dismissal.
3. Re-show by clearing the value.

**Changing what onboarding collects**
1. Extend `OnboardingProfile` in `onboardingStorage.ts`.
2. Add the form fields in `entrypoints/onboarding/steps/`.
3. Map the new field in `syncOnboardingProfile.ts` (into `preferences` unless
   it's a real profile column).

**Running the profile sync after login**
1. `await syncOnboardingProfile(user.id)` once a session exists.
2. It no-ops when no profile is stored.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB3).
- [`../auth/AGENTS.md`](../auth/AGENTS.md) — supplies the `userId`.
- [`../graphql/AGENTS.md`](../graphql/AGENTS.md) — `execute()` used by the sync.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the `onboarding/` entry point and the profile document.
