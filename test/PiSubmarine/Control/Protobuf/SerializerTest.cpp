#include <gtest/gtest.h>

#include "PiSubmarine/Control/Api/Input/Mode/Request.h"
#include "PiSubmarine/Control/Protobuf/Deserializer.h"
#include "PiSubmarine/Control/Protobuf/Serializer.h"

namespace PiSubmarine::Control::Protobuf
{
    TEST(SerializerTest, RoundTripsCompleteOperatorCommand)
    {
        Api::Input::OperatorCommand command{};
        command.LeaseId = Lease::Api::LeaseId{"lease-123"};
        command.Movement = Horizontal::Api::Command::Create(0.5, -0.25).value();
        command.VerticalControl = Vertical::Api::Command::SetDepthTargetTo(12.5_m);
        command.GimbalTarget = Gimbal::Api::Command::Create(Radians{0.2});
        command.LampIntensity = Lamp::Api::Command::Create(NormalizedFraction{0.7});
        command.VideoControl = Video::Api::Command::Enable(
            Video::Api::StreamProfile::HighQuality,
            Video::Api::ManualFocus{NormalizedFraction{0.3}});
        command.ModeRequest = Api::Input::Mode::Request::HoldPositionValue();

        Serializer serializer;
        Deserializer deserializer;

        const auto serializeResult = serializer.Serialize(command);
        ASSERT_TRUE(serializeResult.has_value());
        EXPECT_FALSE(serializeResult->empty());

        const auto deserializeResult = deserializer.Deserialize(*serializeResult);
        ASSERT_TRUE(deserializeResult.has_value());
        EXPECT_EQ(*deserializeResult, command);
    }
}
