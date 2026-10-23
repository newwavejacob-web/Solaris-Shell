#include "../include/common.h"
#include "../include/pipe.h"

int pipe_cmd(char **args, int start, int end, int pipe_pos) {
    // Validate input parameters
    if (pipe_pos <= start || pipe_pos >= end - 1) {
        fprintf(stderr, "conch: INVALID PIPE POSITION\n");
        return 1;
    }

    int cmd1_size = pipe_pos - start + 1;
    int cmd2_size = end - pipe_pos;

    if (cmd1_size <= 0 || cmd2_size <= 0) {
        fprintf(stderr, "conch: INVALID COMMAND SIZE\n");
        return 1;
    }

    char **cmd1 = malloc(sizeof(char*) * cmd1_size);
    char **cmd2 = malloc(sizeof(char*) * cmd2_size);

    if (!cmd1 || !cmd2) {
        fprintf(stderr, "conch: MEMORY ALLOCATION FAILED\n");
        if (cmd1) free(cmd1);
        if (cmd2) free(cmd2);
        return 1;
    }

    int i = 0;
    for (int idx = start; idx < pipe_pos; idx++) {
        cmd1[i] = args[idx];
        i++;
    }
    cmd1[i] = NULL;

    int j = 0;
    for (int idx = pipe_pos + 1; idx < end; idx++) {
        cmd2[j] = args[idx];
        j++;
    }
    cmd2[j] = NULL;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("conch: PIPE");
        free(cmd1);
        free(cmd2);
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Child process 1 - write to pipe
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("conch: DUP2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);

        if (execvp(cmd1[0], cmd1) == -1) {
            perror("conch: COMMAND 1 NOT FOUND");
            exit(EXIT_FAILURE);
        }
    } else if (pid1 < 0) {
        perror("conch: FORK 1");
        close(pipefd[0]);
        close(pipefd[1]);
        free(cmd1);
        free(cmd2);
        return 1;
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Child process 2 - read from pipe
        close(pipefd[1]);
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("conch: DUP2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[0]);

        if (execvp(cmd2[0], cmd2) == -1) {
            perror("conch: COMMAND 2 NOT FOUND");
            exit(EXIT_FAILURE);
        }
    } else if (pid2 < 0) {
        perror("conch: FORK 2");
        close(pipefd[0]);
        close(pipefd[1]);
        free(cmd1);
        free(cmd2);
        return 1;
    }

    // Close both pipes in parent process
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes to finish
    int status1, status2;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    // Free command arrays
    free(cmd1);
    free(cmd2);

    return 1;
}
