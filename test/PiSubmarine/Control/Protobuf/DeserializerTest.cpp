#include <array>

#include <gtest/gtest.h>

#include "PiSubmarine/Control/ErrorCode.h"
#include "PiSubmarine/Control/Protobuf/Deserializer.h"

namespace PiSubmarine::Control::Protobuf
{
    TEST(DeserializerTest, RejectsInvalidPayload)
    {
        Deserializer deserializer;
        const std::array<std::byte, 3> bytes{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};

        const auto result = deserializer.Deserialize(std::span<const std::byte>(bytes));
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().Cause, make_error_code(ErrorCode::DeserializationFailed));
    }
}
