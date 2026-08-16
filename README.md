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

## Run

```bash
# Add a new TODO
todo add <text>
todo add "Do something useful with my life!"

# List TODOs
todo list        # list overdue TODOs
todo list --all  # list all TODOs

# Add due date to a TODO
todo due <id> <date>
todo due 42 "2026-08-16 12:52:28"

# Mark a TODO as done
todo done <id>
todo done 42

# Interactively review overdue TODOs (NOT YET IMPLEMENTED)
todo review
```

The executable is produced in the project root as `todo` / `todo.exe`.

## Notes

- The project uses SQLite for storage.
- SQLite creates the database file automatically if it does not already exist.
- The app stores todos in `todo.db` in the project root.
