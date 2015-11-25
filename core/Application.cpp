#include "Loden/Application.hpp"

namespace Loden
{
	
int Application::main(int argc, const char **argv)
{
	// Parse the command line
	if(!parseCommandLine(argc, argv))
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
	
	while(!mainLoopQuit_)
	{
		pumpEvents();
		
		auto newTime = SDL_GetTicks();
		auto deltaTime = (newTime - lastTime) * 0.001;
		lastTime = newTime;
		availableTime += (float)deltaTime;
		
		// Perform multiples updates in fixed steps for determinism.
		while(availableTime >= updateStep)
		{
			mainLoopUpdateStep(updateStep);
			availableTime -= updateStep;
		}
		
		// Perform render step
		mainLoopRenderStep();
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

} // End of namespace Loden
