#include "Mocks.hpp"

using Util::MonotonicTime;

namespace {
    MonotonicTime CUR_TIME = MonotonicTime(0, 0);
}

namespace Mocks {
    void setMonotonicTime(MonotonicTime time) {
        CUR_TIME = time;
    }
}

namespace Util {
    MonotonicTime MonotonicTime::now() {
        return CUR_TIME;
    }
}
