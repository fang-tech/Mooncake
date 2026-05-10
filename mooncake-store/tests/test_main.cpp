#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "default_config.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    mooncake::init_ylt_log_level();
    return RUN_ALL_TESTS();
}