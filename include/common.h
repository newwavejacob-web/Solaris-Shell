#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

// Buffer sizes
#define BUFFER_SIZE 1024
#define TOKEN_SIZE 64

// ANSI Color codes for terminal aesthetic
#define GREEN "\033[32m"
#define BRIGHT_GREEN "\033[1;32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define BOLD "\033[1m"

#endif // COMMON_H
