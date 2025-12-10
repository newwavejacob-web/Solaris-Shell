#ifndef PARSER_H
#define PARSER_H

// Read a line of input from stdin
char *read_line(void);

// Split line into tokens, handling quotes
char **split_line(char *line);

#endif // PARSER_H
