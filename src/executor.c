#include "../include/common.h"
#include "../include/executor.h"
#include "../include/builtins.h"
#include "../include/pipe.h"

int launch_process(char **args) {
    pid_t pid;
    int status;
    int redirect = -1;
    char *file = NULL;

    // I/O Redirects
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            redirect = 0;
            file = args[i + 1];
            args[i] = NULL;
            break;
        }
        if (strcmp(args[i], ">>") == 0) {
            redirect = 1;
            file = args[i + 1];
            args[i] = NULL;
            break;
        }
        if (strcmp(args[i], "<") == 0) {
            redirect = 2;
            file = args[i + 1];
            args[i] = NULL;
            break;
        }
    }

    pid = fork();
    if (pid == 0) {
        // Child process
        if (redirect == 0) {
            int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("TERMINAL: OPEN");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (redirect == 1) {
            int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1) {
                perror("TERMINAL: OPEN");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (redirect == 2) {
            int fd = open(file, O_RDONLY);
            if (fd == -1) {
                perror("TERMINAL: OPEN");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (execvp(args[0], args) == -1) {
            perror("TERMINAL: COMMAND NOT FOUND");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("TERMINAL: FORK");
    } else {
        // Parent process
        waitpid(pid, &status, WUNTRACED);
    }

    return 1;
}

int execute_command(char **args) {
    int start = 0;
    int pipe_pos = -1;
    int end = 0;

    if (args[0] == NULL) {
        return 1; // Empty command
    }

    // Find pipe position and end
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_pos = i;
        }
        end = i + 1;
    }

    if (pipe_pos != -1) {
        return pipe_cmd(args, start, end, pipe_pos);
    }

    // Check for built-in commands
    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_names[i]) == 0) {
            return (*builtin_functions[i])(args);
        }
    }

    // Execute external command
    return launch_process(args);
}
