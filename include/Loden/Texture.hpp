#ifndef LODEN_TEXTURE_HPP
#define LODEN_TEXTURE_HPP

#include "Loden/Object.hpp"
#include "Loden/Engine.hpp"
#include "Loden/Image/ImageBuffer.hpp"
#include "Agpu/agpu.hpp"

namespace Loden
{
LODEN_DECLARE_CLASS(Texture);

/**
 * Loden texture
 */
class Texture: public ObjectSubclass<Texture, Object>
{
    LODEN_OBJECT_TYPE(Texture);
public:
    Texture(const agpu_texture_ref &handle = nullptr);
    ~Texture();

    static TexturePtr createFromImage(Engine *engine, Image::ImageBuffer *imageBuffer, agpu_texture_format format = AGPU_TEXTURE_FORMAT_UNKNOWN);

    const agpu_texture_ref &getHandle() const
    {
        return handle;
    }

private:
    agpu_texture_ref handle;
};

} // End of namespace

#endif //LODEN_TEXTURE_HPP