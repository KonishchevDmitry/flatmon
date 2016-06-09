#define BOOST_TEST_MODULE TaskScheduler
#include <boost/test/included/unit_test.hpp>

#include <vector>

#include <Util/TaskScheduler.hpp>

using namespace Util;

struct TestTask: Task {
    MonotonicTime::Time interval;
    std::vector<MonotonicTime::Time> times;

    TestTask(MonotonicTime::Time interval)
    : interval(interval) {
    }

    virtual void execute() {
        times.push_back(MonotonicTime::now().time);
        scheduleAfter(interval);
    }
};

struct TestTask100: TestTask {
    TestTask100(): TestTask(100) {
    }

    virtual void execute() {
        TestTask::execute();

        auto now = MonotonicTime::now();
        if(now.time == 200)
            this->pause();
        else if(now.time >= 600)
            this->remove();
    }
};

struct TestTask150: TestTask {
    TestTask150(): TestTask(150) {
    }

    virtual void execute() {
        TestTask::execute();
        if(MonotonicTime::now().time >= 600)
            this->remove();
    }
};

struct TestTask200: TestTask {
    Task* slaveTask;

    TestTask200(Task* slaveTask): TestTask(200), slaveTask(slaveTask) {
    }

    virtual void execute() {
        TestTask::execute();

        if(MonotonicTime::now().time >= 400) {
            this->remove();
            this->slaveTask->scheduleAfter(100);
            this->slaveTask->resume();
        }
    }
};

void compare(const TestTask& task, const std::vector<MonotonicTime::Time>& times) {
    BOOST_REQUIRE_EQUAL_COLLECTIONS(task.times.begin(), task.times.end(), times.begin(), times.end());
}

BOOST_AUTO_TEST_CASE(scheduling) {
    TaskScheduler scheduler;

    TestTask100 task100;
    scheduler.addTask(&task100);

    TestTask150 task150;
    scheduler.addTask(&task150);

    TestTask200 task200(&task100);
    scheduler.addTask(&task200);

    scheduler.run();

    compare(task100, {0, 100, 200, 500, 600});
    compare(task150, {0, 150, 300, 450, 600});
    compare(task200, {0, 200, 400});
}
