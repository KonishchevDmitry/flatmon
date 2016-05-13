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
: tasks_(nullptr), maxReactionTime_(0) {
    // Use maximum available reaction time by default
    maxReactionTime_ -= 1;
    UTIL_ASSERT(maxReactionTime_ > 0);
}

void TaskScheduler::addTask(Task* task) {
    if(tasks_)
        tasks_->insert(task);
    else
        tasks_ = task;
}

void TaskScheduler::setMaxReactionTime(MonotonicTime::Time time) {
    maxReactionTime_ = time;
}

void TaskScheduler::run() {
    // FIXME: time limit
    while(tasks_ && MonotonicTime::now() <= MonotonicTime(0, 300)) {
        Task* task = tasks_;
        bool executed = false;

        auto now = MonotonicTime::now();
        MonotonicTime awakeTime = now + maxReactionTime_;

        do {
            if(task->scheduledAt_ > now) {
                awakeTime = min(awakeTime, task->scheduledAt_);
                task = task->next();
            } else {
                task->execute();
                executed = true;
                task = task->next();
                now = MonotonicTime::now();
            }
        } while(task);

        // If some task has been executed it might changed other tasks' schedule time
        if(!executed) {
            auto delayTime = awakeTime - now;
            UTIL_ASSERT(delayTime.epoch == 0);
            delay(delayTime.time);
        }
    }
}

void Task::scheduleAfter(MonotonicTime::Time time) {
    scheduledAt_ = MonotonicTime::now() + time;
}

}
