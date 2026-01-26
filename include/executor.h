#ifndef EXECUTOR_H
#define EXECUTOR_H

// Execute a command (check built-ins, pipes, or external commands)
int execute_command(char **args);

// Launch external process with I/O redirection support
int launch_process(char **args);

// Launch external process with background support
int launch_process_bg(char **args, int background, const char *cmd_str);

#endif // EXECUTOR_H
