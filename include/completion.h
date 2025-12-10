#ifndef COMPLETION_H
#define COMPLETION_H

// Initialize readline for tab completion
void completion_init(void);

// Get matches for tab completion
char **get_completions(const char *text, int start, int end);

// Command name completion generator
char *command_generator(const char *text, int state);

// Cleanup completion resources
void completion_cleanup(void);

#endif // COMPLETION_H
