# Branch protection for `main`

GitHub branch/repo security settings can't be expressed as a workflow file —
they're applied in **Settings → Branches** (or **Settings → Rules → Rulesets**)
by someone with admin rights on the repo. Configure a protection rule (or
ruleset) for `main` with:

- **Require a pull request before merging**
  - Require at least 1 approval
  - Dismiss stale approvals when new commits are pushed
- **Require status checks to pass before merging** (enable "Require branches
  to be up to date" too), selecting:
  - `Validate gitflow branch name` (from `branch-name-check.yml`)
  - `Analyze (C++)` (from `codeql.yml`)
  - the `build-and-test` matrix jobs (from `ci.yml`)
  - `Coverage (Linux, GCC)` (from `ci.yml`) — fails below 80% line/branch
    coverage (`gcovr --fail-under-line 80 --fail-under-branch 80`)
  - `Scan dependency diff` (from `dependency-review.yml`) — fails on a
    moderate-or-worse known vulnerability newly introduced by the PR
    (`vcpkg.json` or an Actions dependency)
- **Require conversation resolution before merging**
- **Do not allow bypassing the above settings** (applies rules to admins too)
- **Restrict who can push to matching branches** — only allow merges via PR;
  block direct pushes
- **Block force pushes**
- **Restrict deletions**

Since `main` only receives merges from `release/*`, `hotfix/*`, and
`support/*` branches under this project's gitflow model, you may also want a
rule restricting which branches can open PRs against `main` — GitHub
rulesets support this directly (`main` ruleset → target branch pattern
restrictions), whereas classic branch protection does not; the
`branch-name-check.yml` workflow enforces the naming/pairing convention as a
required status check either way.

`codeql.yml`, `dependency-review.yml`, and `osv-scanner.yml` also run against
PRs targeting `develop` (where `feature/*`/`bugfix/*` work actually lands),
not just `main`. If `develop` gets its own protection rule, consider adding
`Scan dependency diff` as a required check there too — that's where most
vulnerable dependencies would actually be introduced, well before a release
PR ever reaches `main`.

## Other scanners (visible-only)

`osv-scanner.yml`, `zizmor.yml`, and `scorecard.yml` upload SARIF to
**Security → Code scanning** but don't fail PR checks — triage their alerts
there rather than via a required status check (see each workflow's header
comment for why). `scorecard.yml`'s branch-protection sub-check scores more
completely with a fine-grained PAT (read-only, "Administration: read") added
as a repo secret named `SCORECARD_READ_TOKEN`; without it, that one
sub-check just degrades gracefully instead of failing.
