#include "Loden/Engine.hpp"
#include "Loden/PipelineStateManager.hpp"
#include "Loden/Printing.hpp"
#include "Loden/Settings.hpp"
#include "Loden/GUI/FontManager.hpp"

namespace Loden
{

Engine::Engine()
{
}

Engine::~Engine()
{
}

EnginePtr Engine::create()
{
    return std::make_shared<Engine> ();
}

bool Engine::initialize(int argc, const char **argv)
{
    if (!createDevice())
        return false;

    if (!loadSettings(argc, argv))
        return false;

    if (!createPipelineStateManager())
        return false;

    if (!createFontManager())
        return false;

    return true;
}

void Engine::shutdown()
{
    graphicsCommandQueue->finishExecution();

    if (fontManager)
    {
        fontManager->shutdown();
        fontManager.reset();
    }

    if (pipelineStateManager)
    {
        pipelineStateManager->shutdown();
        pipelineStateManager.reset();
    }

}

bool Engine::createDevice()
{
    // Get the platform.
    agpu_platform *platform;
    agpuGetPlatforms(1, &platform, nullptr);
    if (!platform)
    {
        printError("Failed to get the AGPU platform\n");
        return false;
    }

    agpu_device_open_info openInfo;
    memset(&openInfo, 0, sizeof(openInfo));

#ifdef _DEBUG
    // Use the debug layer when debugging. This is useful for low level backends.
    openInfo.debug_layer = true;
#endif

    device = platform->openDevice(&openInfo);
    if (!device)
    {
        printError("Failed to open the AGPU device\n");
        return false;
    }


    // Get the main command queue.
    graphicsCommandQueue = device->getDefaultCommandQueue();
    if (!graphicsCommandQueue)
    {
        printError("Failed to get the default AGPU command queue\n");
        return nullptr;
    }

    return true;
}

bool Engine::loadSettings(int argc, const char **argv)
{
    settings = std::make_shared<Settings>();

    //int sampleCount = 4;
    //auto qualityLevels = device->getMultiSampleQualityLevels(sampleCount);
    //settings->setIntValue("Rendering", "SampleCount", sampleCount);
    //settings->setIntValue("Rendering", "SampleQuality", qualityLevels - 1);
    
    return true;
}

bool Engine::createPipelineStateManager()
{
    // Create the pipeline state manager.
    pipelineStateManager = std::make_shared<PipelineStateManager>(this);
    if (!pipelineStateManager->initialize())
    {
        printError("Failed to initialize the pipeline state manager.\n");
        return false;
    }

    return true;
}

bool Engine::createFontManager()
{
    // Create the font manager
    fontManager = std::make_shared<GUI::FontManager>(this);
    if (!fontManager->initialize())
    {
        printError("Failed to initialize the font manager.\n");
        return false;
    }

    return true;
}

const agpu_device_ref &Engine::getAgpuDevice() const
{
    return device;
}

const agpu_command_queue_ref &Engine::getGraphicsCommandQueue() const
{
    return graphicsCommandQueue;
}

const SettingsPtr &Engine::getSettings() const
{
    return settings;
}

const PipelineStateManagerPtr &Engine::getPipelineStateManager() const
{
    return pipelineStateManager;
}

const GUI::FontManagerPtr &Engine::getFontManager() const
{
    return fontManager;
}

} // End of namespace Loden
