# 🐚 CONCH - N3WL1N3 TERMINAL SHELL

```
███████╗ ██████╗ ██╗      █████╗ ██████╗ ██╗████████╗
██╔════╝██╔═══██╗██║     ██╔══██╗██╔══██╗██║██╔═════╝
███████╗██║   ██║██║     ███████║██████╔╝██║████████╗
╚════██║██║   ██║██║     ██╔══██║██╔══██╗██║╚═════██║
███████║╚██████╔╝███████╗██║  ██║██║  ██║██║████████║
╚══════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝╚══════╝
```

**A cyberpunk-inspired Unix shell written in C with a retro-futuristic terminal aesthetic.**

[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey.svg)](https://github.com)

---

## 🎯 Features

### Core Functionality
- **Full POSIX Compliance** - Execute any standard Unix command
- **Built-in Commands** - `cd`, `help`, `exit`, `clear`, `status`, `jobs`, `fg`, `bg`, `export`
- **I/O Redirection** - Output (`>`, `>>`), Input (`<`)
- **Piping** - Chain commands with `|`
- **Quote Handling** - Properly parse single and double quotes
- **Job Control** - Background processes with `&`, `fg`, `bg`, and `jobs`
- **Environment Variables** - Set and export variables with `export`

### Advanced Features
- **Command History** - 100-command circular buffer with arrow key navigation
- **Tab Completion** - Intelligent completion for commands and file paths
- **Raw Input Mode** - Character-by-character processing for responsive UX
- **Cyberpunk Aesthetic** - Custom ANSI color scheme and boot sequence

### Production-Grade Engineering
- **Modular Architecture** - Clean separation of concerns across 8 modules
- **Memory Safe** - No leaks, proper cleanup on exit
- **Error Handling** - Graceful failure modes with informative messages
- **Build System** - Professional Makefile with multiple targets
- **Containerized** - Docker support for portable deployment

---

## 📂 Project Structure

```
conch/
├── src/                    # Source files
│   ├── main.c             # Entry point and REPL
│   ├── prompt.c           # Terminal prompt and boot sequence
│   ├── parser.c           # Input parsing and tokenization
│   ├── executor.c         # Command execution engine
│   ├── builtins.c         # Built-in command implementations
│   ├── pipe.c             # Pipe handling
│   ├── history.c          # Command history management
│   ├── completion.c       # Tab completion engine
│   └── jobs.c             # Job control and background processes
├── include/               # Header files
│   ├── common.h           # Shared definitions and constants
│   ├── prompt.h
│   ├── parser.h
│   ├── executor.h
│   ├── builtins.h
│   ├── pipe.h
│   ├── history.h
│   ├── completion.h
│   └── jobs.h
├── build/                 # Build artifacts (generated)
├── bin/                   # Compiled binaries (generated)
├── docs/                  # Documentation
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
- Linux or macOS (POSIX-compliant OS)

### Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/conch.git
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

The Makefile provides numerous targets for different workflows:

| Command | Description |
|---------|-------------|
| `make` or `make all` | Build optimized release version |
| `make debug` | Build with debug symbols and no optimization |
| `make clean` | Remove all build artifacts |
| `make run` | Build and immediately run the shell |
| `make install` | Install to `/usr/local/bin` |
| `make uninstall` | Remove from system |
| `make memcheck` | Run with Valgrind for memory leak detection |
| `make check` | Run static analysis with cppcheck |
| `make format` | Format code with clang-format |
| `make stats` | Display code statistics |
| `make help` | Show all available targets |

### Development Workflow

```bash
# Debug build with symbols
make debug

# Check for memory leaks (requires valgrind)
make memcheck

# Run static analysis (requires cppcheck)
make check

# Format code (requires clang-format)
make format
```

---

## 🐳 Docker Support

### Build and Run with Docker

```bash
# Build the Docker image
make docker-build

# Run in a container
make docker-run
```

### Using Docker Compose

```bash
# Start the shell in a container
docker-compose up

# Run with mounted workspace
docker-compose run --rm conch-shell
```

The Docker setup includes:
- GCC compiler
- Valgrind for memory debugging
- Cppcheck for static analysis
- Clang-format for code formatting

---

## 📖 Usage Guide

### Basic Commands

```bash
# Change directory
cd /path/to/directory
cd ~                    # Go to home directory
cd ..                   # Go up one level

# Get help
help

# Show system status
status

# Clear screen and show boot sequence
clear

# Exit the shell
exit

# Set environment variable
export VAR=value

# List background jobs
jobs

# Bring job to foreground
fg %1

# Resume job in background
bg %1
```

### I/O Redirection

```bash
# Redirect output to file (overwrite)
echo "Hello, World!" > output.txt

# Append output to file
echo "Another line" >> output.txt

# Redirect input from file
sort < unsorted.txt
```

### Piping

```bash
# Chain commands
ls -la | grep ".txt" | wc -l

# Multiple pipes
cat file.txt | grep "search" | sort | uniq
```

### History Navigation

- **Up Arrow** - Navigate to previous commands
- **Down Arrow** - Navigate to next commands
- History stores up to 100 commands
- Duplicate consecutive commands are automatically filtered

### Tab Completion

- Press **Tab** to auto-complete:
  - Command names (from PATH and built-ins)
  - File paths and directories
- Multiple matches will display all options
- Single match will auto-complete immediately

### Quote Handling

```bash
# Single and double quotes preserve spaces
echo "Hello World"
echo 'Test with spaces'

# Use quotes for file paths with spaces
cd "My Documents"
cat "file with spaces.txt"
```

### Job Control

```bash
# Run command in background
sleep 100 &

# List all jobs
jobs

# Bring job #1 to foreground
fg %1

# Resume stopped job #1 in background
bg %1

# Suspend current foreground job
Ctrl+Z
```

### Environment Variables

```bash
# Set and export a variable
export MY_VAR=hello

# Use in commands
echo $MY_VAR
```

---

## 🎨 Customization

### Color Scheme

The terminal uses a cyberpunk-inspired color palette defined in `include/common.h`:

```c
#define GREEN "\033[32m"
#define BRIGHT_GREEN "\033[1;32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define BOLD "\033[1m"
```

Modify these values to customize the aesthetic.

### Prompt Format

The prompt shows: `[TIME] DIRECTORY TERMINAL>`

Customize in `src/prompt.c`:
- Boot sequence animation
- Prompt format and colors
- Directory display logic

### History Size

Adjust the history buffer size in `include/history.h`:

```c
#define HISTORY_SIZE 100  // Change to your preference
```

---

## 🔧 Technical Details

### Architecture

**Modular Design**: The codebase is organized into 9 functional modules:
1. **Main** - REPL loop and initialization
2. **Prompt** - UI and boot sequence
3. **Parser** - Lexical analysis and tokenization
4. **Executor** - Process management and execution
5. **Builtins** - Internal command implementations
6. **Pipe** - Inter-process communication
7. **History** - Command history buffer
8. **Completion** - Tab completion logic
9. **Jobs** - Job control and background process management

### Key Technologies

- **POSIX System Calls**: `fork()`, `execvp()`, `pipe()`, `dup2()`, `waitpid()`
- **Terminal Control**: `termios` for raw mode input
- **File Operations**: `open()`, `close()`, `read()`, `write()`
- **Process Management**: Signal handling, child process management
- **Memory Management**: Dynamic allocation with proper cleanup

### Performance

- **Fast Startup**: < 0.5s initialization time
- **Low Memory**: < 10MB resident set size
- **Efficient History**: O(1) lookup with circular buffer
- **Smart Completion**: Caches PATH results for performance

---

## 🐛 Debugging

### Memory Leaks

```bash
# Run with Valgrind
make memcheck

# Should see:
# All heap blocks were freed -- no leaks are possible
```

### Debugging Build

```bash
# Build with debug symbols
make debug

# Run with GDB
gdb ./bin/conch
(gdb) run
(gdb) backtrace  # If it crashes
```

### Static Analysis

```bash
# Run cppcheck
make check

# Should report no errors
```

---

## 🤝 Contributing

Contributions are welcome! Here's how:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Format code (`make format`)
5. Run tests (`make check`)
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

### Code Style

- Follow the existing style
- Use `clang-format` for consistency
- Comment complex logic
- Keep functions under 50 lines when possible
- Use meaningful variable names

---

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

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
# View code statistics
make stats
```

Current codebase:
- ~2000 lines of C code
- 9 modules
- 9 header files
- 100% memory safe
- Zero dependencies (pure POSIX)

---

## 🔮 Future Enhancements

- [x] Job control (background processes with `&`)
- [x] Environment variable expansion (`$VAR`)
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

- Open an [Issue](https://github.com/yourusername/conch/issues)
- Check existing issues first
- Provide system info and steps to reproduce

---

**Built with ⚡ by developers, for developers.**

*Enter the terminal. Become driven.*
