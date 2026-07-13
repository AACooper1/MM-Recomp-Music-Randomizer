#ifndef THREAD_H
#define THREAD_H

#include "lib_recomp.hpp"

#include "logging.hpp"

#include <unordered_map>
#include <chrono>
#include <thread>
#include <future>

enum class ThreadState
{
    UNSTARTED,
    RUNNING,
    DONE,
    ERROR
};

class MusicRandoJob
{
    public:
        MusicRandoJob() : state(ThreadState::UNSTARTED) {}
        ~MusicRandoJob() 
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        void start(std::function<int()> func) 
        {
            state.store(ThreadState::RUNNING);
            thread = std::thread(&MusicRandoJob::run, this, func);
            return;
        }
        MusicRandoJob(const MusicRandoJob&) = delete;
        MusicRandoJob& operator=(const MusicRandoJob&) = delete;

        ThreadState poll() const { return state.load(); }
    private:
        void run(std::function<int()> func)
        {
            try
            {
                result = func();
                state.store(ThreadState::DONE);
            }
            catch (...)
            {
                state.store(ThreadState::ERROR);
            }
        }

        std::thread thread;
        int result = 0;
        std::atomic<ThreadState> state = ThreadState::UNSTARTED;
};

int test_async();
int music_rando_create_thread(std::function<int()> func);

#endif