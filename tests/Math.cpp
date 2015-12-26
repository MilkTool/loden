#include "Loden/Math.hpp"
#include "UnitTest++/UnitTest++.h"

using namespace Loden;

SUITE(Math)
{
    TEST(CloseTo)
    {
        CHECK(closeTo(1.0, 1.0));
        CHECK(!closeTo(0.0, 1.0));
        CHECK(!closeTo(1.0, 0.0));
        CHECK(closeTo(0.9 + 0.1, 1.0));
    }
}
