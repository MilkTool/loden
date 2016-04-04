#ifndef LODEN_SERIALIZATOR_HPP
#define LODEN_SERIALIZATOR_HPP

#include "Loden/Object.hpp"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

namespace Loden
{
LODEN_DECLARE_INTERFACE(Serializable);
LODEN_DECLARE_INTERFACE(Serializator);
LODEN_DECLARE_INTERFACE(Deserializator);

/**
 * Serializator object interface.
 */
struct Serializator : public ObjectInterfaceSubclass<Serializator, Object>
{
    LODEN_OBJECT_TYPE(Serializator);

    virtual Serializator &writeInt8(int8_t value) = 0;
    virtual Serializator &writeInt16(int16_t value) = 0;
    virtual Serializator &writeInt32(int32_t value) = 0;
    virtual Serializator &writeInt64(int64_t value) = 0;

    virtual Serializator &writeUInt8(uint8_t value) = 0;
    virtual Serializator &writeUInt16(uint16_t value) = 0;
    virtual Serializator &writeUInt32(uint32_t value) = 0;
    virtual Serializator &writeUInt64(uint64_t value) = 0;

    virtual Serializator &writeFloat32(float value) = 0;
    virtual Serializator &writeFloat64(double value) = 0;

    virtual Serializator &writeString(const std::string &value) = 0;
    virtual Serializator &writeObject(const SerializablePtr &object) = 0;

    virtual Serializator &setKey(const std::string &name) = 0;
    virtual Serializator &beginArray(size_t size) = 0;
    virtual Serializator &endArray() = 0;

    virtual Serializator &beginStringDictionary(size_t size) = 0;
    virtual Serializator &endStringDictionary() = 0;

    virtual Serializator &beginDictionary(size_t size) = 0;
    virtual Serializator &endDictionary() = 0;

    template<typename T>
    Serializator &writeKeyValue(const std::string &name, T &value)
    {
        setKey(name);
        return writeValue(value);
    }

    template<typename T>
    Serializator &writeValue(const T &value);

    template<typename T>
    Serializator &writeArray(const std::vector<T> &array)
    {
        beginArray(array.size());
        for (auto &value : array)
            writeValue(value);
        endArray();
        return *this;
    }

    template<typename T>
    Serializator &writeStringMap(const std::map<std::string, T> &map)
    {
        beginStringDictionary(map.size());
        for (auto &keyValue : map)
        {
            writeString(keyValue.first);
            writeValue(keyValue.second);
        }
        endStringDictionary();
        return *this;
    }

    template<typename T>
    Serializator &writeStringUnorderedMap(const std::unordered_map<std::string, T> &map)
    {
        beginStringDictionary(map.size());
        for (auto &keyValue : map)
        {
            writeString(keyValue.first);
            writeValue(keyValue.second);
        }
        endStringDictionary();
        return *this;
    }

    template<typename K, typename V>
    Serializator &writeMap(const std::map<K, V> &map)
    {
        beginDictionary(map.size());
        for (auto &keyValue : map)
        {
            writeValue(keyValue.first);
            writeValue(keyValue.second);
        }
        endDictionary();
        return *this;
    }

    template<typename K, typename V>
    Serializator &writeUnorderedMap(const std::unordered_map<K, V> &map)
    {
        beginDictionary(map.size());
        for (auto &keyValue : map)
        {
            writeValue(keyValue.first);
            writeValue(keyValue.second);
        }
        endDictionary();
        return *this;
    }
};

/**
* De-serializator object interface.
*/
struct Deserializator : public ObjectInterfaceSubclass<Deserializator, Object>
{
    LODEN_OBJECT_TYPE(Deserializator);

    virtual Deserializator &readInt8(int8_t &value) = 0;
    virtual Deserializator &readInt16(int16_t &value) = 0;
    virtual Deserializator &readInt32(int32_t &value) = 0;
    virtual Deserializator &readInt64(int64_t &value) = 0;

    virtual Deserializator &readUInt8(uint8_t &value) = 0;
    virtual Deserializator &readUInt16(uint16_t &value) = 0;
    virtual Deserializator &readUInt32(uint32_t &value) = 0;
    virtual Deserializator &readUInt64(uint64_t &value) = 0;

    virtual Deserializator &readFloat32(float &value) = 0;
    virtual Deserializator &readFloat64(double &value) = 0;

    virtual Deserializator &readString(std::string &value) = 0;
    virtual Deserializator &readObject(SerializablePtr &object) = 0;

