#include "../include/common.h"
#include "../include/executor.h"
#include "../include/builtins.h"
#include "../include/pipe.h"
#include "../include/jobs.h"

// Check if command should run in background and remove the &
static int check_background(char **args, char **full_command) {
    int background = 0;
    int i = 0;
    int len = 0;

    // Find end of args and calculate command length
    while (args[i] != NULL) {
        len += strlen(args[i]) + 1;
        i++;
    }

    // Build full command string for job tracking
    *full_command = malloc(len + 1);
    if (*full_command) {
        (*full_command)[0] = '\0';
        for (int j = 0; args[j] != NULL; j++) {
            if (j > 0) strcat(*full_command, " ");
            strcat(*full_command, args[j]);
        }
    }

    // Check for & at end
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        background = 1;
        args[i - 1] = NULL;
        // Remove trailing " &" from command string
        if (*full_command) {
            int cmd_len = strlen(*full_command);
            if (cmd_len >= 2) {
                (*full_command)[cmd_len - 2] = '\0';
            }
        }
    }

    return background;
}

int launch_process(char **args) {
    return launch_process_bg(args, 0, NULL);
}

int launch_process_bg(char **args, int background, const char *cmd_str) {
    pid_t pid;
    int status;
    int redirect_out = -1;
    int redirect_in = -1;
    int redirect_err = -1;
    char *out_file = NULL;
    char *in_file = NULL;
    char *err_file = NULL;
    int append_out = 0;

    // I/O Redirects - handle multiple redirections
    for (int i = 0; args[i] != NULL; i++) {
        // stderr redirect 2>
        if (strcmp(args[i], "2>") == 0) {
            redirect_err = 1;
            err_file = args[i + 1];
            // Shift remaining args
            for (int j = i; args[j + 2] != NULL; j++) {
                args[j] = args[j + 2];
            }
            args[i] = NULL;
            i--;
            continue;
        }
        // stdout redirect >
        if (strcmp(args[i], ">") == 0) {
            redirect_out = 1;
            append_out = 0;
            out_file = args[i + 1];
            for (int j = i; args[j + 2] != NULL; j++) {
                args[j] = args[j + 2];
            }
            args[i] = NULL;
            i--;
            continue;
        }
        // stdout append >>
        if (strcmp(args[i], ">>") == 0) {
            redirect_out = 1;
            append_out = 1;
            out_file = args[i + 1];
            for (int j = i; args[j + 2] != NULL; j++) {
                args[j] = args[j + 2];
            }
            args[i] = NULL;
            i--;
            continue;
        }
        // stdin redirect <
        if (strcmp(args[i], "<") == 0) {
            redirect_in = 1;
            in_file = args[i + 1];
            for (int j = i; args[j + 2] != NULL; j++) {
                args[j] = args[j + 2];
            }
            args[i] = NULL;
            i--;
            continue;
        }
    }

    pid = fork();
    if (pid == 0) {
        // Child process

        // Put child in its own process group
        pid_t child_pid = getpid();
        setpgid(child_pid, child_pid);

        // If foreground, give child terminal control
        if (!background && shell_is_interactive) {
            tcsetpgrp(shell_terminal, child_pid);
        }

        // Restore default signal handlers in child
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        // Handle stdout redirection
        if (redirect_out == 1 && out_file) {
            int flags = O_WRONLY | O_CREAT | (append_out ? O_APPEND : O_TRUNC);
            int fd = open(out_file, flags, 0644);
            if (fd == -1) {
                perror("TERMINAL: OPEN");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Handle stdin redirection
        if (redirect_in == 1 && in_file) {
            int fd = open(in_file, O_RDONLY);
            if (fd == -1) {
                perror("TERMINAL: OPEN");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Handle stderr redirection
        if (redirect_err == 1 && err_file) {
            int fd = open(err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("TERMINAL: OPEN");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDERR_FILENO);
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
        setpgid(pid, pid);  // Ensure child is in its own process group

        if (background) {
            // Background job
            int job_id = job_add(pid, cmd_str ? cmd_str : args[0], 1);
            printf("[%d] %d\n", job_id, pid);
        } else {
            // Foreground job
            if (shell_is_interactive) {
                tcsetpgrp(shell_terminal, pid);
            }

            // Wait for foreground process
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));

            // If stopped, add to job list
            if (WIFSTOPPED(status)) {
                int job_id = job_add(pid, cmd_str ? cmd_str : args[0], 1);
                printf("\n[%d]+ Stopped                 %s\n", job_id, cmd_str ? cmd_str : args[0]);
            }

            // Put shell back in foreground
            if (shell_is_interactive) {
                tcsetpgrp(shell_terminal, shell_pgid);
                tcsetattr(shell_terminal, TCSADRAIN, &shell_tmodes);
            }
        }
    }

    return 1;
}

int execute_command(char **args) {
    int start = 0;
    int pipe_pos = -1;
    int end = 0;
    int background = 0;
    char *full_command = NULL;

    if (args[0] == NULL) {
        return 1; // Empty command
    }

    // Check for background execution and build command string
    background = check_background(args, &full_command);

    // Check for completed background jobs
    job_check_status();

    // Find pipe position and end
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_pos = i;
        }
        end = i + 1;
    }

    if (pipe_pos != -1) {
        int result = pipe_cmd(args, start, end, pipe_pos);
        free(full_command);
        return result;
    }

    // Check for built-in commands (builtins don't run in background)
    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_names[i]) == 0) {
            int result = (*builtin_functions[i])(args);
            free(full_command);
            return result;
        }
    }

    // Execute external command
    int result = launch_process_bg(args, background, full_command);
    free(full_command);
    return result;
}
