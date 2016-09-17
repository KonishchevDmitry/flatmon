#ifndef TimeSensitiveTasks_hpp
#define TimeSensitiveTasks_hpp

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

class TimeSensitiveTasks {
    public:
        static bool acquire(Util::Task* task);
        static void release(Util::Task* task);

    private:
        static Util::Task* acquiredTask_;
};

#endif
