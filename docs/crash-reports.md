# Crash reports

Every crash so far has been a single wrong type or a lost value somewhere in the
reconstruction, and each one fixed takes a class of others with it. So a report helps.

- The mingw build writes a report with a symbolised stack trace to
  `d2crash-<date>-<time>.txt` next to the executable.
- The Visual C++ build writes `Crash.txt` and a minidump to `BlizzardError/<date time>/`
  in the game folder.

Attach the file to an [issue](https://github.com/jaenster/redivivus/issues), with what you
were doing when it happened.
