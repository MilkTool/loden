#include "Loden/JSONSerializator.hpp"

namespace Loden
{

JSONSerializator::JSONSerializator()
{
    currentState = WriteState::None;
}

JSONSerializator::~JSONSerializator()
{
}

Serializator &JSONSerializator::writeInt8(int8_t value)
{
    return *this;
}

Serializator &JSONSerializator::writeInt16(int16_t value)
{
    return *this;
}

Serializator &JSONSerializator::writeInt32(int32_t value)
{
    return *this;
}

Serializator &JSONSerializator::writeInt64(int64_t value)
{
    return *this;
}

Serializator &JSONSerializator::writeUInt8(uint8_t value)
{
    return *this;
}

Serializator &JSONSerializator::writeUInt16(uint16_t value)
{
    return *this;
}

Serializator &JSONSerializator::writeUInt32(uint32_t value)
{
    return *this;
}

Serializator &JSONSerializator::writeUInt64(uint64_t value)
{
    return *this;
}


Serializator &JSONSerializator::writeFloat32(float value)
{
    return *this;
}

Serializator &JSONSerializator::writeFloat64(double value)
{
    return *this;
}


Serializator &JSONSerializator::writeString(const std::string &value)
{
    return *this;
}

Serializator &JSONSerializator::writeObject(const SerializablePtr &object)
{
    return *this;
}

Serializator &JSONSerializator::setKey(const std::string &name)
{
    return *this;
}

Serializator &JSONSerializator::beginArray(size_t size)
{
    return *this;
}

Serializator &JSONSerializator::endArray()
{
    return *this;
}

Serializator &JSONSerializator::beginStringDictionary(size_t size)
{
    return *this;
}

Serializator &JSONSerializator::endStringDictionary()
{
    return *this;
}

Serializator &JSONSerializator::beginDictionary(size_t size)
{
    return *this;
}

Serializator &JSONSerializator::endDictionary()
{
    return *this;
}


} // End of namespace Loden
