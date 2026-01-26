#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
#include <termios.h>

// Maximum number of jobs
#define MAX_JOBS 64

// Job states
typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} job_state_t;

// Job structure
typedef struct {
    int id;                     // Job ID
    pid_t pgid;                 // Process group ID
    char *command;              // Command string
    job_state_t state;          // Current state
    int background;             // Running in background?
    int notified;               // Has user been notified of state change?
} job_t;

// Global shell state
extern pid_t shell_pgid;
extern struct termios shell_tmodes;
extern int shell_terminal;
extern int shell_is_interactive;

// Job table
extern job_t *jobs[MAX_JOBS];
extern int next_job_id;

// Initialize job control
void jobs_init(void);

// Add a new job
int job_add(pid_t pgid, const char *command, int background);

// Remove a job
void job_remove(int job_id);

// Find job by process group ID
job_t *job_find_by_pgid(pid_t pgid);

// Find job by job ID
job_t *job_find_by_id(int job_id);

// Update job state
void job_set_state(job_t *job, job_state_t state);

// Check for and handle completed/stopped jobs
void job_check_status(void);

// Wait for a job (foreground)
void job_wait(job_t *job);

// Continue a job in foreground
void job_foreground(job_t *job, int cont);

// Continue a job in background
void job_background(job_t *job, int cont);

// Print job status
void job_print(job_t *job);

// Print all jobs
void jobs_list(void);

// Signal handlers
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);

// Setup signal handlers
void setup_signal_handlers(void);

// Put shell in foreground
void shell_foreground(void);

#endif // JOBS_H
