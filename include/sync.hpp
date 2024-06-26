#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <vector>

struct Sync {
    // Tasks for threads
    std::mutex tasksMutex;
    std::condition_variable cvTasks;
    std::condition_variable noTasks;
    // Other threads
    std::atomic_bool stopThreads = ATOMIC_VAR_INIT(false);
    std::atomic_bool pauseThreads = ATOMIC_VAR_INIT(false);

    std::vector<short> workDone;

    Sync() {}
};