#ifdef ARDUINO
    #include <Arduino.h>
#else
    #include <cstddef>
    #include "Mocks.hpp"
#endif

#include "MonotonicTime.hpp"
#include "TaskScheduler.hpp"

namespace Util {

TaskScheduler::TaskScheduler()
: tasks_(nullptr) {
}

void TaskScheduler::addTask(Task* task) {
    if(tasks_)
        tasks_->insert(task);
    else
        tasks_ = task;
}

void TaskScheduler::run() {
    // FIXME: time limit
    while(tasks_ && MonotonicTime::now() <= MonotonicTime(0, 300)) {
        Task* task = tasks_;

        do {
            auto now = MonotonicTime::now();

            do {
                if(task->scheduledAt_ > now) {
                    task = task->next();
                    continue;
                }

                task->execute();
                task = task->next();
                break;
            } while(task);
        } while(task);

        delay(1);
    }
}

void Task::scheduleAfter(MonotonicTime::Time time) {
    scheduledAt_ = MonotonicTime::now() + time;
}

}
