#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_SIZE 100

// Initialize history system
void history_init(void);

// Add command to history
void history_add(const char *line);

// Get command from history by index (0 = most recent)
char *history_get(int index);

// Get previous command in history
char *history_prev(void);

// Get next command in history
char *history_next(void);

// Reset history navigation
void history_reset_nav(void);

// Get current history count
int history_count(void);

// Clean up history
void history_cleanup(void);

#endif // HISTORY_H
