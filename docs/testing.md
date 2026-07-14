# Testing & coverage

Testing is mandatory and drives development (TDD): new behaviour gets a
test alongside it, not after the fact.

- **Unit tests** ([`tests/unit`](https://github.com/iainchesworth/CountdownSolver/tree/develop/tests/unit))
  cover each behaviour in isolation, including the Numbers, Letters, and
  Conundrum game logic in `countdown::solver`.
- **Integration tests** ([`tests/integration`](https://github.com/iainchesworth/CountdownSolver/tree/develop/tests/integration))
  exercise the file-backed dictionary and full solve pipelines together.
- **App tests** ([`tests/app`](https://github.com/iainchesworth/CountdownSolver/tree/develop/tests/app))
  exercise the Qt-facing `Solver` facade with Qt Test.
- **QML tests** ([`tests/qml`](https://github.com/iainchesworth/CountdownSolver/tree/develop/tests/qml))
  drive the real `Countdown` QML module (Numbers/Letters/Conundrum/Settings
  pages) headlessly with Qt Quick Test, exercising the QML → `solver` →
  library round trip end to end.

The Catch2 suites are registered with CTest via `catch_discover_tests`, so
each `TEST_CASE` is an individually runnable CTest; the Qt Test/Quick Test
suites each register as a single CTest entry covering every case in that
binary.

```sh
ctest --preset windows-msvc-debug
```

## Coverage

A dedicated `linux-gcc-coverage` preset builds everything with
`--coverage` (gcov) instrumentation:

```console
$ cmake --preset linux-gcc-coverage -DCMAKE_PREFIX_PATH=<qt>/6.8.3/gcc_64
$ cmake --build --preset linux-gcc-coverage
$ QT_QPA_PLATFORM=offscreen ctest --preset linux-gcc-coverage
$ gcovr --root . --filter 'src/.*' --exclude 'src/app/main\.cpp' \
    --exclude-throw-branches --exclude-unreachable-branches \
    --gcov-ignore-errors=no_working_dir_found \
    --object-directory build/linux-gcc-coverage --print-summary
```

If your default `gcov` doesn't match the compiler that built the coverage
data (e.g. building with `g++-15` but `gcov` resolves to an older
default), add `--gcov-executable gcov-<N>` — a version mismatch doesn't
error, it just silently reports 0% coverage.

CI runs this on every push/PR (`Coverage (Linux, GCC)` in `ci.yml`,
pinning `--gcov-executable gcov-15` to match the `g++-15` it builds with)
and **fails the job below 80% line or branch coverage**.
