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
todo add "do something useful with your life..."
todo list
todo review (NOT YET IMPLEMENTED)
```

The executable is produced in the project root as `todo` / `todo.exe`.

## Notes

- The project uses SQLite for storage.
- SQLite creates the database file automatically if it does not already exist.
- The app stores todos in `todo.db` in the project root.
