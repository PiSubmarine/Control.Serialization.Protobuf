#pragma once

#include "PiSubmarine/Control/ISerializer.h"

namespace PiSubmarine::Control::Protobuf
{
    class Serializer final : public ISerializer
    {
    public:
        [[nodiscard]] Error::Api::Result<std::vector<std::byte>> Serialize(
            const Api::Input::OperatorCommand& command) const override;
    };
}
