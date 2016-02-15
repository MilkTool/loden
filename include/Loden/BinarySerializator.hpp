#ifndef LODEN_BINARY_SERIALIZATOR_HPP
#define LODEN_BINARY_SERIALIZATOR_HPP

#include "Loden/Serializator.hpp"
#include <stdio.h>

namespace Loden
{

LODEN_DECLARE_CLASS(BinarySerializator);

/**
* Binary serializator
*/
class BinarySerializator : public ObjectSubclass<BinarySerializator, Serializator>
{
    LODEN_OBJECT_TYPE(BinarySerializator)
public:
    BinarySerializator();
    ~BinarySerializator();

    bool open(const std::string &filename);
    void close();

    virtual Serializator &writeInt8(int8_t value) override;
    virtual Serializator &writeInt16(int16_t value) override;
    virtual Serializator &writeInt32(int32_t value) override;
    virtual Serializator &writeInt64(int64_t value) override;

    virtual Serializator &writeUInt8(uint8_t value) override;
    virtual Serializator &writeUInt16(uint16_t value) override;
    virtual Serializator &writeUInt32(uint32_t value) override;
    virtual Serializator &writeUInt64(uint64_t value) override;

    virtual Serializator &writeFloat32(float value) override;
    virtual Serializator &writeFloat64(double value) override;

    virtual Serializator &writeString(const std::string &value) override;
    virtual Serializator &writeObject(const SerializablePtr &object) override;

    virtual Serializator &setKey(const std::string &name) override;
    virtual Serializator &beginArray(size_t size) override;
    virtual Serializator &endArray() override;

    virtual Serializator &beginStringDictionary(size_t size) override;
    virtual Serializator &endStringDictionary() override;

    virtual Serializator &beginDictionary(size_t size) override;
    virtual Serializator &endDictionary() override;

private:
    FILE *outputFile;
    uint32_t serializedObjectCount;
    std::unordered_map<const void *, uint32_t> serializedObjectIds;
};

} // End of namespace Loden

#endif //LODEN_BINARY_SERIALIZATOR_HPP