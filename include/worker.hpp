#pragma once

#include <memory>
#include <thread>

#include "sync.hpp"
#include "data.hpp"

class Worker {
public:
    void task();
    void detach() { 
        thread = std::thread(&Worker::task, this);
        thread.detach(); 
    }

    Worker(const int a_id, std::shared_ptr<Data> a_data, std::shared_ptr<Sync> a_sync) :
        id(a_id), data(a_data), sync(a_sync) {
    }

private:
    // technically should be const int, but then seems to be more sensitive to worker deletions/insertions?
    int id;
    std::thread thread;

    std::shared_ptr<Data> data;
    std::shared_ptr<Sync> sync;
};