    virtual Deserializator &beginArray(size_t &size) = 0;
    virtual Deserializator &endArray() = 0;

    virtual Deserializator &beginStringDictionary(size_t &size) = 0;
    virtual Deserializator &endStringDictionary() = 0;

    virtual Deserializator &beginDictionary(size_t &size) = 0;
    virtual Deserializator &endDictionary() = 0;

    template<typename T>
    Deserializator &readValue(T &value);

    template<typename T>
    Serializator &readArray(std::vector<T> &array)
    {
        size_t size = 0;
        beginArray(size);
        array.resize(size);
        for (auto &value : array)
            readValue(value);
        endArray();
        return *this;
    }

    template<typename T>
    Deserializator &readStringMap(std::map<std::string, T> &map)
    {
        size_t size = 0;
        beginStringDictionary(map.size());
        for (size_t i = 0; i < size; ++i)
        {
            std::string key;
            readString(key);
            readValue(map[key]);
        }
        endStringDictionary();
        return *this;
    }

    template<typename T>
    Deserializator &readStringUnorderedMap(std::unordered_map<std::string, T> &map)
    {
        size_t size = 0;
        beginStringDictionary(map.size());
        for (size_t i = 0; i < size; ++i)
        {
            std::string key;
            readString(key);
            readValue(map[key]);
        }
        endStringDictionary();
        return *this;
    }

    template<typename K, typename V>
    Deserializator &readMap(std::map<K, V> &map)
    {
        size_t size = 0;
        beginDictionary(map.size());
        for (size_t i = 0; i < size; ++i)
        {
            K key;
            readValue(key);
            readValue(map[key]);
        }
        endDictionary();
        return *this;
    }

