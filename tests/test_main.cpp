#include <gtest/gtest.h>

// All tests are started from a separate function, as required by the lab.
int runTests(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
int main(int argc, char** argv) { return runTests(argc, argv); }
