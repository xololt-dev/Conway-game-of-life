#pragma once

#include <memory>
#include <thread>

#include "sync.hpp"
#include "data.hpp"

class Worker {
public:
    void run();
    /*
    Worker(const int a_id, std::shared_ptr<Data> a_data,
        std::mutex& a_tasksMutex, std::condition_variable& a_cvTasks) :
        id(a_id), currentGen(a_data->currentGen), nextGen(a_data->nextGen),
        tasks(std::make_shared<std::queue<Task>>(a_data->tasks)),
        tasksMutex(std::make_shared<std::mutex>(a_tasksMutex)),
        cvTasks(std::make_shared<std::condition_variable>(a_cvTasks)) {
        thread = std::thread(&Worker::run, this);

        thread.detach();
    }
    */

    Worker(const int a_id, std::shared_ptr<Data> a_data, std::shared_ptr<Sync> a_sync) :
        id(a_id), data(a_data), sync(a_sync) {
        thread = std::thread(&Worker::run, this);

        thread.detach();
    }

private:
    const int id;
    std::thread thread;

    std::shared_ptr<Data> data;
    std::shared_ptr<Sync> sync;
};