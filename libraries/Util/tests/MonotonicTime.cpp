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
    auto maxTime = std::numeric_limits<MonotonicTime::Time>::max();

    BOOST_REQUIRE(MonotonicTime(0, 0) + 100 == MonotonicTime(0, 100));
    BOOST_REQUIRE(MonotonicTime(1, 0) + 100 == MonotonicTime(1, 100));

    BOOST_REQUIRE(MonotonicTime(0, maxTime) + 0 == MonotonicTime(0, maxTime));
    BOOST_REQUIRE(MonotonicTime(0, maxTime) + 1 == MonotonicTime(1, 0));
    BOOST_REQUIRE(MonotonicTime(0, maxTime) + maxTime == MonotonicTime(1, maxTime - 1));

    BOOST_REQUIRE(MonotonicTime(1, 1) + MonotonicTime(1, 2) == MonotonicTime(2, 3));
    BOOST_REQUIRE(MonotonicTime(1, 2) + MonotonicTime(1, maxTime) == MonotonicTime(3, 1));
}

BOOST_AUTO_TEST_CASE(subtraction) {
    auto maxTime = std::numeric_limits<MonotonicTime::Time>::max();

    BOOST_REQUIRE(MonotonicTime(2, 10) - 6 == MonotonicTime(2, 4));
    BOOST_REQUIRE(MonotonicTime(2, 10) - 10 == MonotonicTime(2, 0));
    BOOST_REQUIRE(MonotonicTime(2, 10) - 11 == MonotonicTime(1, maxTime));

    BOOST_REQUIRE(MonotonicTime(3, 10) - MonotonicTime(2, 6) == MonotonicTime(1, 4));
    BOOST_REQUIRE(MonotonicTime(3, 10) - MonotonicTime(2, 12) == MonotonicTime(0, maxTime - 1));
}
