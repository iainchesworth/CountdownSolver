# CI / dependency provisioning

See [CI & dependency provisioning](https://iainchesworth.github.io/CountdownSolver/ci/)
in the published docs (source: [`docs/ci.md`](../docs/ci.md)) for the full
write-up on how Qt is provisioned here and in CI, and which
`CMAKE_PREFIX_PATH` to pass per platform.

Quick reference:

```powershell
# Windows (PowerShell)
pwsh ci/install-qt.ps1                 # installs to C:\Qt by default
```

```bash
# Linux / macOS
./ci/install-qt.sh                     # installs to ~/Qt by default
```
