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
