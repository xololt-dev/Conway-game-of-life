#include "worker.hpp"

#include <thread>
/*
void Worker::run() {
    while (!stopThreads) {
        while (!pauseThreads) {
            std::unique_lock<std::mutex> uniqueLock(*tasksMutex);
            // W8 for our turn
            cvTasks->wait(uniqueLock, [this]{ return !queueInUse; });

            Task task;
            bool taskExists = false;

            if (!queueInUse) {
                queueInUse = true;
                
                if (!tasks.empty()) {
                    workDone[a_id] = 0;

                    task = tasks.front();
                    taskExists = true;
                    
                    tasks.pop();
                }
                else workDone[a_id] = 1;

                queueInUse = false;
            }
            
            uniqueLock.unlock();
            cvTasks->notify_one();

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
                        short aliveCount = (*currentGen.get())(left, top);
                        aliveCount += (*currentGen.get())(x, top);
                        aliveCount += (*currentGen.get())(right, top);

                        aliveCount += (*currentGen.get())(left, y);
                        aliveCount += (*currentGen.get())(right, y);

                        aliveCount += (*currentGen.get())(left, bottom);
                        aliveCount += (*currentGen.get())(x, bottom);
                        aliveCount += (*currentGen.get())(right, bottom);

                        // If alive
                        if ((*currentGen.get())(x, y) == 1) {
                            if (aliveCount < 2)
                                (*nextGen.get())(x,y) = 0;
                            else if (aliveCount > 3)
                                (*nextGen.get())(x,y) = 0;
                            else
                                (*nextGen.get())(x,y) = 1;
                        }
                        // If dead
                        else {
                            if (aliveCount == 3)
                                (*nextGen.get())(x,y) = 1;
                        }
                    }
                }
            }
        }

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
*/