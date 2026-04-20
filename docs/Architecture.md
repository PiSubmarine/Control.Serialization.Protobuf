# Control.Serialization.Protobuf

`PiSubmarine.Control.Serialization.Protobuf` implements
`Control.Serialization.Api` using protobuf as the on-wire format.

## Responsibility

This module owns:

- protobuf schema for `Control.Api::Input::OperatorCommand`
- serializer implementation
- deserializer implementation

It does not own:

- UDP transport
- control command production
- lease validation
