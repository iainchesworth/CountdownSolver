# Git workflow

This project follows gitflow. `main` (releases) and `develop` (integration)
are the only long-lived branches. Every other branch MUST be named
`feature/*`, `bugfix/*`, `release/*`, `hotfix/*`, or `support/*` —
`.github/workflows/branch-name-check.yml` enforces this as a required PR
check (feature/bugfix land on `develop`; release/hotfix/support land on
`main`).

**New sessions/worktrees start on an auto-created branch named
`claude/<slug>-<hash>`, which does not match this convention.** Before
making any commit, check the current branch:

```
git branch --show-current
```

If it doesn't match `^(feature|bugfix|release|hotfix|support)/`, rename it
before committing:

```
git branch -m <new-name>
```

This is safe as a plain rename only if the branch has no upstream yet —
check first with `git rev-parse --abbrev-ref --symbolic-full-name @{u}`
(errors if there's no upstream, meaning nothing has been pushed). If it does
have an upstream, ask before renaming rather than renaming and
force-pushing.

Pick the prefix that matches the work: `feature/` for new functionality,
`bugfix/` for fixing broken or inefficient existing behavior, etc.
