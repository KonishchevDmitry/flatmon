#define BOOST_TEST_MODULE TaskScheduler
#include <boost/test/included/unit_test.hpp>

#include <vector>

#include <Util/TaskScheduler.hpp>

using namespace Util;

struct TestTask: public Task {
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

void compare(const TestTask& task, const std::vector<MonotonicTime::Time>& times) {
    BOOST_REQUIRE_EQUAL_COLLECTIONS(task.times.begin(), task.times.end(), times.begin(), times.end());
}

BOOST_AUTO_TEST_CASE(scheduling) {
    TaskScheduler scheduler;

    TestTask task100(100);
    scheduler.addTask(&task100);

    TestTask task250(250);
    scheduler.addTask(&task250);

    scheduler.run();

    compare(task100, {0, 100, 200, 300});
    compare(task250, {0, 250});
}
