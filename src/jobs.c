#include "../include/common.h"
#include "../include/jobs.h"
#include <signal.h>
#include <errno.h>

// Global shell state
pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;

// Job table
job_t *jobs[MAX_JOBS];
int next_job_id = 1;

// Initialize job control
void jobs_init(void) {
    shell_terminal = STDIN_FILENO;
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        // Wait until we are in the foreground
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp())) {
            kill(-shell_pgid, SIGTTIN);
        }

        // Ignore interactive and job-control signals
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        // Put ourselves in our own process group
        shell_pgid = getpid();
        if (setpgid(shell_pgid, shell_pgid) < 0) {
            perror("TERMINAL: setpgid");
            exit(1);
        }

        // Grab control of the terminal
        tcsetpgrp(shell_terminal, shell_pgid);

        // Save default terminal attributes
        tcgetattr(shell_terminal, &shell_tmodes);
    }

    // Initialize job table
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i] = NULL;
    }
    next_job_id = 1;

    // Setup signal handlers
    setup_signal_handlers();
}

// Setup signal handlers
void setup_signal_handlers(void) {
    struct sigaction sa;

    // SIGCHLD handler - to reap background processes
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

// SIGCHLD handler
void sigchld_handler(int sig) {
    (void)sig;
    int status;
    pid_t pid;

    // Reap all available zombie children
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        // Find the job for this process
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i] != NULL && jobs[i]->pgid == pid) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    jobs[i]->state = JOB_DONE;
                    jobs[i]->notified = 0;
                } else if (WIFSTOPPED(status)) {
                    jobs[i]->state = JOB_STOPPED;
                    jobs[i]->notified = 0;
                } else if (WIFCONTINUED(status)) {
                    jobs[i]->state = JOB_RUNNING;
                }
                break;
            }
        }
    }
}

// Add a new job
int job_add(pid_t pgid, const char *command, int background) {
    // Find empty slot
    int slot = -1;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i] == NULL) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fprintf(stderr, "TERMINAL: Too many jobs\n");
        return -1;
    }

    job_t *job = malloc(sizeof(job_t));
    if (!job) {
        perror("TERMINAL: malloc");
        return -1;
    }

    job->id = next_job_id++;
    job->pgid = pgid;
    job->command = strdup(command);
    job->state = JOB_RUNNING;
    job->background = background;
    job->notified = 0;

    jobs[slot] = job;
    return job->id;
}

// Remove a job
void job_remove(int job_id) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i] != NULL && jobs[i]->id == job_id) {
            free(jobs[i]->command);
            free(jobs[i]);
            jobs[i] = NULL;
            return;
        }
    }
}

// Find job by process group ID
job_t *job_find_by_pgid(pid_t pgid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i] != NULL && jobs[i]->pgid == pgid) {
            return jobs[i];
        }
    }
    return NULL;
}

// Find job by job ID
job_t *job_find_by_id(int job_id) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i] != NULL && jobs[i]->id == job_id) {
            return jobs[i];
        }
    }
    return NULL;
}

// Update job state
void job_set_state(job_t *job, job_state_t state) {
    if (job) {
        job->state = state;
        job->notified = 0;
    }
}

// Check for and report completed/stopped jobs
void job_check_status(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i] != NULL && !jobs[i]->notified) {
            if (jobs[i]->state == JOB_DONE) {
                printf("[%d]  Done                    %s\n",
                       jobs[i]->id, jobs[i]->command);
                job_remove(jobs[i]->id);
            } else if (jobs[i]->state == JOB_STOPPED && jobs[i]->background) {
                printf("[%d]+ Stopped                 %s\n",
                       jobs[i]->id, jobs[i]->command);
                jobs[i]->notified = 1;
            }
        }
    }
}

// Wait for a foreground job
void job_wait(job_t *job) {
    int status;
    pid_t pid;

    do {
        pid = waitpid(-job->pgid, &status, WUNTRACED);
    } while (pid > 0 && !WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));

    if (WIFSTOPPED(status)) {
        job->state = JOB_STOPPED;
        job->background = 1;
        printf("\n[%d]+ Stopped                 %s\n", job->id, job->command);
    } else {
        job_remove(job->id);
    }
}

// Continue a job in foreground
void job_foreground(job_t *job, int cont) {
    // Put job in foreground
    tcsetpgrp(shell_terminal, job->pgid);

    // Send SIGCONT if needed
    if (cont) {
        if (kill(-job->pgid, SIGCONT) < 0) {
            perror("TERMINAL: kill (SIGCONT)");
        }
        job->state = JOB_RUNNING;
    }

    // Wait for job
    job_wait(job);

    // Put shell back in foreground
    tcsetpgrp(shell_terminal, shell_pgid);

    // Restore shell terminal modes
    tcsetattr(shell_terminal, TCSADRAIN, &shell_tmodes);
}

// Continue a job in background
void job_background(job_t *job, int cont) {
    job->background = 1;

    if (cont) {
        if (kill(-job->pgid, SIGCONT) < 0) {
            perror("TERMINAL: kill (SIGCONT)");
        }
        job->state = JOB_RUNNING;
    }

    printf("[%d]+ %s &\n", job->id, job->command);
}

// Print job status
void job_print(job_t *job) {
    const char *state_str;
    switch (job->state) {
        case JOB_RUNNING: state_str = "Running"; break;
        case JOB_STOPPED: state_str = "Stopped"; break;
        case JOB_DONE: state_str = "Done"; break;
        default: state_str = "Unknown"; break;
    }
    printf("[%d]%c %s                 %s%s\n",
           job->id,
           '+',  // Most recent job
           state_str,
           job->command,
           job->background ? " &" : "");
}

// Print all jobs
void jobs_list(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i] != NULL && jobs[i]->state != JOB_DONE) {
            job_print(jobs[i]);
        }
    }
}

// Put shell in foreground
void shell_foreground(void) {
    tcsetpgrp(shell_terminal, shell_pgid);
}
