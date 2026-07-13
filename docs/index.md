# Countdown Solver

A cross-platform desktop app that solves the two puzzle games from the UK TV
show **[Countdown](https://en.wikipedia.org/wiki/Countdown_(game_show))** —
handy at home, or for double-checking a contestant's working:

- **Numbers** — pick six numbers and a target between 100 and 999, and the
  solver finds an arithmetic expression (`+ - x /`, each number used at most
  once, no fractions or negative numbers along the way) that hits the target
  exactly, or gets as close as possible.
- **Letters** — pick nine letters, and the solver finds every valid word
  hiding in them, longest first.

There's a third game on the show, the **Conundrum** (unscramble nine letters
into one word), which the app can generate and solve too.

![Numbers page](assets/screenshots/numbers.png)

## Try it

Grab a build from the project's
[releases](https://github.com/iainchesworth/CountdownSolver/releases), or
build it yourself — see [Getting started](getting-started.md).

## A tour of the app

=== "Letters"

    ![Letters page](assets/screenshots/letters.png)

=== "Conundrum"

    ![Conundrum page](assets/screenshots/conundrum.png)

=== "Settings"

    ![Settings page](assets/screenshots/settings.png)

## Under the hood

All the game logic lives in a small, dependency-free C++23 library,
`countdown::solver`, with no GUI or platform code in sight — it's unit
tested on its own and could be dropped into a different frontend entirely.
A thin Qt 6 Quick (QML) app sits on top for the GUI you see above. See
[Architecture](architecture.md) for the tour.

## License

CountdownSolver is free and open source under the
[GPL-3.0-or-later](https://github.com/iainchesworth/CountdownSolver/blob/main/LICENSE)
license. Contributions are welcome — see [Contributing](CONTRIBUTING.md).
