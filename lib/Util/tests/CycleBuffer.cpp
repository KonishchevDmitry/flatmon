#define BOOST_TEST_MODULE CycleBuffer
#include <boost/test/included/unit_test.hpp>

#include <cstring>

#include <Util/CycleBuffer.hpp>

using Util::NumericCycleBuffer;

template<typename T, size_t N>
struct TestCycleBuffer: NumericCycleBuffer<T, N> {
    TestCycleBuffer() {
        this->start_ = 2;
    }

    T* data() {
        return this->items_;
    }
};

template<typename B>
void compare(B& buffer, const typename B::ValueType* expected) {
    BOOST_REQUIRE(!memcmp(buffer.data(), expected, buffer.capacity() * sizeof(typename B::ValueType)));
}

BOOST_AUTO_TEST_CASE(addAndMedian) {
    const size_t size = 5;
    int expected[size];
    TestCycleBuffer<int, size> buffer;

    BOOST_REQUIRE_EQUAL(buffer.capacity(), 5);
    BOOST_REQUIRE_EQUAL(buffer.size(), 0);

    memset(expected, 0, sizeof expected);
    memset(buffer.data(), 0, buffer.capacity() * sizeof(int));
    compare(buffer, expected);

    buffer.add(10);
    expected[2] = 10;
    compare(buffer, expected);
    BOOST_REQUIRE_EQUAL(buffer.size(), 1);
    BOOST_REQUIRE_EQUAL(buffer.median(), 10);

    buffer.add(7);
    expected[3] = 7;
    compare(buffer, expected);
    BOOST_REQUIRE_EQUAL(buffer.size(), 2);
    BOOST_REQUIRE_EQUAL(buffer.median(), 8);

    buffer.add(1);
    expected[4] = 1;
    compare(buffer, expected);
    BOOST_REQUIRE_EQUAL(buffer.size(), 3);
    BOOST_REQUIRE_EQUAL(buffer.median(), 7);

    buffer.add(3);
    expected[0] = 3;
    compare(buffer, expected);
    BOOST_REQUIRE_EQUAL(buffer.size(), 4);
    BOOST_REQUIRE_EQUAL(buffer.median(), 5);

    buffer.add(9);
    expected[1] = 9;
    compare(buffer, expected);
    BOOST_REQUIRE_EQUAL(buffer.size(), 5);
    BOOST_REQUIRE_EQUAL(buffer.median(), 7);

    buffer.add(2);
    expected[2] = 2;
    compare(buffer, expected);
    BOOST_REQUIRE_EQUAL(buffer.size(), 5);
    BOOST_REQUIRE_EQUAL(buffer.median(), 3);

    buffer.add(12);
    expected[3] = 12;
    compare(buffer, expected);
    BOOST_REQUIRE_EQUAL(buffer.size(), 5);
    BOOST_REQUIRE_EQUAL(buffer.median(), 3);
}

BOOST_AUTO_TEST_CASE(get) {
    TestCycleBuffer<int, 5> buffer;

    buffer.add(10);
    buffer.add(20);
    buffer.add(30);
    buffer.add(40);
    buffer.add(50);

    BOOST_REQUIRE_EQUAL(buffer.size(), 5);
    BOOST_REQUIRE_EQUAL(buffer[0], 10);
    BOOST_REQUIRE_EQUAL(buffer[1], 20);
    BOOST_REQUIRE_EQUAL(buffer[2], 30);
    BOOST_REQUIRE_EQUAL(buffer[3], 40);
    BOOST_REQUIRE_EQUAL(buffer[4], 50);

    buffer.add(60);

    BOOST_REQUIRE_EQUAL(buffer.size(), 5);
    BOOST_REQUIRE_EQUAL(buffer[0], 20);
    BOOST_REQUIRE_EQUAL(buffer[1], 30);
    BOOST_REQUIRE_EQUAL(buffer[2], 40);
    BOOST_REQUIRE_EQUAL(buffer[3], 50);
    BOOST_REQUIRE_EQUAL(buffer[4], 60);
}

BOOST_AUTO_TEST_CASE(maxValue) {
    {
        TestCycleBuffer<int, 5> buffer;

        for(int value = 0; value < buffer.capacity(); value += 10) {
            buffer.add(value);
            BOOST_REQUIRE_EQUAL(buffer.maxValue(), value);
        }
    }

    {
        TestCycleBuffer<int, 5> buffer;
        int maxValue = (buffer.capacity() - 1) * 10;

        for(int value = maxValue; value >= 0; value -= 10) {
            buffer.add(value);
            BOOST_REQUIRE_EQUAL(buffer.maxValue(), maxValue);
        }
    }
}
