#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include "default_config.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    FLAGS_logtostderr = 1;
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    google::InitGoogleLogging(argv[0]);
    mooncake::init_ylt_log_level();
    const int result = RUN_ALL_TESTS();
    google::ShutdownGoogleLogging();
    return result;
}