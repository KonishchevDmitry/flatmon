#ifndef ARDUINO
    #include "Mocks.hpp"
#endif

#include "Core.hpp"
#include "MonotonicTime.hpp"
#include "TaskScheduler.hpp"

namespace Util {

TaskScheduler::TaskScheduler()
: tasks_(nullptr), executing_task_(nullptr), executing_task_next_(nullptr) {
}

TaskScheduler::~TaskScheduler() {
    while(tasks_)
        tasks_->remove();
}

void TaskScheduler::addTask(Task* task) {
    UTIL_ASSERT(!task->scheduler_);

    if(tasks_)
        tasks_->insert(task);
    else
        tasks_ = task;

    task->scheduler_ = this;
}

void TaskScheduler::run() {
    MonotonicTime::Time maxDelay = 0;
    maxDelay -= 1;
    UTIL_ASSERT(maxDelay > 0);

    while(tasks_) {
        Task* task = tasks_;
        bool executed = false;

        auto now = MonotonicTime::now();
        MonotonicTime awakeTime = now + maxDelay;

        do {
            if(task->paused()) {
                task = task->next();
            } else if(task->scheduledAt_ > now) {
                awakeTime = min(awakeTime, task->scheduledAt_);
                task = task->next();
            } else {
                // Attention: Tasks may be added/removed during task execution

                executing_task_ = task;
                task->execute();
                executed = true;

                if(executing_task_) {
                    // The task hasn't been removed during execution
                    executing_task_ = nullptr;
                    task = task->next();
                } else {
                    // The task has been removed during execution
                    task = executing_task_next_;
                    executing_task_next_ = nullptr;
                }

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

Task::Task(): paused_(false), scheduler_(nullptr) {
}

Task::~Task() {
    this->remove();
}

void Task::scheduleAfter(MonotonicTime::Time time) {
    scheduledAt_ = MonotonicTime::now() + time;
}

bool Task::isTimedOut(MonotonicTime::Time time) {
    auto now = MonotonicTime::now();
    return now >= scheduledAt_ && now - scheduledAt_ >= time;
}

void Task::pause() {
    paused_ = true;
}

bool Task::paused() {
    return paused_;
}

void Task::resume() {
    paused_ = false;
}

void Task::remove() {
    if(!this->scheduler_)
        return;

    Task* next;
    if(LinkedNodeList::remove(&next))
        this->scheduler_->tasks_ = next;

    // Notify scheduler about task removing
    if(this == this->scheduler_->executing_task_ || this == this->scheduler_->executing_task_next_) {
        this->scheduler_->executing_task_ = nullptr;
        this->scheduler_->executing_task_next_ = next;
    }

    this->scheduler_ = nullptr;
}

}
