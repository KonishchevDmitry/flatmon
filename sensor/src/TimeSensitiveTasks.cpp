#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

#include "TimeSensitiveTasks.hpp"

using Util::Task;

Task* TimeSensitiveTasks::acquiredTask_ = nullptr;

bool TimeSensitiveTasks::acquire(Task* task) {
    if(acquiredTask_) {
        return acquiredTask_ == task;
    } else {
        acquiredTask_ = task;
        return true;
    }
}

void TimeSensitiveTasks::release(Task* task) {
    if(acquiredTask_ == task)
        acquiredTask_ = nullptr;
}
