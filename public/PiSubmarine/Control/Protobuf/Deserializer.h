#pragma once

#include "PiSubmarine/Control/IDeserializer.h"

namespace PiSubmarine::Control::Protobuf
{
    class Deserializer final : public IDeserializer
    {
    public:
        [[nodiscard]] Error::Api::Result<Api::Input::OperatorCommand> Deserialize(
            std::span<const std::byte> bytes) const override;
    };
}
