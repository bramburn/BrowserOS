# `.claude/skills/dev7-pr/` — stage 7: PR & external review

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

The final stage of the `/dev` pipeline. It pushes the branch, creates a PR
with `gh pr create` using the PRD's Level 1 summary as the body, waits ten
minutes for the automated **Greptile** review, pulls the PR comments, applies
any fixes, pushes again, and writes a wrap-up to
`.llm/<slug>/tmp_done.md` (PR URL, commit count, summary, follow-ups). It is
the pipeline's last hard user-pause, and the only stage that talks to GitHub.

## Contents

```
dev7-pr/
└── SKILL.md    ← 2.2 KB: name, description ("Sub-skill of the /dev workflow."), argument-hint
```

## Rules

**D7R — The PR body is PRD Level 1.** `prd.md`'s executive summary (written in
[`dev3-prd/`](../dev3-prd/AGENTS.md)) is pasted as the body, and must contain
three parts: Summary bullets, one Design paragraph, Test plan. If the PRD
lacks them, fix the PRD, not the PR.

**D7R2 — The Greptile wait is 600 s, then check.** `sleep 600` followed by
`gh pr view --comments` and
`gh api repos/{owner}/{repo}/pulls/{pr_number}/comments`. Fetch both: the
first shows conversation, the second shows line-level review comments.

**D7R3 — PR-round fixes stay narrow.** "Only change what the comment asks
for." Reviewer-driven edits are not a place to pick up deferred work.

**D7R4 — Stage 7 commits as `fix:` again** (`fix: address PR review comments
for <slug>`), matching stage 6's prefix. It only commits if fixes were made;
a clean review produces no extra commit.

**D7R5 — Report the PR URL and stop.** The closing message must include the URL
from `gh pr create`; the feature is then ready for *human* review, not for
further agent automation.

**D7R6 — `tmp_done.md` is the run's closing record.** PR URL, total commits,
what was built, and any tech debt noted during review. It lives in `.llm/`
alongside the rest of the run and is not committed.

## Workflows

**Creating the PR**
1. `/dev7-pr add_user_auth` — reads `prd.md`, `design.md`, `git log --oneline -10`.
2. `git push -u origin HEAD`, then `gh pr create --title "feat: …" --body "<PRD Level 1>"`.
3. Announce the 10-minute Greptile wait, then fetch comments.
4. Apply any comment fixes narrowly, commit, push.
5. Write `.llm/add_user_auth/tmp_done.md` and report the PR URL.

**Human override**
1. The stage pauses before creating the PR — do not open one the user has not
   approved, and do not merge on their behalf when the review lands.

## Cross-references

- [`SKILL.md`](SKILL.md) — the stage definition.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../dev3-prd/AGENTS.md`](../dev3-prd/AGENTS.md) — authors the PR body (Level 1).
- [`../dev6-review-fix/AGENTS.md`](../dev6-review-fix/AGENTS.md) — the usual predecessor.
- [`../dev/SKILL.md`](../dev/SKILL.md) — the pipeline manifest listing stage 7.
- [`../../../.config/AGENTS.md`](../../../.config/AGENTS.md) — Worktrunk, which
  creates the feature branch this stage pushes.
