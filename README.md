# todo.c 🗓️

A small C command-line TODO app backed by SQLite.

## Build

On Linux/macOS:

```bash
./build.sh
```

On Windows PowerShell:

```powershell
.\build.ps1
```

### Tests

```bash
ctest --test-dir build --output-on-failure
```

## Run

```bash
# Add a new TODO
todo add <text> [--due <date>]
todo add "Do something useful with my life!"
todo add "Become the president of the US" --due "tomorrow"
todo add "Fly to the moon" --due "2099-12-31 23:59:59"

# List TODOs
todo list                 # list open TODOs
todo list --overdue       # list overdue TODOs
todo list --all           # list all TODOs
todo list --until <date>  # list issue which are due until <date> (NOT YET IMPLEMENTED)

# Add due date to a TODO
todo due <id> <date>
todo due 13 "today"
todo due 42 "2026-08-16 12:52:28"

# Mark a TODO as done
todo done <id>
todo done 42

# Interactively review overdue TODOs (NOT YET IMPLEMENTED)
todo review

# <date> can be:
 - ISO datetime: "YYYY-MM-DD hh:mm:ss"
 - ISO date:     "YYYY-MM-DD"
 - Today:        "today"
 - Tomorrow:     "tomorrow"
 - In a week:    "week"
 - In a month:   "month"
 - In a year:    "year"
```

The executable is produced in the project root as `todo` / `todo.exe`.

## Notes

- The project uses SQLite for storage.
- SQLite creates the database file automatically if it does not already exist.
- The app stores todos in `todo.db` in the project root.