    template<typename K, typename V>
    Deserializator &readUnorderedMap(std::unordered_map<K, V> &map)
    {
        size_t size = 0;
        beginDictionary(map.size());
        for (size_t i = 0; i < size; ++i)
        {
            K key;
            readValue(key);
            readValue(map[key]);
        }
        endDictionary();
        return *this;
    }
};

namespace detail
{
template<typename T>
struct WriteValueDispatch;

template<>
struct WriteValueDispatch<int8_t>
{
    static void apply(Serializator *self, int8_t value)
    {
        self->writeInt8(value);
    }
};

template<>
struct WriteValueDispatch<int16_t>
{
    static void apply(Serializator *self, int16_t value)
    {
        self->writeInt16(value);
    }
};

template<>
struct WriteValueDispatch<int32_t>
{
    static void apply(Serializator *self, int32_t value)
    {
        self->writeInt32(value);
    }
};

template<>
struct WriteValueDispatch<int64_t>
{
    static void apply(Serializator *self, int64_t value)
    {
        self->writeInt64(value);
    }
};

template<>
struct WriteValueDispatch<uint8_t>
{
    static void apply(Serializator *self, uint8_t value)
    {
        self->writeUInt8(value);
    }
};

template<>
struct WriteValueDispatch<uint16_t>
{
    static void apply(Serializator *self, uint16_t value)
    {
        self->writeUInt16(value);
    }
};

template<>
struct WriteValueDispatch<uint32_t>
{
    static void apply(Serializator *self, uint32_t value)
    {
        self->writeUInt32(value);
    }
};

template<>
struct WriteValueDispatch<uint64_t>
{
    static void apply(Serializator *self, uint64_t value)
    {
        self->writeUInt64(value);
    }
};

template<>
struct WriteValueDispatch<float>
{
    static void apply(Serializator *self, float value)
    {
        self->writeFloat32(value);
    }
};

template<>
struct WriteValueDispatch<double>
{
    static void apply(Serializator *self, double value)
    {
        self->writeFloat64(value);
    }
};

template<>
struct WriteValueDispatch<std::string>
{
    static void apply(Serializator *self, const std::string &value)
    {
        self->writeString(value);
    }
};

template<typename T>
struct WriteValueDispatch<std::shared_ptr<T>>
{
    static void apply(Serializator *self, const std::shared_ptr<T> &object)
    {
        self->writeObject(object);
    }
};

template<typename T>
struct WriteValueDispatch<std::vector<T>>
{
    static void apply(Serializator *self, const std::vector<T> &array)
    {
        self->writeArray(array);
    }
};

template<typename T>
struct WriteValueDispatch<std::map<std::string, T>>
{
    static void apply(Serializator *self, const std::map<std::string, T> &map)
    {
        self->writeStringMap(map);
    }
};

template<typename T>
struct WriteValueDispatch<std::unordered_map<std::string, T>>
{
    static void apply(Serializator *self, const std::map<std::string, T> &map)
    {
        self->writeStringUnorderedMap(map);
    }
};

template<typename K, typename V>
struct WriteValueDispatch<std::map<K, V>>
{
    static void apply(Serializator *self, const std::map<K, V> &map)
    {
        self->writeMap(map);
    }
};

template<typename K, typename V>
struct WriteValueDispatch<std::unordered_map<K, V>>
{
    static void apply(Serializator *self, const std::unordered_map<K, V> &map)
    {
        self->writeUnorderedMap(map);
    }
};

template<typename T>
struct ReadValueDispatch;

template<>
struct ReadValueDispatch<int8_t>
{
    static void apply(Deserializator *self, int8_t &value)
    {
        self->readInt8(value);
    }
};

template<>
struct ReadValueDispatch<int16_t>
{
    static void apply(Deserializator *self, int16_t &value)
    {
        self->readInt16(value);
    }
};

template<>
struct ReadValueDispatch<int32_t>
{
    static void apply(Deserializator *self, int32_t &value)
    {
        self->readInt32(value);
    }
};

template<>
struct ReadValueDispatch<int64_t>
{
    static void apply(Deserializator *self, int64_t &value)
    {
        self->readInt64(value);
    }
};

template<>
struct ReadValueDispatch<uint8_t>
{
    static void apply(Deserializator *self, uint8_t &value)
    {
        self->readUInt8(value);
    }
};

template<>
struct ReadValueDispatch<uint16_t>
{
    static void apply(Deserializator *self, uint16_t &value)
    {
        self->readUInt16(value);
    }
};

template<>
struct ReadValueDispatch<uint32_t>
{
    static void apply(Deserializator *self, uint32_t &value)
    {
        self->readUInt32(value);
    }
};

template<>
struct ReadValueDispatch<uint64_t>
{
    static void apply(Deserializator *self, uint64_t &value)
    {
        self->readUInt64(value);
    }
};

template<>
struct ReadValueDispatch<float>
{
    static void apply(Deserializator *self, float &value)
    {
        self->readFloat32(value);
    }
};

template<>
struct ReadValueDispatch<double>
{
    static void apply(Deserializator *self, double &value)
    {
        self->readFloat64(value);
    }
};

template<>
struct ReadValueDispatch<std::string>
{
    static void apply(Deserializator *self, std::string &value)
    {
        self->readString(value);
    }
};

template<typename T>
struct ReadValueDispatch<std::shared_ptr<T>>
{
    static void apply(Deserializator *self, std::shared_ptr<T> &object)
    {
        SerializablePtr serializable;
        self->readObject(serializable);
        if (serializable)
        {
            object = querySharedInterface<T>(serializable);
        }
    }
};

template<typename T>
struct ReadValueDispatch<std::vector<T>>
{
    static void apply(Deserializator *self, std::vector<T> &array)
    {
        self->readArray(array);
    }
};

template<typename T>
struct ReadValueDispatch<std::map<std::string, T>>
{
    static void apply(Deserializator *self, std::map<std::string, T> &map)
    {
        self->readStringMap(map);
    }
};

template<typename T>
struct ReadValueDispatch<std::unordered_map<std::string, T>>
{
    static void apply(Deserializator *self, std::map<std::string, T> &map)
    {
        self->readStringUnorderedMap(map);
    }
};

template<typename K, typename V>
struct ReadValueDispatch<std::map<K, V>>
{
    static void apply(Deserializator *self, std::map<K, V> &map)
    {
        self->readMap(map);
    }
};

template<typename K, typename V>
struct ReadValueDispatch<std::unordered_map<K, V>>
{
    static void apply(Deserializator *self, std::unordered_map<K, V> &map)
    {
        self->readUnorderedMap(map);
    }
};

}

template<typename T>
inline Deserializator &Deserializator::readValue(T &value)
{
    detail::ReadValueDispatch<T>::apply(this, value);
    return *this;
}

template<typename T>
inline Serializator &Serializator::writeValue(const T &value)
{
    detail::WriteValueDispatch<T>::apply(this, value);
    return *this;
}

} // End of namespace Loden

#endif //LODEN_SERIALIZABLE_HPP
