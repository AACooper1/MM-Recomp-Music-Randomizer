#ifndef THREAD_H
#define THREAD_H

#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include "globals.h"

#include <stdarg.h>

#define JOB_MSG_BUFFER_SIZE 256
#define MAX_THREADS 16
#define MAX_JOBS 16

typedef enum ThreadState_t {
    THREAD_UNSTARTED,
    THREAD_RUNNING,
    THREAD_IDLE,
    THREAD_ERROR,
    THREAD_FATAL,
    THREAD_KILL
} ThreadState;

typedef enum JobState_t {
    JOB_UNSTARTED,
    JOB_RUNNING,
    JOB_DONE,
    JOB_WAIT,   // Wait for user input
    JOB_ERROR,
    JOB_KILL
} JobState;

typedef struct Job_t {
    int jobId;
    int threadId;

    JobState state;
    char msg[JOB_MSG_BUFFER_SIZE];
    int result;

    int (*func)(va_list); // Func must be a DLL func
} Job;

typedef struct Thread_t {
    int id;
    ThreadState state;

    int numJobs;
    int lastJob;
    Job* jobs[MAX_JOBS];
} Thread;

int numThreads;
Thread* threads[MAX_THREADS];

RECOMP_IMPORT(".", int music_rando_send_thread_msg(int jobId, ThreadState state));
RECOMP_IMPORT(".", int music_rando_poll_thread(int jobId, char* msg));
RECOMP_IMPORT(".", void music_rando_cleanup_thread(int jobId));

// Thread API
int thread_create(int (*func)(va_list));
int thread_destroy(int threadId);
Thread* thread_get(int threadId);
int thread_set_state(int thread, ThreadState state);

// Thread internals
int _thread_get_next_job(int threadId);

// Job API
int job_create(int threadId, int (*func)(va_list), ...);
int job_destroy(int threadId, int jobId);
Job* job_get(int threadId, int jobId);

int job_run(int threadId, int jobId, ...);
int job_set_state(int threadId, int jobId, JobState state);

// Job internals


#endif