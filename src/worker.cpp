#include "worker.hpp"

#include <thread>

void Worker::task() {
    while (!sync->stopThreads) {
        while (!sync->pauseThreads) {
            std::unique_lock<std::mutex> uniqueLock(sync->tasksMutex);
            // W8 for our turn
            sync->cvTasks.wait(uniqueLock, [this]{ return !sync->queueInUse; });
            // sync->cvTasks.wait(uniqueLock);

            Task task;
            bool taskExists = false;

            if (!sync->queueInUse) {
                sync->queueInUse = true;
                
                if (!data->tasks.empty()) {
                    sync->workDone[id] = 0;

                    task = data->tasks.front();
                    taskExists = true;
                    
                    data->tasks.pop();
                }
                else {
                    sync->workDone[id] = 1;
                    sync->queueInUse = false;
                    sync->noTasks.notify_one();
                    continue;
                }

                sync->queueInUse = false;
            }
            
            sync->cvTasks.notify_one();

            // Do work here - seperate function?
            if (taskExists) {
                for (short x = task.coordinates[0]; 
                    x <= task.coordinates[2]; x++) {
                    int left = x - 1;
                    int right = x + 1;

                    if (x == 0)
                        left = std::get<0>(task.currentGen->dimensions()) - 1;
                    else if (x == std::get<0>(task.currentGen->dimensions()) - 1)
                        right = 0;

                    for (short y = task.coordinates[1];
                        y <= task.coordinates[3]; y++) {
                        // Edges
                        int bottom = y + 1;
                        int top = y - 1;

                        if (y == 0)
                            top = std::get<1>(task.currentGen->dimensions()) - 1;
                        else if (y == std::get<1>(task.currentGen->dimensions()) - 1)
                            bottom = 0;

                        // Count alive neighbours
                        short aliveCount = (*data->currentGen.get())(left, top);
                        aliveCount += (*data->currentGen.get())(x, top);
                        aliveCount += (*data->currentGen.get())(right, top);

                        aliveCount += (*data->currentGen.get())(left, y);
                        aliveCount += (*data->currentGen.get())(right, y);

                        aliveCount += (*data->currentGen.get())(left, bottom);
                        aliveCount += (*data->currentGen.get())(x, bottom);
                        aliveCount += (*data->currentGen.get())(right, bottom);

                        // If alive
                        if ((*data->currentGen.get())(x, y) == 1) {
                            if (aliveCount < 2)
                                (*data->nextGen.get())(x,y) = 0;
                            else if (aliveCount > 3)
                                (*data->nextGen.get())(x,y) = 0;
                            else
                                (*data->nextGen.get())(x,y) = 1;
                        }
                        // If dead
                        else {
                            if (aliveCount == 3)
                                (*data->nextGen.get())(x,y) = 1;
                        }
                    }
                }
            }
        }

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}