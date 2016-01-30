#ifndef LODEN_ENGINE_HPP
#define LODEN_ENGINE_HPP

#include "Loden/Common.hpp"
#include "AGPU/agpu.hpp"

namespace Loden
{

namespace GUI
{
LODEN_DECLARE_CLASS(FontManager);
}

LODEN_DECLARE_CLASS(Engine);
LODEN_DECLARE_CLASS(Settings);
LODEN_DECLARE_CLASS(PipelineStateManager);
LODEN_DECLARE_CLASS(VirtualFileSystem);

/**
 * The entry point access the resources of the engine.
 */
class LODEN_CORE_EXPORT Engine
{
public:
    Engine();
    ~Engine();

    static EnginePtr create();

    bool initialize(int argc, const char **argv);
    void shutdown();

    const SettingsPtr &getSettings() const;

    const agpu_device_ref &getAgpuDevice() const;
    const agpu_command_queue_ref &getGraphicsCommandQueue() const;

    const PipelineStateManagerPtr &getPipelineStateManager() const;
    const GUI::FontManagerPtr &getFontManager() const;

private:
    bool createDevice();
    bool loadSettings(int argc, const char **argv);
    bool createPipelineStateManager();
    bool createFontManager();

    agpu_device_ref device;
    agpu_command_queue_ref graphicsCommandQueue;

    SettingsPtr settings;
    PipelineStateManagerPtr pipelineStateManager;
    GUI::FontManagerPtr fontManager;
};

} // End of namespace Loden

#endif //LODEN_ENGINE_HPP
