#ifndef Util_TaskScheduler_hpp
#define Util_TaskScheduler_hpp

#include "Core.hpp"
#include "LinkedNodeList.hpp"
#include "MonotonicTime.hpp"
#include "TypeTraits.hpp"

namespace Util {

class Task;

class TaskScheduler: TypeTraits::NonCopyable {
    public:
        TaskScheduler();
        ~TaskScheduler();

        void addTask(Task* task);
        void setMaxReactionTime(MonotonicTime::Time time);
        void run();

    private:
        Task* tasks_;
        Task* executing_task_;
        Task* executing_task_next_;

    friend class Task;
};

class Task: public LinkedNodeList<Task> {
    public:
        Task();
        ~Task();

    public:
        virtual void execute() = 0;
        void scheduleAfter(MonotonicTime::Time time);
        void pause();
        bool paused();
        void resume();
        void remove();

    private:
        bool paused_;
        MonotonicTime scheduledAt_;
        TaskScheduler* scheduler_;

    friend class TaskScheduler;
};

}

#endif
