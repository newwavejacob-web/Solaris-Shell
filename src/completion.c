#include "../include/common.h"
#include "../include/completion.h"
#include <dirent.h>
#include <sys/stat.h>

#define MAX_COMPLETIONS 256

static char *completions[MAX_COMPLETIONS];
static int completion_count = 0;

// Common commands to suggest
static const char *common_commands[] = {
    "ls", "cd", "pwd", "cat", "grep", "find", "mkdir", "rm", "cp", "mv",
    "echo", "touch", "chmod", "chown", "ps", "kill", "top", "man", "help",
    "clear", "exit", "status", "vim", "nano", "git", "make", "gcc", "python",
    NULL
};

void completion_init(void) {
    for (int i = 0; i < MAX_COMPLETIONS; i++) {
        completions[i] = NULL;
    }
    completion_count = 0;
}

static void add_completion(const char *match) {
    if (completion_count < MAX_COMPLETIONS) {
        completions[completion_count] = strdup(match);
        completion_count++;
    }
}

static int is_executable(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return (st.st_mode & S_IXUSR) != 0;
    }
    return 0;
}

// Get command completions from PATH
static void get_path_commands(const char *prefix, int prefix_len) {
    char *path_env = getenv("PATH");
    if (!path_env) return;

    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");

    while (dir != NULL && completion_count < MAX_COMPLETIONS) {
        DIR *d = opendir(dir);
        if (d) {
            struct dirent *entry;
            while ((entry = readdir(d)) != NULL && completion_count < MAX_COMPLETIONS) {
                if (strncmp(entry->d_name, prefix, prefix_len) == 0) {
                    char full_path[1024];
                    snprintf(full_path, sizeof(full_path), "%s/%s", dir, entry->d_name);
                    if (is_executable(full_path)) {
                        add_completion(entry->d_name);
                    }
                }
            }
            closedir(d);
        }
        dir = strtok(NULL, ":");
    }

    free(path_copy);
}

// Get file/directory completions
static void get_file_completions(const char *prefix) {
    char dir_path[1024] = ".";
    char file_prefix[256] = "";
    int prefix_len;

    // Split path and filename
    const char *last_slash = strrchr(prefix, '/');
    if (last_slash != NULL) {
        int dir_len = last_slash - prefix;
        if (dir_len == 0) {
            strcpy(dir_path, "/");
        } else {
            strncpy(dir_path, prefix, dir_len);
            dir_path[dir_len] = '\0';
        }
        strcpy(file_prefix, last_slash + 1);
    } else {
        strcpy(file_prefix, prefix);
    }

    prefix_len = strlen(file_prefix);

    DIR *d = opendir(dir_path);
    if (!d) return;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL && completion_count < MAX_COMPLETIONS) {
        if (entry->d_name[0] == '.' && file_prefix[0] != '.') {
            continue; // Skip hidden files unless explicitly requested
        }

        if (strncmp(entry->d_name, file_prefix, prefix_len) == 0) {
            char full_match[1024];
            if (strcmp(dir_path, ".") == 0) {
                snprintf(full_match, sizeof(full_match), "%s", entry->d_name);
            } else {
                snprintf(full_match, sizeof(full_match), "%s/%s", dir_path, entry->d_name);
            }

            // Add trailing slash for directories
            struct stat st;
            if (stat(full_match, &st) == 0 && S_ISDIR(st.st_mode)) {
                strcat(full_match, "/");
            }

            add_completion(full_match);
        }
    }

    closedir(d);
}

char **get_completions(const char *text, int start, int end) {
    (void)end;
    completion_count = 0;

    if (start == 0) {
        // Command completion
        int text_len = strlen(text);

        // Check common commands first
        for (int i = 0; common_commands[i] != NULL; i++) {
            if (strncmp(common_commands[i], text, text_len) == 0) {
                add_completion(common_commands[i]);
            }
        }

        // Check PATH commands
        if (completion_count < 10) { // Limit PATH search if we already have matches
            get_path_commands(text, text_len);
        }
    } else {
        // File/path completion
        get_file_completions(text);
    }

    // Return NULL-terminated array
    completions[completion_count] = NULL;
    return completions;
}

char *command_generator(const char *text, int state) {
    static int list_index, len;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while (common_commands[list_index] != NULL) {
        const char *name = common_commands[list_index];
        list_index++;

        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}

void completion_cleanup(void) {
    for (int i = 0; i < completion_count; i++) {
        if (completions[i] != NULL) {
            free(completions[i]);
            completions[i] = NULL;
        }
    }
    completion_count = 0;
}
