#include "../include/common.h"
#include "../include/parser.h"

char *read_line(void) {
    int buffer_size = BUFFER_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffer_size);
    int c;

    if (!buffer) {
        fprintf(stderr, "TERMINAL: MEMORY ALLOCATION FAILED\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }

        position++;

        if (position >= buffer_size) {
            buffer_size += BUFFER_SIZE;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                fprintf(stderr, "TERMINAL: MEMORY REALLOCATION FAILED\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **split_line(char *line) {
    int buffer_size = TOKEN_SIZE;
    int position = 0;
    char **tokens = malloc(buffer_size * sizeof(char*));
    char *token_start;
    int i = 0;
    char quote_char = '\0';

    if (!tokens) {
        fprintf(stderr, "TERMINAL: TOKEN ALLOCATION FAILED\n");
        exit(EXIT_FAILURE);
    }

    while (line[i] && (line[i] == ' ' || line[i] == '\t')) {
        i++;
    }

    while (line[i] != '\0') {
        token_start = &line[i];

        // Quoted strings
        if (line[i] == '"' || line[i] == '\'') {
            quote_char = line[i];
            i++; // Skip opening quote
            token_start = &line[i]; // Start token after quote

            while (line[i] != '\0' && line[i] != quote_char) {
                i++;
            }

            if (line[i] == quote_char) {
                line[i] = '\0'; // Null terminate the token
                i++;
            }
        } else {
            // Regular token - find next whitespace
            while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t') {
                i++;
            }

            if (line[i] != '\0') {
                line[i] = '\0'; // Null terminate
                i++;
            }
        }

        tokens[position] = token_start;
        position++;

        if (position >= buffer_size) {
            buffer_size += TOKEN_SIZE;
            tokens = realloc(tokens, buffer_size * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "TERMINAL: TOKEN REALLOCATION FAILED\n");
                exit(EXIT_FAILURE);
            }
        }

        while (line[i] && (line[i] == ' ' || line[i] == '\t')) {
            i++;
        }
    }

    tokens[position] = NULL;
    return tokens;
}
