#include "../include/common.h"
#include "../include/builtins.h"
#include "../include/prompt.h"

char *builtin_names[] = {
    "cd",
    "help",
    "exit",
    "clear",
    "status"
};

int (*builtin_functions[]) (char **) = {
    &builtin_cd,
    &builtin_help,
    &builtin_exit,
    &builtin_clear,
    &builtin_status
};

int num_builtins(void) {
    return sizeof(builtin_names) / sizeof(char *);
}

int builtin_cd(char **args) {
    if (args[1] == NULL) {
        if (chdir(getenv("HOME")) != 0) {
            perror("TERMINAL: CD");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("TERMINAL: CD");
        }
    }
    return 1;
}

int builtin_help(char **args) {
    (void)args;
    printf(YELLOW "NEO TERMINAL HELP\n" RESET);
    printf("=============================\n");
    printf("Built-in commands:\n");
    for (int i = 0; i < num_builtins(); i++) {
        printf("  %s\n", builtin_names[i]);
    }
    printf("\nI/O Redirection:\n");
    printf("  > file    - Redirect output to file\n");
    printf("  >> file   - Append output to file\n");
    printf("  < file    - Redirect input from file\n");
    printf("  cmd1 | cmd2 - Pipe output of cmd1 to cmd2\n");
    printf("\nHistory Navigation:\n");
    printf("  Up/Down arrows - Navigate command history\n");
    printf("  Tab - Auto-complete commands and paths\n");
    printf("\nUse 'man command' for more information on system commands.\n");
    return 1;
}

int builtin_exit(char **args) {
    (void)args;
    printf(GREEN "TERMINAL SESSION TERMINATED\n" RESET);
    printf("we do it because we are driven.\n");
    return 0;
}

int builtin_clear(char **args) {
    (void)args;
    printf("\033[2J\033[H");
    print_boot_sequence();
    return 1;
}

int builtin_status(char **args) {
    (void)args;
    char *cwd = getcwd(NULL, 0);
    printf(CYAN "SYSTEM STATUS: OPERATIONAL\n" RESET);
    printf("NEO TERMINAL VERSION: 2.0\n");
    printf("ACCESS: USER\n");
    if (cwd) {
        printf("CURRENT DIR: %s\n", cwd);
        free(cwd);
    }
    return 1;
}
