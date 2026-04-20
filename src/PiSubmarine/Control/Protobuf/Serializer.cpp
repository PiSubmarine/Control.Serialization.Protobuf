#include "PiSubmarine/Control/Protobuf/Serializer.h"

#include <string>

#include "OperatorCommand.pb.h"
#include "PiSubmarine/Error/Api/MakeError.h"
#include "PiSubmarine/Control/ErrorCode.h"

namespace PiSubmarine::Control::Protobuf
{
    namespace
    {
        [[nodiscard]] ::pisubmarine::control::protobuf::EnabledVideo_StreamProfile ToProto(
            const Video::Api::StreamProfile profile)
        {
            switch (profile)
            {
            case Video::Api::StreamProfile::LowLatency:
                return ::pisubmarine::control::protobuf::EnabledVideo_StreamProfile_LOW_LATENCY;
            case Video::Api::StreamProfile::Standard:
                return ::pisubmarine::control::protobuf::EnabledVideo_StreamProfile_STANDARD;
            case Video::Api::StreamProfile::HighQuality:
                return ::pisubmarine::control::protobuf::EnabledVideo_StreamProfile_HIGH_QUALITY;
            }

            return ::pisubmarine::control::protobuf::EnabledVideo_StreamProfile_STANDARD;
        }
    }

    Error::Api::Result<std::vector<std::byte>> Serializer::Serialize(const Api::Input::OperatorCommand& command) const
    {
        ::pisubmarine::control::protobuf::OperatorCommand protoCommand;

        protoCommand.set_lease_id(command.LeaseId.Value);
        protoCommand.mutable_movement()->set_surge(command.Movement.Surge());
        protoCommand.mutable_movement()->set_yaw(command.Movement.Yaw());

        auto* protoVertical = protoCommand.mutable_vertical_control();
        if (command.VerticalControl.Is<Vertical::Api::Command::KeepCurrent>())
        {
            protoVertical->set_keep_current(true);
        }
        else if (const auto* depthTarget = command.VerticalControl.TryGet<Vertical::Api::Command::SetDepthTarget>())
        {
            protoVertical->set_depth_meters(depthTarget->Depth.Value);
        }
        else if (const auto* ballastPosition =
                     command.VerticalControl.TryGet<Vertical::Api::Command::SetBallastPosition>())
        {
            protoVertical->set_ballast_position(static_cast<double>(ballastPosition->Position));
        }

        if (command.GimbalTarget.has_value())
        {
            protoCommand.mutable_gimbal_target()->set_pitch_radians(command.GimbalTarget->Pitch().Value);
        }

        if (command.LampIntensity.has_value())
        {
            protoCommand.mutable_lamp_intensity()->set_intensity(static_cast<double>(command.LampIntensity->Intensity()));
        }

        if (command.VideoControl.has_value())
        {
            auto* protoVideo = protoCommand.mutable_video_control();
            if (!command.VideoControl->IsEnabled())
            {
                protoVideo->set_disabled(true);
            }
            else
            {
                const auto& enabled = *command.VideoControl->TryGetEnabled();
                auto* protoEnabled = protoVideo->mutable_enabled();
                protoEnabled->set_profile(ToProto(enabled.Profile));

                if (std::holds_alternative<Video::Api::AutoFocus>(enabled.Focus))
                {
                    protoEnabled->mutable_auto_focus();
                }
                else if (const auto* manualFocus = std::get_if<Video::Api::ManualFocus>(&enabled.Focus))
                {
                    protoEnabled->mutable_manual_focus()->set_position(static_cast<double>(manualFocus->Position));
                }
            }
        }

        if (command.ModeRequest.has_value())
        {
            auto* protoModeRequest = protoCommand.mutable_mode_request();
            if (command.ModeRequest->Is<Api::Input::Mode::Manual>())
            {
                protoModeRequest->mutable_manual();
            }
            else if (command.ModeRequest->Is<Api::Input::Mode::HoldPosition>())
            {
                protoModeRequest->mutable_hold_position();
            }
        }

        std::string serialized;
        if (!protoCommand.SerializeToString(&serialized))
        {
            return std::unexpected(Error::Api::MakeError(
                Error::Api::ErrorCondition::DeviceError,
                make_error_code(ErrorCode::SerializationFailed)));
        }

        std::vector<std::byte> bytes;
        bytes.reserve(serialized.size());
        for (const char character : serialized)
        {
            bytes.push_back(static_cast<std::byte>(character));
        }

        return bytes;
    }
}
