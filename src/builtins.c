#include "../include/common.h"
#include "../include/builtins.h"
#include "../include/prompt.h"
#include "../include/jobs.h"

char *builtin_names[] = {
    "cd",
    "help",
    "exit",
    "clear",
    "status",
    "jobs",
    "fg",
    "bg",
    "export"
};

int (*builtin_functions[]) (char **) = {
    &builtin_cd,
    &builtin_help,
    &builtin_exit,
    &builtin_clear,
    &builtin_status,
    &builtin_jobs,
    &builtin_fg,
    &builtin_bg,
    &builtin_export
};

int num_builtins(void) {
    return sizeof(builtin_names) / sizeof(char *);
}

int builtin_cd(char **args) {
    if (args[1] == NULL) {
        if (chdir(getenv("HOME")) != 0) {
            perror("conch: CD");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("conch: CD");
        }
    }
    return 1;
}

int builtin_help(char **args) {
    (void)args;
    printf(YELLOW "CONCH HELP\n" RESET);
    printf("=============================\n");
    printf("Built-in commands:\n");
    for (int i = 0; i < num_builtins(); i++) {
        printf("  %s\n", builtin_names[i]);
    }
    printf("\nI/O Redirection:\n");
    printf("  > file    - Redirect output to file\n");
    printf("  >> file   - Append output to file\n");
    printf("  < file    - Redirect input from file\n");
    printf("  2> file   - Redirect stderr to file\n");
    printf("  cmd1 | cmd2 - Pipe output of cmd1 to cmd2\n");
    printf("\nJob Control:\n");
    printf("  command & - Run command in background\n");
    printf("  Ctrl+Z    - Suspend foreground job\n");
    printf("  Ctrl+C    - Interrupt foreground job\n");
    printf("  jobs      - List background jobs\n");
    printf("  fg [%%N]   - Bring job N to foreground\n");
    printf("  bg [%%N]   - Resume job N in background\n");
    printf("\nEnvironment Variables:\n");
    printf("  $VAR or ${VAR} - Expand variable\n");
    printf("  export VAR=val - Set environment variable\n");
    printf("\nHistory Navigation:\n");
    printf("  Up/Down arrows - Navigate command history\n");
    printf("  Tab - Auto-complete commands and paths\n");
    printf("\nUse 'man command' for more information on system commands.\n");
    return 1;
}

int builtin_exit(char **args) {
    (void)args;
    printf(GREEN "CONCH SESSION TERMINATED\n" RESET);
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
    printf("CONCH VERSION: " CONCH_VERSION "\n");
    printf("ACCESS: USER\n");
    if (cwd) {
        printf("CURRENT DIR: %s\n", cwd);
        free(cwd);
    }
    return 1;
}

int builtin_jobs(char **args) {
    (void)args;
    jobs_list();
    return 1;
}

int builtin_fg(char **args) {
    job_t *job = NULL;

    if (args[1] == NULL) {
        // Find most recent job
        for (int i = MAX_JOBS - 1; i >= 0; i--) {
            if (jobs[i] != NULL && jobs[i]->state != JOB_DONE) {
                job = jobs[i];
                break;
            }
        }
    } else {
        // Parse job ID (handle %N format)
        int job_id;
        if (args[1][0] == '%') {
            job_id = atoi(&args[1][1]);
        } else {
            job_id = atoi(args[1]);
        }
        job = job_find_by_id(job_id);
    }

    if (job == NULL) {
        fprintf(stderr, "conch: fg: no such job\n");
        return 1;
    }

    printf("%s\n", job->command);
    job_foreground(job, job->state == JOB_STOPPED);
    return 1;
}

int builtin_bg(char **args) {
    job_t *job = NULL;

    if (args[1] == NULL) {
        // Find most recent stopped job
        for (int i = MAX_JOBS - 1; i >= 0; i--) {
            if (jobs[i] != NULL && jobs[i]->state == JOB_STOPPED) {
                job = jobs[i];
                break;
            }
        }
    } else {
        // Parse job ID (handle %N format)
        int job_id;
        if (args[1][0] == '%') {
            job_id = atoi(&args[1][1]);
        } else {
            job_id = atoi(args[1]);
        }
        job = job_find_by_id(job_id);
    }

    if (job == NULL) {
        fprintf(stderr, "conch: bg: no such job\n");
        return 1;
    }

    if (job->state != JOB_STOPPED) {
        fprintf(stderr, "conch: bg: job already running\n");
        return 1;
    }

    job_background(job, 1);
    return 1;
}

int builtin_export(char **args) {
    if (args[1] == NULL) {
        // List all exported environment variables
        extern char **environ;
        for (char **env = environ; *env != NULL; env++) {
            printf("export %s\n", *env);
        }
        return 1;
    }

    // Process each export argument
    for (int i = 1; args[i] != NULL; i++) {
        char *arg = args[i];
        char *equals = strchr(arg, '=');

        if (equals != NULL) {
            // VAR=value format
            *equals = '\0';
            char *name = arg;
            char *value = equals + 1;

            if (setenv(name, value, 1) != 0) {
                perror("conch: export");
            }
            *equals = '=';  // Restore for any further use
        } else {
            // Just VAR format - export existing variable
            char *value = getenv(arg);
            if (value == NULL) {
                // Set empty if doesn't exist
                if (setenv(arg, "", 1) != 0) {
                    perror("conch: export");
                }
            }
        }
    }

    return 1;
}
