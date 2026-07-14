#ifndef THREAD_H
#define THREAD_H

#include "lib_recomp.hpp"

#include "logging.hpp"

#include <unordered_map>
#include <queue>
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

class StatusMessage
{
    public:
        void update(std::string msg)
        {
            std::lock_guard<std::mutex> lock(mutex);
            _msg = msg;
        }

        std::string& get()
        {
            std::lock_guard<std::mutex> lock(mutex);
            return _msg;
        }
    private:
        std::mutex mutex;
        std::string _msg = "Starting Thread...";
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
            else
            {
                logger.dev.disable_header();
                logger.dev << "Thread was not joinable!" << std::endl;
                logger.dev.enable_header();
            }
        }

        template <typename Func, typename... Args>
        void start(Func&& func, Args&&... args) 
        {
            state.store(ThreadState::RUNNING);
            thread = std::thread(
                [this](auto&& func, auto&&... args) {
                    run(std::forward<decltype(func)>(func), std::forward<decltype(args)>(args)...);
                },
                std::forward<Func>(func), std::forward<Args>(args)...
            );
            return;
        }
        MusicRandoJob(const MusicRandoJob&) = delete;
        MusicRandoJob& operator=(const MusicRandoJob&) = delete;

        ThreadState poll(char* msg_addr) 
        {
            char temp[256] = {0};
            strncpy_s(temp, msg.get().c_str(), 255);
            for (int i = 0; i < 256; i++)
            {
                msg_addr[i] = temp[i ^ 3];
            }
            msg_addr[255] = '\0';

            return state.load(); 
        }
    private:
        template <typename Func, typename... Args>
        void run(Func&& func, Args&&... args)
        {
            try
            {
                result = func(msg, std::forward<Args>(args)...);
                state.store(ThreadState::DONE);
            }
            catch (...)
            {
                state.store(ThreadState::ERROR);
            }
        }

        std::thread thread;
        StatusMessage msg;
        int result = 0;
        std::atomic<ThreadState> state = ThreadState::UNSTARTED;
};

extern std::unordered_map<int, std::unique_ptr<MusicRandoJob>> jobs;

template <typename Func, typename... Args>
int music_rando_create_thread(Func&& func, Args&&... args)
{
    int jobId = 0;
    while(jobs.contains(jobId))
    {
        jobId++;
    }

    jobs.emplace(jobId, std::make_unique<MusicRandoJob>());

    jobs[jobId]->start(std::forward<Func>(func), std::forward<Args>(args)...);

    return jobId;
}
#endif