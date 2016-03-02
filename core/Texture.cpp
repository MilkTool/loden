#include "Loden/Texture.hpp"

namespace Loden
{
Texture::Texture(const agpu_texture_ref &handle)
    : handle(handle)
{
}

Texture::~Texture()
{
}

TexturePtr Texture::createFromImage(Engine *engine, Image::ImageBuffer *imageBuffer, agpu_texture_format format)
{
    auto &device = engine->getAgpuDevice();

    // Create the texture
    agpu_texture_description desc;
    memset(&desc, 0, sizeof(desc));
    desc.type = AGPU_TEXTURE_2D;
    desc.format = format;
    desc.width = (agpu_uint)imageBuffer->getWidth();
    desc.height = (agpu_uint)imageBuffer->getHeight();
    desc.depthOrArraySize = 1;
    desc.miplevels = 1;
    desc.sample_count = 1;
    desc.sample_quality = 0;
    desc.flags = AGPU_TEXTURE_FLAG_UPLOADED;
    agpu_texture_ref texture = device->createTexture(&desc);
    if (!texture)
        return nullptr;

    // Upload the texture data.
    texture->uploadTextureData(0, 0, imageBuffer->getPitch(), imageBuffer->getSlicePitch(), imageBuffer->get());

    return std::make_shared<Texture>(texture);
}

} // End of namespace Loden
