#define BOOST_TEST_MODULE LinkedNodeList
#include <boost/test/included/unit_test.hpp>

#include <cstddef>
#include <vector>

#include <Util/LinkedNodeList.hpp>

using namespace Util;

struct Node: public LinkedNodeList<Node> {
    int value;

    Node(int value): value(value) {
    }
};

std::vector<int> traverse(Node* node) {
    std::vector<int> values;

    while(node) {
        values.push_back(node->value);
        node = node->next();
    }

    return values;
}

void compare(Node* nodes, const std::vector<int>& expected) {
    std::vector<int> list = traverse(nodes);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(list.begin(), list.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(insert_remove) {
    Node one(1), two(2), three(3), four(4);

    Node* nodes = &one;
    Node* next;

    one.insert(&two);
    two.insert(&four);
    two.insert(&three);
    compare(nodes, {1, 2, 3, 4});

    BOOST_REQUIRE(!three.remove(&next));
    BOOST_REQUIRE(next == &four);
    compare(nodes, {1, 2, 4});

    BOOST_REQUIRE(!four.remove(&next));
    BOOST_REQUIRE(next == nullptr);
    compare(nodes, {1, 2});

    BOOST_REQUIRE(one.remove(&next));
    BOOST_REQUIRE(next == &two);
    nodes = next;
    compare(nodes, {2});
}
