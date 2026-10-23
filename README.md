# 🐚 CONCH SHELL

```
 ██████╗ ██████╗ ███╗   ██╗ ██████╗██╗  ██╗
██╔════╝██╔═══██╗████╗  ██║██╔════╝██║  ██║
██║     ██║   ██║██╔██╗ ██║██║     ███████║
██║     ██║   ██║██║╚██╗██║██║     ██╔══██║
╚██████╗╚██████╔╝██║ ╚████║╚██████╗██║  ██║
 ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚═╝  ╚═╝
```

**A cyberpunk-inspired Unix shell written in C with a retro-futuristic terminal aesthetic.**

[![C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey.svg)](#-testing)

Conch is a learning project in systems programming. It is an interactive
command shell, not a POSIX `sh`: it runs programs, single pipes, simple
redirects and basic job control, but it has no scripting language (see
[Limitations](#-limitations)).

---

## 🎯 Features

### Core Functionality
- **External Commands** - Runs any program on `PATH` via `fork()`/`execvp()`
- **Built-in Commands** - `cd`, `help`, `exit`, `clear`, `status`, `jobs`, `fg`, `bg`, `export`
- **I/O Redirection** - Output (`>`, `>>`), input (`<`) and stderr (`2>`), one redirect per command
- **Piping** - A single pipe between two commands (`cmd1 | cmd2`)
- **Quote Handling** - A word that starts with `"` or `'` runs to the matching quote, so it can contain spaces
- **Job Control** - Background processes with `&`, plus `jobs`, `fg`, `bg` and Ctrl+Z for simple (non-pipeline) commands
- **Environment Variables** - `$VAR` / `${VAR}` expansion and `export`

### Interactive Features
- **Command History** - 100-entry in-memory circular buffer with Up/Down arrow navigation (not saved between sessions)
- **Tab Completion** - Completes command names from a built-in list of common commands and executables on `PATH`
- **Raw Input Mode** - Character-by-character input; Backspace is the only editing key (no Left/Right cursor movement)
- **Cyberpunk Aesthetic** - Custom ANSI color scheme and boot sequence

### Engineering
- **Modular Architecture** - 9 source modules with matching headers
- **Leak-Checked** - The test suite runs clean under `valgrind --leak-check=full` (see [Memory Checking](#memory-checking))
- **Error Messages** - Failures are reported with a `conch:` prefix and the shell keeps running
- **Build System** - Makefile with release, debug, test, memcheck and analysis targets
- **Containerized** - Docker image with the toolchain, valgrind and cppcheck

---

## 📂 Project Structure

```
conch/
├── src/                    # Source files
│   ├── main.c             # Entry point, REPL and raw-mode line reader
│   ├── prompt.c           # Terminal prompt and boot sequence
│   ├── parser.c           # Variable expansion and tokenization
│   ├── executor.c         # Command execution and redirection
│   ├── builtins.c         # Built-in command implementations
│   ├── pipe.c             # Pipe handling
│   ├── history.c          # Command history management
│   ├── completion.c       # Tab completion engine
│   └── jobs.c             # Job control and background processes
├── include/               # Header files (common.h + one per module)
├── tests/
│   └── run_tests.sh       # Test suite (pipes, redirects, builtins)
├── build/                 # Build artifacts (generated)
├── bin/                   # Compiled binaries (generated)
├── Makefile              # Build system
├── Dockerfile            # Container definition
├── docker-compose.yml    # Container orchestration
└── README.md             # This file
```

---

## 🚀 Quick Start

### Prerequisites

- GCC or Clang compiler
- GNU Make
- Linux or macOS
- Perl and Bash (for the test suite only); valgrind for `make memcheck` (Linux only)

### Installation

```bash
# Clone the repository
git clone https://github.com/newwavejacob-web/Solaris-Shell.git conch
cd conch

# Build the shell
make

# Run it
./bin/conch

# Or build and run in one command
make run
```

### System-Wide Installation

```bash
# Install to /usr/local/bin (requires sudo)
make install

# Now you can run it from anywhere
conch

# To uninstall
make uninstall
```

---

## 🛠️ Build System

| Command | Description |
|---------|-------------|
| `make` or `make all` | Build optimized release version |
| `make debug` | Build with debug symbols and no optimization |
| `make clean` | Remove all build artifacts |
| `make run` | Build and immediately run the shell |
| `make test` | Run the test suite against the release build |
| `make memcheck` | Run the test suite under valgrind (Linux) |
| `make memcheck-interactive` | Run the shell interactively under valgrind |
| `make install` | Install to `/usr/local/bin` |
| `make uninstall` | Remove from system |
| `make check` | Run static analysis with cppcheck |
| `make format` | Format code with clang-format |
| `make stats` | Display code statistics |
| `make help` | Show all available targets |

---

## 🐳 Docker Support

```bash
# Build the Docker image
make docker-build

# Run the shell in a container
make docker-run

# Run the tests / leak check inside the container
docker run --rm --entrypoint make conch-shell test
docker run --rm --entrypoint make conch-shell memcheck
```

With Docker Compose, start an interactive shell with the `./workspace`
directory mounted:

```bash
docker compose run --rm conch-shell
```

The image is based on `gcc:latest` and includes valgrind, cppcheck and
clang-format.

---

## 📖 Usage Guide

### Basic Commands

```bash
cd /path/to/directory
cd                      # Go to $HOME
cd ..                   # Go up one level

help                    # List builtins and syntax
status                  # Show version and current directory
clear                   # Clear screen and show boot sequence
exit                    # Exit the shell (Ctrl+D on an empty line also exits)

export VAR=value        # Set an environment variable
export                  # List the environment

jobs                    # List background jobs
fg %1                   # Bring job 1 to the foreground
bg %1                   # Resume stopped job 1 in the background
```

### I/O Redirection

Redirect operators must be separate words (`> out.txt`, not `>out.txt`).

```bash
echo "Hello, World!" > output.txt   # overwrite
echo "Another line" >> output.txt   # append
sort < unsorted.txt                 # input
ls missing 2> errors.txt            # stderr
```

### Piping

```bash
ls -la | grep ".txt"
cat file.txt | sort
```

Only one `|` per command line is supported.

### History Navigation

- **Up Arrow** - Previous command
- **Down Arrow** - Next command
- Up to 100 commands, kept in memory for the current session
- Consecutive duplicate commands are stored once

### Tab Completion

- Press **Tab** to complete a command name (from a list of common commands
  and executables on `PATH`)
- A single match replaces the line; multiple matches are listed (up to 20)
- Completion matches against the whole line, so it only helps while typing
  the first word

### Quote Handling

```bash
echo "Hello World"
cd "My Documents"
cat 'file with spaces.txt'
```

### Job Control

```bash
sleep 100 &     # Run in background
jobs            # List jobs
fg %1           # Bring job 1 to the foreground
bg %1           # Resume stopped job 1 in the background
# Ctrl+Z suspends the foreground job
```

### Environment Variables

```bash
export MY_VAR=hello
echo $MY_VAR ${MY_VAR}
```

---

## ⚠️ Limitations

Conch is not POSIX-compliant. Notably, it does **not** support:

- Command lists and conditionals: `;`, `&&`, `||`
- More than one pipe (`a | b | c`), or redirects inside a pipeline
- More than one redirect per command (`cmd < in > out` ignores `> out`)
- Arguments after a redirect (`sort < file -r` drops `-r`)
- Redirects without surrounding spaces (`>file`)
- Single quotes suppressing expansion (`'$HOME'` is expanded)
- Joining adjacent quoted text (`foo"bar"` stays two pieces)
- Backslash escapes, globbing (`*.txt`), `~` expansion, `$?`, subshells,
  here-documents, aliases, scripts or control flow
- Background pipelines (`a | b &` runs in the foreground)

Each of the first group is covered by an expected-failure (`XFAIL`) test in
`tests/run_tests.sh`, which reports `XPASS` once it starts working.

Known issues found while auditing (not yet fixed):

- Recalling a history entry longer than 1024 characters with the Up arrow
  overflows the line buffer (`strcpy` in `src/main.c`).
- When stdout is not a terminal, the echoed command line can be flushed after
  the command's own output.
- `make check` reports two cppcheck errors: `realloc` failure paths in
  `expand_variables` (`src/parser.c`) leak the old buffer.

---

## 🧪 Testing

```bash
make test
```

`tests/run_tests.sh` feeds scripts to `bin/conch` on stdin and checks the
files and output they produce. It covers external commands, quoting,
variable expansion, every redirect operator, pipes, and every builtin.
Interactive-only features (arrow-key history, Tab completion, Ctrl+C/Ctrl+Z)
need a real terminal and are not covered.

Current result on macOS (Apple clang 17, arm64) and Linux (Debian, `gcc:latest`, arm64):

```
passed: 29  failed: 0  xfail: 9  xpass: 0
```

### Memory Checking

```bash
make memcheck       # Linux; on macOS use the Docker command below
docker run --rm --entrypoint make conch-shell memcheck
```

This runs the whole test suite (all 38 cases, including the expected-failure
ones) with each shell session under:

```
valgrind --leak-check=full --show-leak-kinds=all \
         --errors-for-leak-kinds=definite,indirect,possible \
         --child-silent-after-fork=yes --error-exitcode=99
```

and fails if any session reports an error or a leak. Last result
(valgrind 3.24.0, Debian, arm64): every session ends with
`All heap blocks were freed -- no leaks are possible` and
`ERROR SUMMARY: 0 errors`.

The check only covers the code paths the suite exercises; the interactive
line editor, history recall and Tab completion are not run under valgrind.
`--child-silent-after-fork` means forked children (which immediately `exec`
or exit) are not checked.

### Debugging Build

```bash
make debug
gdb ./bin/conch
```

### Static Analysis

```bash
make check
```

Requires cppcheck. See [Known issues](#-limitations) for its current findings.

---

## 🎨 Customization

### Color Scheme

The color palette is defined in `include/common.h`:

```c
#define GREEN "\033[32m"
#define BRIGHT_GREEN "\033[1;32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define BOLD "\033[1m"
```

### Prompt Format

The prompt shows: `[TIME] DIRECTORY CONCH>`

Customize the boot sequence, prompt format and colors in `src/prompt.c`.

### History Size

Adjust the history buffer size in `include/history.h`:

```c
#define HISTORY_SIZE 100  // Change to your preference
```

---

## 🔧 Technical Details

### Architecture

1. **Main** - REPL loop, raw-mode line reader
2. **Prompt** - UI and boot sequence
3. **Parser** - Variable expansion and tokenization
4. **Executor** - Process launch and I/O redirection
5. **Builtins** - Internal command implementations
6. **Pipe** - Two-command pipelines
7. **History** - Command history buffer
8. **Completion** - Tab completion logic
9. **Jobs** - Job table, signal handling and terminal control

### Key APIs

- **Processes**: `fork()`, `execvp()`, `waitpid()`, `setpgid()`, `kill()`
- **I/O**: `pipe()`, `dup2()`, `open()`, `close()`, `read()`
- **Terminal Control**: `termios` raw mode, `tcsetpgrp()`
- **Signals**: `sigaction()` SIGCHLD handler for background jobs

### Performance

- **Startup**: about 0.5s, almost all of it a deliberate 0.5s pause in the boot sequence (`usleep` in `src/prompt.c`)
- **Memory**: about 1.6 MB maximum resident set size (`/usr/bin/time -l`, macOS arm64)
- **History**: O(1) lookup by index in a circular buffer
- **Completion**: Scans the `PATH` directories on every Tab press (no caching)

---

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Format code (`make format`)
5. Run the tests (`make test`, and `make memcheck` on Linux)
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

### Code Style

- Follow the existing style
- Comment complex logic
- Keep functions under 50 lines when possible
- Use meaningful variable names

---

## 👤 Author

**SOLARIS**

Created with the ethos: *"we do it because we are driven."*

---

## 🙏 Acknowledgments

- Inspired by classic Unix shells (bash, zsh, sh)
- Cyberpunk aesthetic influenced by sci-fi terminals
- Built as a learning exercise in systems programming

---

## 📊 Stats

```bash
make stats
```

Current codebase:
- ~1,800 lines of C (source and headers)
- 9 modules, 9 header files
- No third-party libraries (libc only)

---

## 🔮 Future Enhancements

- [x] Job control (background processes with `&`)
- [x] Environment variable expansion (`$VAR`)
- [ ] Multiple pipes and redirects in pipelines
- [ ] `;`, `&&` and `||`
- [ ] Alias support
- [ ] Script execution mode
- [ ] Configuration file (~/.conchrc)
- [ ] Plugin system
- [ ] Syntax highlighting
- [ ] Multi-line command editing
- [ ] Reverse-i-search (Ctrl+R)
- [ ] Customizable keybindings

---

## 📬 Support

Having issues? Found a bug?

- Open an [Issue](https://github.com/newwavejacob-web/Solaris-Shell/issues)
- Check existing issues first
- Provide system info and steps to reproduce

---

**Built with ⚡ by developers, for developers.**

*Enter the terminal. Become driven.*
