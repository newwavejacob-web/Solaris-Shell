# Solaris Shell

A small Unix shell written in C, styled after the green-phosphor terminals from *Fallout*. It's a single file, `conch.c`, built directly on the POSIX process APIs: `fork`, `execvp`, `waitpid`, `pipe`, `dup2`.

## Features

- **REPL loop.** Reads, parses, and runs commands until you type `exit`.
- **Quote-aware tokenizer.** `"double"` and `'single'` quoted arguments stay as one token, and the input buffers grow as needed.
- **External commands.** Run in a forked child with `execvp`, with `PATH` lookup.
- **I/O redirection.** `>` (truncate), `>>` (append), `<` (input), set up with `open` + `dup2` in the child.
- **Pipes.** `cmd1 | cmd2` connects two processes with `pipe()`, and the parent waits on both children.
- **Built-ins.** `cd` (defaults to `$HOME`), `help`, `status`, `clear`, `exit`, dispatched through a function-pointer table.
- **Terminal theme.** ASCII boot banner, ANSI colors, and a `[HH:MM:SS] dir TERMINAL>` prompt.

## Build & run

```bash
gcc -Wall -o solaris conch.c
./solaris
```

## Examples

```
[14:02:11] Solaris-Shell TERMINAL> ls -la | grep conch
[14:02:15] Solaris-Shell TERMINAL> echo "hello wasteland" > out.txt
[14:02:20] Solaris-Shell TERMINAL> wc -l < out.txt
[14:02:24] Solaris-Shell TERMINAL> status
```

## Limitations / roadmap

- Only one pipe per command line (no `a | b | c`).
- Pipes and redirection can't be combined in the same command.
- No job control yet: no background `&`, `fg`/`bg`, or signal handling for Ctrl-C/Ctrl-Z.
- No environment variable expansion or globbing beyond what the invoked programs do themselves.
