#include "Loden/Application.hpp"

namespace Loden
{
	
int Application::main(int argc, const char **argv)
{
	// Parse the command line
	if(!parseCommandLine(argc, argv))
		return -1;

    // Create the engine
    engine = Engine::create();
    if (!engine->initialize(argc, argv))
        return -1;

	// Initialize the application
	if(!initialize())
		return -1;
		
	// Run the application
	if(!run())
		return -1;

	// Shutdown the application
	if(!shutdown())
		return -1;
	return 0;
}

const EnginePtr &Application::getEngine() const
{
    return engine;
}

bool Application::parseCommandLine(int argc, const char **argv)
{
	return true;
}

bool Application::initialize()
{
	return true;
}

bool Application::run()
{
	return true;
}

bool Application::shutdown()
{
    engine->shutdown();
    engine.reset();
	return true;
}

void Application::mainLoopQuit()
{
	mainLoopQuit_ = true;
}

bool Application::enterMainLoop(float updateStep)
{
	mainLoopQuit_ = false;
	auto lastTime = SDL_GetTicks();
	float availableTime = 0;
    int frameCount = 0;
    float frameTime = 0;
	
	while(!mainLoopQuit_)
	{
		pumpEvents();
		
		auto newTime = SDL_GetTicks();
		auto deltaTime = (newTime - lastTime) * 0.001;
		lastTime = newTime;
		availableTime += (float)deltaTime;
        frameTime += deltaTime;
        if (frameTime > 1)
        {
            auto fps = frameCount / frameTime;
            updateFpsDisplay(fps);
            frameTime = 0;
            frameCount = 0;
        }
		
		// Perform multiples updates in fixed steps for determinism.
		while(availableTime >= updateStep)
		{
			mainLoopUpdateStep(updateStep);
			availableTime -= updateStep;
		}
		
		// Perform render step
		mainLoopRenderStep();
        ++frameCount;
	}
	return true;
}

void Application::pumpEvents()
{
}

void Application::mainLoopUpdateStep(float deltaTime)
{
}

void Application::mainLoopRenderStep()
{
}

void Application::updateFpsDisplay(float fps)
{
}

} // End of namespace Loden
