#define BOOST_TEST_MODULE MonotonicTime
#include <boost/test/included/unit_test.hpp>

#include <Util/MonotonicTime.hpp>

#include "mocks.hpp"

using Util::MonotonicTime;

BOOST_AUTO_TEST_CASE(mocking) {
    BOOST_REQUIRE(MonotonicTime::now() == MonotonicTime(0, 0));

    MonotonicTime mockedTime = MonotonicTime(111, 222);
    Mocks::setMonotonicTime(mockedTime);
    BOOST_REQUIRE(MonotonicTime::now() == mockedTime);
}

BOOST_AUTO_TEST_CASE(comparsion) {
    BOOST_REQUIRE(MonotonicTime(0, 0) == MonotonicTime(0, 0));
    BOOST_REQUIRE(MonotonicTime(0, 0) != MonotonicTime(0, 1));
    BOOST_REQUIRE(MonotonicTime(0, 0) != MonotonicTime(1, 0));
}
