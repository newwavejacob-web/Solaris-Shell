#include "../include/common.h"
#include "../include/parser.h"

// Expand environment variables in a string
// Handles $VAR and ${VAR} formats
char *expand_variables(const char *input) {
    if (input == NULL) return NULL;

    int capacity = BUFFER_SIZE;
    char *result = malloc(capacity);
    if (!result) return NULL;

    int result_pos = 0;
    int i = 0;

    while (input[i] != '\0') {
        // Check for $ (variable expansion)
        if (input[i] == '$') {
            i++;
            char var_name[256];
            int var_pos = 0;
            int braces = 0;

            // Check for ${VAR} format
            if (input[i] == '{') {
                braces = 1;
                i++;
            }

            // Read variable name
            while (input[i] != '\0') {
                if (braces) {
                    if (input[i] == '}') {
                        i++;
                        break;
                    }
                } else {
                    // Variable names: alphanumeric and underscore
                    if (!((input[i] >= 'a' && input[i] <= 'z') ||
                          (input[i] >= 'A' && input[i] <= 'Z') ||
                          (input[i] >= '0' && input[i] <= '9') ||
                          input[i] == '_')) {
                        break;
                    }
                }
                if (var_pos < 255) {
                    var_name[var_pos++] = input[i];
                }
                i++;
            }
            var_name[var_pos] = '\0';

            // Get environment variable value
            char *value = getenv(var_name);
            if (value != NULL) {
                int value_len = strlen(value);
                // Ensure enough space
                while (result_pos + value_len + 1 >= capacity) {
                    capacity *= 2;
                    result = realloc(result, capacity);
                    if (!result) return NULL;
                }
                strcpy(&result[result_pos], value);
                result_pos += value_len;
            }
        } else {
            // Regular character
            if (result_pos + 2 >= capacity) {
                capacity *= 2;
                result = realloc(result, capacity);
                if (!result) return NULL;
            }
            result[result_pos++] = input[i++];
        }
    }

    result[result_pos] = '\0';
    return result;
}

char *read_line(void) {
    int buffer_size = BUFFER_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffer_size);
    int c;

    if (!buffer) {
        fprintf(stderr, "conch: MEMORY ALLOCATION FAILED\n");
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
                fprintf(stderr, "conch: MEMORY REALLOCATION FAILED\n");
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
        fprintf(stderr, "conch: TOKEN ALLOCATION FAILED\n");
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
                fprintf(stderr, "conch: TOKEN REALLOCATION FAILED\n");
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
