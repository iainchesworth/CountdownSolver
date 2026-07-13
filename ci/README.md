# CI / dependency provisioning

Qt is **not** built from source (it is huge, and Qt's deeply-nested build tree
blows past Windows' 260-char `MAX_PATH` under a normal project path). Instead we
use the **official prebuilt Qt binaries** via
[`aqtinstall`](https://github.com/miurahr/aqtinstall):

- **CI** uses `jurplel/install-qt-action` (a wrapper around aqtinstall) — see
  [`.github/workflows/ci.yml`](../.github/workflows/ci.yml).
- **Locally**, run the matching script below; both install the same pinned Qt
  and print the `CMAKE_PREFIX_PATH` to pass to CMake.

vcpkg still provides Catch2 (the `tests` feature); it no longer provides Qt.

## Pinned version

`QT_VERSION = 6.8.3` (keep this in sync across the scripts here and the CI
workflow's `install-qt-action` `version:` input).

## Local install

```powershell
# Windows (PowerShell)
pwsh ci/install-qt.ps1                 # installs to C:\Qt by default
```

```bash
# Linux / macOS
./ci/install-qt.sh                     # installs to ~/Qt by default
```

Then configure the app build against it, e.g. on Windows:

```powershell
cmake --preset windows-msvc -DCOUNTDOWN_BUILD_APP=ON `
  -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build --preset windows-msvc
```

`CMAKE_PREFIX_PATH` per platform:

| Platform | Path |
| --- | --- |
| Windows (MSVC) | `<outdir>/6.8.3/msvc2022_64` |
| Linux (GCC)    | `<outdir>/6.8.3/gcc_64` |
| macOS          | `<outdir>/6.8.3/macos` |
