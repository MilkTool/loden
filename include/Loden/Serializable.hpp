#ifndef LODEN_SERIALIZABLE_HPP
#define LODEN_SERIALIZABLE_HPP

#include "Loden/Object.hpp"

namespace Loden
{
LODEN_DECLARE_INTERFACE(Serializable);
LODEN_DECLARE_INTERFACE(Serializator);
LODEN_DECLARE_INTERFACE(Deserializator);

/**
 * Serializable object interface.
 */
struct Serializable : public ObjectInterfaceSubclass<Serializable, Object>
{
    LODEN_OBJECT_TYPE(Serializable);

    virtual void serialize(Serializator &output) = 0;
    virtual void deserialize(Deserializator &input) = 0;
};

} // End of namespace Loden

#endif //LODEN_SERIALIZABLE_HPP
