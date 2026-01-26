#ifndef PARSER_H
#define PARSER_H

// Read a line of input from stdin
char *read_line(void);

// Split line into tokens, handling quotes
char **split_line(char *line);

// Expand environment variables ($VAR and ${VAR})
char *expand_variables(const char *input);

#endif // PARSER_H
