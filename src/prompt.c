#include "../include/common.h"
#include "../include/prompt.h"

void print_boot_sequence(void) {
    printf(GREEN);
    printf(" ██████╗ ██████╗ ███╗   ██╗ ██████╗██╗  ██╗\n");
    printf("██╔════╝██╔═══██╗████╗  ██║██╔════╝██║  ██║\n");
    printf("██║     ██║   ██║██╔██╗ ██║██║     ███████║\n");
    printf("██║     ██║   ██║██║╚██╗██║██║     ██╔══██║\n");
    printf("╚██████╗╚██████╔╝██║ ╚████║╚██████╗██║  ██║\n");
    printf(" ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚═╝  ╚═╝\n\n");
    printf("CONCH SHELL\n");
    printf("ENTERING SHELL\n");
    printf("INITIALIZING COMMAND INTERFACE...\n");
    printf("LOADING SYSTEM DIAGNOSTICS...\n");
    usleep(500000); // 0.5 second delay
    printf("WELCOME TO CONCH\n");
    printf("COPYRIGHT 2025 SOLARIS\n");
    printf(RESET);
    printf("\n");
}

void print_prompt(void) {
    time_t rawtime;
    struct tm *timeinfo;
    char time_buffer[20];
    char *cwd;
    char *basename_cwd;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", timeinfo);

    cwd = getcwd(NULL, 0);
    if (cwd != NULL) {
        basename_cwd = strrchr(cwd, '/');
        if (basename_cwd != NULL) {
            basename_cwd++;
        } else {
            basename_cwd = cwd;
        }

        printf(BRIGHT_GREEN "[%s]" RESET " " YELLOW "%s" RESET " " CYAN "CONCH>" RESET " ",
               time_buffer, basename_cwd);
        free(cwd);
    } else {
        printf(BRIGHT_GREEN "[%s]" RESET " " CYAN "CONCH>" RESET " ", time_buffer);
    }

    fflush(stdout);
}
