#define BOOST_TEST_MODULE MonotonicTime
#include <boost/test/included/unit_test.hpp>

#include <limits>

#include <Util/Mocks.hpp>
#include <Util/MonotonicTime.hpp>

using namespace Util;

BOOST_AUTO_TEST_CASE(mocking) {
    BOOST_REQUIRE(MonotonicTime::now() == MonotonicTime(0, 0));

    Mocks::setTime(111);
    BOOST_REQUIRE(MonotonicTime::now() == MonotonicTime(0, 111));
}

BOOST_AUTO_TEST_CASE(comparsion) {
    BOOST_REQUIRE(MonotonicTime(0, 1) == MonotonicTime(0, 1));
    BOOST_REQUIRE(MonotonicTime(0, 0) != MonotonicTime(0, 1));
    BOOST_REQUIRE(MonotonicTime(0, 0) != MonotonicTime(1, 0));

    BOOST_REQUIRE(MonotonicTime(0, 1) <= MonotonicTime(0, 1));
    BOOST_REQUIRE(MonotonicTime(0, 1) >= MonotonicTime(0, 1));

    BOOST_REQUIRE(MonotonicTime(0, 1) < MonotonicTime(0, 2));
    BOOST_REQUIRE(MonotonicTime(0, 1) < MonotonicTime(1, 0));

    BOOST_REQUIRE(MonotonicTime(0, 2) > MonotonicTime(0, 1));
    BOOST_REQUIRE(MonotonicTime(1, 0) > MonotonicTime(0, 1));
}

BOOST_AUTO_TEST_CASE(adding) {
    BOOST_REQUIRE(MonotonicTime(0, 0) + 100 == MonotonicTime(0, 100));
    BOOST_REQUIRE(MonotonicTime(1, 0) + 100 == MonotonicTime(1, 100));

    auto maxTime = std::numeric_limits<MonotonicTime::Time>::max();
    BOOST_REQUIRE(MonotonicTime(0, maxTime) + 0 == MonotonicTime(0, maxTime));
    BOOST_REQUIRE(MonotonicTime(0, maxTime) + 1 == MonotonicTime(1, 0));
    BOOST_REQUIRE(MonotonicTime(0, maxTime) + maxTime == MonotonicTime(1, maxTime - 1));
}
