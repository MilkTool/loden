#include "Loden/GUI/FontManager.hpp"
#include "Loden/JSON.hpp"
#include "Loden/FileSystem.hpp"
#include "FreeTypeFont.hpp"
#include "LodenFont.hpp"

namespace Loden
{
namespace GUI
{

FontManager::FontManager(Engine *engine)
    : engine(engine)
{
}

FontManager::~FontManager()
{
}

bool FontManager::initialize()
{
    FontLoaderPtr fontLoader = std::make_shared<FreeTypeFontLoader>(engine);
    if (fontLoader->initialize())
        fontLoaders.push_back(fontLoader);

    fontLoader = std::make_shared<LodenFontLoader>(engine);
    if (fontLoader->initialize())
        fontLoaders.push_back(fontLoader);

    loadFontsFromFile("core-assets/fonts/fonts.json");

    return true;
}

bool FontManager::loadFontsFromFile(const std::string &fontsDescriptionFileName)
{
    rapidjson::Document document;
    if (!parseJsonFromFile(fontsDescriptionFileName, &document))
        return false;

    // Load the referenced fonts first.
    auto basePath = dirname(fontsDescriptionFileName);
    if (document.HasMember("include"))
    {
        auto &inclusionValue = document["include"];
        if (!inclusionValue.IsObject())
            return false;

        for (auto it = inclusionValue.MemberBegin(); it != inclusionValue.MemberEnd(); ++it)
        {
            //auto name = it->name.GetString();
            auto &fileNameValue = it->value;
            if (!fileNameValue.IsString())
                return false;

            auto fileName = fileNameValue.GetString();
            if (!loadFontsFromFile(joinPath(basePath, fileName)))
                return false;
        }
    }

    // Load the fonts.
    if (document.HasMember("fonts"))
    {
        auto &fontsDesc = document["fonts"];
        if (!fontsDesc.IsObject())
            return false;

        for (auto it = fontsDesc.MemberBegin(); it != fontsDesc.MemberEnd(); ++it)
        {
            auto fontName = it->name.GetString();
            auto &fontDesc = it->value;
            if (!fontDesc.IsObject() || !fontDesc.HasMember("faces"))
                return false;

            // Create the font
            auto font = std::make_shared<Font> ();
            addFont(fontName, font);

            // Load the faces
            auto &facesDesc = fontDesc["faces"];
            for (auto faceIt = facesDesc.MemberBegin(); faceIt != facesDesc.MemberEnd(); ++faceIt)
            {
                auto faceName = faceIt->name.GetString();
                auto &faceDesc = faceIt->value;
                if (!faceDesc.IsObject() || !faceDesc.HasMember("file"))
                    return false;

                auto &faceFileName = faceDesc["file"];
                if (!faceFileName.IsString())
                    return false;

                auto face = loadFaceFromFile(joinPath(basePath, faceFileName.GetString()));
                if (face)
                    font->addFace(faceName, face);
            }

            // Cache the special faces.
            font->loadSpecialFaces();
        }
    }

    // Load the defaults.
    if (document.HasMember("default-font") && document["default-font"].IsString())
        defaultFont = getFont(document["default-font"].GetString());
    
    if (document.HasMember("default-mono-font") && document["default-mono-font"].IsString())
        defaultMononospacedFont = getFont(document["default-mono-font"].GetString());
    
    if (document.HasMember("default-sans-font") && document["default-sans-font"].IsString())
        defaultSansFont = getFont(document["default-sans-font"].GetString());

    if (document.HasMember("default-serif-font") && document["default-serif-font"].IsString())
        defaultSerifFont = getFont(document["default-serif-font"].GetString());

    return true;
}

FontFacePtr FontManager::loadFaceFromFile(const std::string &fileName)
{
    for (auto &loader : fontLoaders)
    {
        if (loader->canLoadFaceFromFile(fileName))
            return loader->loadFaceFromFile(fileName);
    }

    return nullptr;
}

void FontManager::shutdown()
{
    for (auto &font : fonts)
        font.second->release();
    fonts.clear();

    for(auto &loader : fontLoaders)
        loader->shutdown();
    fontLoaders.clear();
}

void FontManager::addFont(const std::string &name, const FontPtr &font)
{
    fonts.insert(std::make_pair(name, font));
}

FontPtr FontManager::getFont(const std::string &name)
{
    auto it = fonts.find(name);
    if (it != fonts.end())
        return it->second;
    return nullptr;
}

FontPtr FontManager::getDefaultFont() const
{
    return defaultFont;
}

FontPtr FontManager::getDefaultSansFont() const
{
    return defaultSansFont;
}

FontPtr FontManager::getDefaultSerifFont() const
{
    return defaultSerifFont;
}

FontPtr FontManager::getDefaultMonospaceFont() const
{
    return defaultMononospacedFont;
}

} // End of namespace Loden
} // End of namespace GUI
