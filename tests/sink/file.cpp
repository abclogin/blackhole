#include <system_error>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/file.hpp>
#include <blackhole/detail/sink/file.hpp>

#include "mocks/node.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace {

TEST(backend_t, ThrowsIfUnableToOpenStream) {
    EXPECT_THROW(backend_t("/__mythic/file.log", 1), std::system_error);
}

}  // namespace
}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole

namespace blackhole {
namespace testing {
namespace sink {

using ::blackhole::sink::file_t;

using ::testing::Return;
using ::testing::StrictMock;

namespace file {

using ::blackhole::sink::file::inner_t;

TEST(inner_t, IntervalSanitizer) {
    inner_t inner("", 0);

    EXPECT_NE(0, inner.interval());
    EXPECT_EQ(std::numeric_limits<std::size_t>::max(), inner.interval());
}

TEST(inner_t, IntervalOverflow) {
    std::size_t interval = 3;
    std::size_t counter = 0;

    counter = (counter + 1) % interval;
    EXPECT_EQ(1, counter);

    counter = (counter + 1) % interval;
    EXPECT_EQ(2, counter);

    counter = (counter + 1) % interval;
    EXPECT_EQ(0, counter);
}

}  // namespace file

TEST(file_t, Type) {
    EXPECT_EQ("file", std::string(blackhole::factory<file_t>::type()));
}

TEST(file_t, FromRequiresFilename) {
    StrictMock<config::testing::mock::node_t> config;

    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(nullptr));

    EXPECT_THROW(factory<sink::file_t>::from(config), std::invalid_argument);
}

TEST(file_t, PatternFromConfig) {
    using config::testing::mock::node_t;

    StrictMock<config::testing::mock::node_t> config;

    auto npath = new node_t;

    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(npath));

    EXPECT_CALL(*npath, to_string())
        .Times(1)
        .WillOnce(Return("/tmp/blackhole.log"));

    EXPECT_CALL(config, subscript_key("flush"))
        .Times(1)
        .WillOnce(Return(nullptr));

    EXPECT_EQ("/tmp/blackhole.log", factory<sink::file_t>::from(config).path());
}

TEST(file_t, FlushIntervalFromConfig) {
    using config::testing::mock::node_t;

    StrictMock<config::testing::mock::node_t> config;

    auto npath = new node_t;
    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(npath));

    EXPECT_CALL(*npath, to_string())
        .Times(1)
        .WillOnce(Return("/tmp/blackhole.log"));

    auto nflush = new node_t;
    EXPECT_CALL(config, subscript_key("flush"))
        .Times(1)
        .WillOnce(Return(nflush));

    EXPECT_CALL(*nflush, to_uint64())
        .Times(1)
        .WillOnce(Return(30));

    factory<sink::file_t>::from(config);
}

}  // namespace sink
}  // namespace testing
}  // namespace blackhole
