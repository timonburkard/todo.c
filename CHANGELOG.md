# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v0.0.3] -- 2026-08-18

### Added

- [cli]  prompt for creation of a new database if none exists in the current directory
- [cli]  added `--help`.
- [test] adjusted tests due to the new prompting behavior.

### Changed

- [minor] added more comments
- [error] harmonized error handling

### Fixed

- [review] invalid due date input is now re-prompted.

## [v0.0.2] -- 2026-08-17

### Added

- [review] added the interactive `review` command
- [cli]    added `--search <text>` option to `list`
- [cli]    added `--until <date>` option to `list`
- [cmd]    added support for providing dates in different formats
- [tests]  added integration tests
- [cli]    added `due` and `done` commands
- [ci]     added build pipeline
- [doc]    added MIT license
- [doc]    added more info to README

### Changed

- [cli]   changed the default `list` behavior to show all open issues
- [build] enabled Release builds with `-O3` optimization
- [build] added stricter compiler flags

### Fixed

- [review] fixed invalid due date input handling

## [v0.0.1] -- 2026-08-16

### Added

- [general] added initial project
- [cli]     added initial CLI functionality
- [cli]     added version information
- [ci]      added initial CI pipeline
- [doc]     added README
