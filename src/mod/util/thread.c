#include "thread.h"

int thread_create(int (*func)(va_list), ...)
{
    if (numThreads >= MAX_THREADS)
    {
        logger.error("Tried to start a thread, but there were already %i!\n", numThreads);
        return -1;
    }

    Thread* newThread = recomp_alloc(sizeof(Thread));
    Lib_MemSet(newThread, 0, sizeof(Thread));

    int threadIdx = -1;
    for (int i = 0; i < MAX_THREADS; i++)
    {
        if (!threads[i])
        {
            threadIdx = i;
        }
    }

    if (threadIdx < 0)
    {
        logger.critical("numThreads is %i, but failed to find an empty array slot!\n", numThreads);
        return -2;
    }

    newThread->numJobs = 0;
    newThread->id = threadIdx;
    newThread->state = THREAD_UNSTARTED;

    threads[threadIdx] = newThread;

    va_list args;
    va_start(args, func);

    newThread->jobs[newThread->numJobs++] = job_create(threadIdx, func, args);

    va_end(args);

    numThreads++;
    return newThread->id;
}

int thread_destroy(int threadId)
{
    Thread* thread = thread_get(threadId);

    if (thread <= 0)
    {
        return thread;
    }

    while(thread->numJobs > 0)
    {
        int jobId = _thread_get_next_job(threadId);
        job_destroy(threadId, jobId);
    }

    thread_set_state(threadId, THREAD_KILL);
    threads[threadId] = 0;
    numThreads--;
    recomp_free(thread);

    return 0;
}

Thread* thread_get(int threadId)
{
    if (threadId >= MAX_THREADS || threadId < 0)
    {
        logger.error("Tried to access thread %i, but index was out of bounds!\n", threadId);
        return -1;
    }
    Thread* thread = threads[threadId];
    
    if (!thread)
    {
        logger.debug("Tried to access thread %i, which was %p.\n", threadId, thread);
        return 0;
    }

    return thread;
}


int _thread_get_next_job(int threadId)
{
    Thread* thread = thread_get(threadId);
    
    if (thread <= 0)
    {
        return thread;
    }

    if (thread->numJobs <= 0)
    {
        return 0;
    }

    for (int i = 0; i < 16; i++)
    {
        int jobId = thread->lastJob;
        Job* job = get_job(threadId, (++jobId + i) % MAX_JOBS);

        if (job > 0)
        {
            return job;
        }
    }
    
    return 0;
}

int thread_set_state(int threadId, ThreadState state)
{
    Thread* thread = thread_get(threadId);

    if (thread <= 0)
    {
        return thread;
    }

    return music_rando_send_thread_msg(threadId, state);
}



int job_create(int threadId, int (*func)(va_list), ...)
{
    Thread* thread = thread_get(threadId);

    if (thread == 0)
    {
        return -1;
    }

    if (thread < 0)
    {
        return thread;
    }

    if (thread->numJobs >= MAX_JOBS)
    {
        logger.error("Tried to create new job on thread %i, but it already had %i!\n", threadId, thread->numJobs);
        return -2;
    }

    Job* newJob = recomp_alloc(sizeof(Job));
    Lib_MemSet(newJob, 0, sizeof(Job));

    newJob->threadId = threadId;

    newJob->state = JOB_UNSTARTED;
    newJob->func = func;

    int jobIdx = -1;
    for (int i = 0; i < MAX_THREADS; i++)
    {
        if (!thread->jobs[i])
        {
            jobIdx = i;
        }
    }

    if (jobIdx < 0)
    {
        logger.critical("Thread %i has %i jobs, but failed to find an empty array slot!\n", threadId, thread->numJobs);
        return -2;
    }

    thread->jobs[jobIdx] =  newJob;

    thread->numJobs++;

    va_list args;
    va_start(args, func);

    newJob->state = job_run(threadId, jobIdx, func, args);

    va_end(args);

    return newJob->jobId;
}


int job_destroy(int threadId, int jobId)
{
    Thread* thread = thread_get(threadId);
    
    if (thread <= 0)
    {
        return thread;
    }
    
    Job* job = job_get(threadId, jobId);
    
    if (job <= 0)
    {
        return job;
    }
    
    if (job->state == JOB_RUNNING || job->state == JOB_WAIT)
    {
        return -8;
    }
    
    if (job_set_state(threadId, jobId, JOB_KILL))
    {
        return -16;
    }
    
    thread->jobs[job->jobId] = 0;
    thread->numJobs--;
    recomp_free(job);
    
    return 0;
}

Job* job_get(int threadId, int jobId)
{
    Thread* thread = thread_get(threadId);

    if (thread <= 0)
    {
        return thread;
    }

    if (jobId >= MAX_JOBS || jobId < 0)
    {
        logger.error("Tried to access job %i, but index was out of bounds!\n", jobId);
        return -4;
    }

    Job* job = thread->jobs[jobId];

    if (!job)
    {
        logger.dev("Tried to access %i on thread %i, but job didn't exist!\n", jobId, threadId);
        return 0;
    }

    return job;
}

int job_run(int threadId, int jobId)
{
    Thread* thread = thread_get(threadId);
    
    if (thread <= 0)
    {
        return thread;
    }

    Job* job = job_get(threadId, jobId);

    if (job <= 0)
    {
        return job;
    }

    va_list args;
    va_start(args, jobId);

    job->func(args);

    va_end(args);

    return 0;
}