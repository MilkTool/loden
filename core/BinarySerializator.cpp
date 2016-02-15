#include "Loden/BinarySerializator.hpp"
#include "Loden/Serializable.hpp"
#include <assert.h>

namespace Loden
{

BinarySerializator::BinarySerializator()
{
    outputFile = nullptr;
    serializedObjectCount = 0;
}

BinarySerializator::~BinarySerializator()
{
    if (outputFile)
        close();
}

bool BinarySerializator::open(const std::string &filename)
{
    assert(!outputFile);
    outputFile = fopen(filename.c_str(), "wb");
    if (!outputFile)
        return false;
    return true;
}

void BinarySerializator::close()
{
    if(outputFile)
        fclose(outputFile);
}

Serializator &BinarySerializator::writeInt8(int8_t value)
{
    assert(outputFile);
    fwrite(&value, 1, 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeInt16(int16_t value)
{
    assert(outputFile);
    fwrite(&value, 2, 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeInt32(int32_t value)
{
    assert(outputFile);
    fwrite(&value, 4, 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeInt64(int64_t value)
{
    assert(outputFile);
    fwrite(&value, 8, 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeUInt8(uint8_t value)
{
    assert(outputFile);
    fwrite(&value, 1, 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeUInt16(uint16_t value)
{
    assert(outputFile);
    fwrite(&value, 2, 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeUInt32(uint32_t value)
{
    assert(outputFile);
    fwrite(&value, 4, 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeUInt64(uint64_t value)
{
    assert(outputFile);
    fwrite(&value, 8, 1, outputFile);
    return *this;
}


Serializator &BinarySerializator::writeFloat32(float value)
{
    assert(outputFile);
    fwrite(&value, 4, 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeFloat64(double value)
{
    assert(outputFile);
    fwrite(&value, 8, 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeString(const std::string &value)
{
    writeUInt32((uint32_t)value.size());
    fwrite(value.data(), value.size(), 1, outputFile);
    return *this;
}

Serializator &BinarySerializator::writeObject(const SerializablePtr &object)
{
    // Was this object already written?
    auto canonicalPointer = object->getCanonicalPointer();
    auto objectIdIt = serializedObjectIds.find(canonicalPointer);
    if (objectIdIt != serializedObjectIds.end())
    {
        auto id = objectIdIt->second;
        writeUInt32(id);
        return *this;
    }

    // Generate the new object id
    auto newId = serializedObjectCount++;
    serializedObjectIds[canonicalPointer] = newId;
    writeUInt32(0xFFFFFFFF);

    // Write the object content.
    object->serialize(*this);
    return *this;
}

Serializator &BinarySerializator::setKey(const std::string &name)
{
    // Do nothing, for now.
    return *this;
}

Serializator &BinarySerializator::beginArray(size_t size)
{
    writeUInt32((uint32_t)size);
    return *this;
}

Serializator &BinarySerializator::endArray()
{
    // Do nothing
    return *this;
}

Serializator &BinarySerializator::beginStringDictionary(size_t size)
{
    writeUInt32((uint32_t)size);
    return *this;
}

Serializator &BinarySerializator::endStringDictionary()
{
    return *this;
}

Serializator &BinarySerializator::beginDictionary(size_t size)
{
    writeUInt32((uint32_t)size);
    return *this;
}

Serializator &BinarySerializator::endDictionary()
{
    return *this;
}


} // End of namespace Loden
