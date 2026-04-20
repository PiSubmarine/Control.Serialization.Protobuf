#include "PiSubmarine/Control/Protobuf/Deserializer.h"

#include <string>

#include "OperatorCommand.pb.h"
#include "PiSubmarine/Control/ErrorCode.h"
#include "PiSubmarine/Error/Api/MakeError.h"

namespace PiSubmarine::Control::Protobuf
{
    namespace
    {
        [[nodiscard]] Error::Api::Error MakeControlError(const ErrorCode code)
        {
            return Error::Api::MakeError(Error::Api::ErrorCondition::ContractError, make_error_code(code));
        }

        [[nodiscard]] Error::Api::Result<Horizontal::Api::Command> DeserializeMovement(
            const ::pisubmarine::control::protobuf::HorizontalCommand& protoMovement)
        {
            const auto movementResult = Horizontal::Api::Command::Create(protoMovement.surge(), protoMovement.yaw());
            if (!movementResult.has_value())
            {
                return std::unexpected(MakeControlError(ErrorCode::InvalidPayload));
            }

            return *movementResult;
        }

        [[nodiscard]] Vertical::Api::Command DeserializeVertical(
            const ::pisubmarine::control::protobuf::VerticalCommand& protoVertical)
        {
            switch (protoVertical.value_case())
            {
            case ::pisubmarine::control::protobuf::VerticalCommand::kDepthMeters:
                return Vertical::Api::Command::SetDepthTargetTo(Meters{protoVertical.depth_meters()});
            case ::pisubmarine::control::protobuf::VerticalCommand::kBallastPosition:
                return Vertical::Api::Command::SetBallastPositionTo(NormalizedFraction{protoVertical.ballast_position()});
            case ::pisubmarine::control::protobuf::VerticalCommand::kKeepCurrent:
            case ::pisubmarine::control::protobuf::VerticalCommand::VALUE_NOT_SET:
                return Vertical::Api::Command::KeepCurrentValue();
            }

            return Vertical::Api::Command::KeepCurrentValue();
        }

        [[nodiscard]] Video::Api::StreamProfile FromProto(
            const ::pisubmarine::control::protobuf::EnabledVideo_StreamProfile profile)
        {
            switch (profile)
            {
            case ::pisubmarine::control::protobuf::EnabledVideo_StreamProfile_LOW_LATENCY:
                return Video::Api::StreamProfile::LowLatency;
            case ::pisubmarine::control::protobuf::EnabledVideo_StreamProfile_STANDARD:
                return Video::Api::StreamProfile::Standard;
            case ::pisubmarine::control::protobuf::EnabledVideo_StreamProfile_HIGH_QUALITY:
                return Video::Api::StreamProfile::HighQuality;
            }

            return Video::Api::StreamProfile::Standard;
        }

        [[nodiscard]] Video::Api::FocusMode DeserializeFocus(
            const ::pisubmarine::control::protobuf::EnabledVideo& protoEnabled)
        {
            switch (protoEnabled.focus_case())
            {
            case ::pisubmarine::control::protobuf::EnabledVideo::kManualFocus:
                return Video::Api::ManualFocus{NormalizedFraction{protoEnabled.manual_focus().position()}};
            case ::pisubmarine::control::protobuf::EnabledVideo::kAutoFocus:
            case ::pisubmarine::control::protobuf::EnabledVideo::FOCUS_NOT_SET:
                return Video::Api::AutoFocus{};
            }

            return Video::Api::AutoFocus{};
        }
    }

    Error::Api::Result<Api::Input::OperatorCommand> Deserializer::Deserialize(std::span<const std::byte> bytes) const
    {
        ::pisubmarine::control::protobuf::OperatorCommand protoCommand;
        if (!protoCommand.ParseFromArray(reinterpret_cast<const char*>(bytes.data()), static_cast<int>(bytes.size())))
        {
            return std::unexpected(MakeControlError(ErrorCode::DeserializationFailed));
        }

        Api::Input::OperatorCommand command{};
        command.LeaseId = Lease::Api::LeaseId{protoCommand.lease_id()};

        if (protoCommand.has_movement())
        {
            const auto movementResult = DeserializeMovement(protoCommand.movement());
            if (!movementResult.has_value())
            {
                return std::unexpected(movementResult.error());
            }

            command.Movement = *movementResult;
        }

        if (protoCommand.has_vertical_control())
        {
            try
            {
                command.VerticalControl = DeserializeVertical(protoCommand.vertical_control());
            }
            catch (...)
            {
                return std::unexpected(MakeControlError(ErrorCode::InvalidPayload));
            }
        }

        if (protoCommand.has_gimbal_target())
        {
            command.GimbalTarget = Gimbal::Api::Command::Create(Radians{protoCommand.gimbal_target().pitch_radians()});
        }

        if (protoCommand.has_lamp_intensity())
        {
            try
            {
                command.LampIntensity = Lamp::Api::Command::Create(
                    NormalizedFraction{protoCommand.lamp_intensity().intensity()});
            }
            catch (...)
            {
                return std::unexpected(MakeControlError(ErrorCode::InvalidPayload));
            }
        }

        if (protoCommand.has_video_control())
        {
            try
            {
                const auto& protoVideo = protoCommand.video_control();
                switch (protoVideo.state_case())
                {
                case ::pisubmarine::control::protobuf::VideoCommand::kEnabled:
                    command.VideoControl = Video::Api::Command::Enable(
                        FromProto(protoVideo.enabled().profile()),
                        DeserializeFocus(protoVideo.enabled()));
                    break;
                case ::pisubmarine::control::protobuf::VideoCommand::kDisabled:
                case ::pisubmarine::control::protobuf::VideoCommand::STATE_NOT_SET:
                    command.VideoControl = Video::Api::Command::Disable();
                    break;
                }
            }
            catch (...)
            {
                return std::unexpected(MakeControlError(ErrorCode::InvalidPayload));
            }
        }

        if (protoCommand.has_mode_request())
        {
            const auto& protoModeRequest = protoCommand.mode_request();
            switch (protoModeRequest.value_case())
            {
            case ::pisubmarine::control::protobuf::ModeRequest::kManual:
                command.ModeRequest = Api::Input::Mode::Request::ManualValue();
                break;
            case ::pisubmarine::control::protobuf::ModeRequest::kHoldPosition:
                command.ModeRequest = Api::Input::Mode::Request::HoldPositionValue();
                break;
            case ::pisubmarine::control::protobuf::ModeRequest::VALUE_NOT_SET:
                break;
            }
        }

        return command;
    }
}
