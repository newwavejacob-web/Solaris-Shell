#ifndef BUILTINS_H
#define BUILTINS_H

// Built-in command names
extern char *builtin_names[];

// Built-in command function pointers
extern int (*builtin_functions[]) (char **);

// Get number of built-in commands
int num_builtins(void);

// Built-in command implementations
int builtin_cd(char **args);
int builtin_help(char **args);
int builtin_exit(char **args);
int builtin_clear(char **args);
int builtin_status(char **args);

#endif // BUILTINS_H
