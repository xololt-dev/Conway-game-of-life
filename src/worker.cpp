#include "worker.hpp"

#include <thread>

void Worker::task() {
    while (!sync->stopThreads) {
        while (!sync->pauseThreads) {
            if (failSafe)
                goto end;
            
            std::unique_lock<std::mutex> uniqueLock(sync->tasksMutex);
            // W8 for our turn
            sync->cvTasks.wait(uniqueLock);

            Task task;
            bool taskExists = false;

            if (!data->tasks.empty()) {
                sync->workDone[id] = 0;

                task = data->tasks.front();
                taskExists = true;
                
                data->tasks.pop();
            }
            else {
                sync->workDone[id] = 1;
                // sync->cvTasks.notify_one();
                sync->noTasks.notify_one();
                continue;
            }
        
            sync->cvTasks.notify_one();

            // Do work here - seperate function?
            if (taskExists) {
                Matrix<short>& matrixCurr = *data->currentGen.get();

                for (short x = task.coordinates[0]; 
                    x <= task.coordinates[2]; x++) {
                    int left = x - 1;
                    int right = x + 1;

                    if (x == 0)
                        left = std::get<0>(matrixCurr.dimensions()) - 1;
                    else if (x == std::get<0>(matrixCurr.dimensions()) - 1)
                        right = 0;

                    for (short y = task.coordinates[1];
                        y <= task.coordinates[3]; y++) {
                        // Edges
                        int bottom = y + 1;
                        int top = y - 1;

                        if (y == 0)
                            top = std::get<1>(matrixCurr.dimensions()) - 1;
                        else if (y == std::get<1>(matrixCurr.dimensions()) - 1)
                            bottom = 0;

                        // Count alive neighbours
                        Matrix<short>& matrixNext = *data->nextGen.get();
                        unsigned short aliveCount = matrixCurr(left, top);
                        aliveCount += matrixCurr(x, top);
                        aliveCount += matrixCurr(right, top);

                        aliveCount += matrixCurr(left, y);
                        aliveCount += matrixCurr(right, y);

                        aliveCount += matrixCurr(left, bottom);
                        aliveCount += matrixCurr(x, bottom);
                        aliveCount += matrixCurr(right, bottom);

                        // If alive
                        if (matrixCurr(x, y) == 1) {
                            if (aliveCount < 2 || aliveCount > 3)
                                matrixNext(x,y) = 0;
                            else
                                matrixNext(x,y) = 1;
                        }
                        // If dead
                        else if (aliveCount == 3)
                            matrixNext(x,y) = 1;
                    }
                }
            }
        }

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    end:
    sync->workDone[id] = 1;
}