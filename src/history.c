#include "../include/common.h"
#include "../include/history.h"

static char *history[HISTORY_SIZE];
static int history_index = 0;
static int history_size = 0;
static int nav_position = -1;

void history_init(void) {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i] = NULL;
    }
    history_index = 0;
    history_size = 0;
    nav_position = -1;
}

void history_add(const char *line) {
    if (line == NULL || strlen(line) == 0) {
        return;
    }

    // Don't add duplicate consecutive commands
    if (history_size > 0 && history[(history_index - 1 + HISTORY_SIZE) % HISTORY_SIZE] != NULL) {
        if (strcmp(history[(history_index - 1 + HISTORY_SIZE) % HISTORY_SIZE], line) == 0) {
            return;
        }
    }

    // Free old entry if we're overwriting
    if (history[history_index] != NULL) {
        free(history[history_index]);
    }

    // Add new entry
    history[history_index] = strdup(line);
    history_index = (history_index + 1) % HISTORY_SIZE;

    if (history_size < HISTORY_SIZE) {
        history_size++;
    }

    nav_position = -1; // Reset navigation
}

char *history_get(int index) {
    if (index < 0 || index >= history_size) {
        return NULL;
    }

    int actual_index = (history_index - 1 - index + HISTORY_SIZE) % HISTORY_SIZE;
    return history[actual_index];
}

char *history_prev(void) {
    if (history_size == 0) {
        return NULL;
    }

    if (nav_position < history_size - 1) {
        nav_position++;
    }

    return history_get(nav_position);
}

char *history_next(void) {
    if (nav_position <= 0) {
        nav_position = -1;
        return NULL;
    }

    nav_position--;
    return history_get(nav_position);
}

void history_reset_nav(void) {
    nav_position = -1;
}

int history_count(void) {
    return history_size;
}

void history_cleanup(void) {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (history[i] != NULL) {
            free(history[i]);
            history[i] = NULL;
        }
    }
}
