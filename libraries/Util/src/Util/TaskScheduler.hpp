#ifndef Util_TaskScheduler_hpp
#define Util_TaskScheduler_hpp

#include "LinkedNodeList.hpp"
#include "MonotonicTime.hpp"
#include "TypeTraits.hpp"

namespace Util {

class Task;

class TaskScheduler: TypeTraits::NonCopyable {
    public:
        TaskScheduler();
        void addTask(Task* task);
        void setMaxReactionTime(MonotonicTime::Time time);
        void run();

    private:
        Task* tasks_;
        MonotonicTime::Time maxReactionTime_;
};

class Task: public LinkedNodeList<Task> {
    public:
        void scheduleAfter(MonotonicTime::Time time);

    private:
        virtual void execute() = 0;

    private:
        MonotonicTime scheduledAt_;

    friend class TaskScheduler;
};

}

#endif
