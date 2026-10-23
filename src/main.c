#include "../include/common.h"
#include "../include/prompt.h"
#include "../include/parser.h"
#include "../include/executor.h"
#include "../include/history.h"
#include "../include/completion.h"
#include "../include/jobs.h"
#include <termios.h>
#include <ctype.h>

#define KEY_UP 65
#define KEY_DOWN 66
#define KEY_RIGHT 67
#define KEY_LEFT 68
#define KEY_TAB 9
#define KEY_BACKSPACE 127
#define KEY_CTRL_D 4
#define KEY_CTRL_C 3

static struct termios orig_termios;
static int raw_mode_enabled = 0;

// Enable raw mode for character-by-character input
void enable_raw_mode(void) {
    if (raw_mode_enabled) return;

    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_mode_enabled = 1;
}

// Disable raw mode
void disable_raw_mode(void) {
    if (!raw_mode_enabled) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    raw_mode_enabled = 0;
}

// Read line with history and tab completion support
char *read_line_advanced(void) {
    int buffer_size = BUFFER_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffer_size);
    char c;

    if (!buffer) {
        fprintf(stderr, "conch: MEMORY ALLOCATION FAILED\n");
        exit(EXIT_FAILURE);
    }

    buffer[0] = '\0';
    enable_raw_mode();

    while (1) {
        if (read(STDIN_FILENO, &c, 1) != 1) break;

        // Handle escape sequences (arrow keys)
        if (c == 27) {
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;

            if (seq[0] == '[') {
                if (seq[1] == KEY_UP) {
                    // Up arrow - previous history
                    char *hist = history_prev();
                    if (hist != NULL) {
                        // Clear current line
                        printf("\r");
                        print_prompt();
                        for (int i = 0; i < position; i++) printf(" ");
                        printf("\r");
                        print_prompt();

                        // Copy history to buffer
                        strcpy(buffer, hist);
                        position = strlen(buffer);
                        printf("%s", buffer);
                        fflush(stdout);
                    }
                } else if (seq[1] == KEY_DOWN) {
                    // Down arrow - next history
                    char *hist = history_next();

                    // Clear current line
                    printf("\r");
                    print_prompt();
                    for (int i = 0; i < position; i++) printf(" ");
                    printf("\r");
                    print_prompt();

                    if (hist != NULL) {
                        strcpy(buffer, hist);
                        position = strlen(buffer);
                        printf("%s", buffer);
                    } else {
                        buffer[0] = '\0';
                        position = 0;
                    }
                    fflush(stdout);
                }
            }
        } else if (c == KEY_TAB) {
            // Tab completion
            buffer[position] = '\0';
            char **matches = get_completions(buffer, 0, position);

            if (matches[0] != NULL) {
                if (matches[1] == NULL) {
                    // Single match - auto-complete
                    printf("\r");
                    print_prompt();
                    for (int i = 0; i < position; i++) printf(" ");
                    printf("\r");
                    print_prompt();

                    strcpy(buffer, matches[0]);
                    position = strlen(buffer);
                    printf("%s", buffer);
                    fflush(stdout);
                } else {
                    // Multiple matches - show them
                    printf("\n");
                    for (int i = 0; matches[i] != NULL && i < 20; i++) {
                        printf("  %s", matches[i]);
                        if ((i + 1) % 4 == 0) printf("\n");
                    }
                    if (matches[0] != NULL) printf("\n");

                    print_prompt();
                    printf("%s", buffer);
                    fflush(stdout);
                }
            }
            completion_cleanup();
        } else if (c == '\n' || c == '\r') {
            // Enter key
            printf("\n");
            buffer[position] = '\0';
            disable_raw_mode();
            return buffer;
        } else if (c == KEY_BACKSPACE) {
            // Backspace
            if (position > 0) {
                position--;
                buffer[position] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
        } else if (c == KEY_CTRL_D) {
            // Ctrl+D - EOF
            if (position == 0) {
                printf("\n");
                free(buffer);
                disable_raw_mode();
                return NULL;
            }
        } else if (c == KEY_CTRL_C) {
            // Ctrl+C - interrupt
            printf("^C\n");
            buffer[0] = '\0';
            position = 0;
            print_prompt();
            fflush(stdout);
        } else if (isprint(c)) {
            // Regular printable character
            buffer[position] = c;
            position++;

            if (position >= buffer_size) {
                buffer_size += BUFFER_SIZE;
                buffer = realloc(buffer, buffer_size);
                if (!buffer) {
                    fprintf(stderr, "conch: MEMORY REALLOCATION FAILED\n");
                    exit(EXIT_FAILURE);
                }
            }

            printf("%c", c);
            fflush(stdout);
        }
    }

    // read() hit EOF (e.g. piped input ran out)
    disable_raw_mode();
    if (position == 0) {
        free(buffer);
        return NULL;
    }
    buffer[position] = '\0';
    return buffer;
}

int main(void) {
    char *line;
    char **args;
    int status = 1;

    // Initialize systems
    jobs_init();
    history_init();
    completion_init();

    print_boot_sequence();

    // Main Shell REPL Loop
    while (status) {
        print_prompt();
        history_reset_nav();

        line = read_line_advanced();

        if (line == NULL) {
            // EOF (Ctrl+D)
            printf(GREEN "CONCH SESSION TERMINATED\n" RESET);
            break;
        }

        if (strlen(line) > 0) {
            history_add(line);

            // Expand environment variables
            char *expanded = expand_variables(line);
            if (expanded) {
                args = split_line(expanded);
                status = execute_command(args);
                free(args);
                free(expanded);
            }
        }

        free(line);
    }

    // Cleanup
    history_cleanup();
    completion_cleanup();

    return EXIT_SUCCESS;
}
