#ifndef LODEN_APPLICATION_HPP
#define LODEN_APPLICATION_HPP

#include "Loden/Common.hpp"
#include "SDL.h"

namespace Loden
{

/**
 * Loden Application class
 */	
class LODEN_CORE_EXPORT Application
{
public:
	virtual int main(int argc, const char **argv);
	
protected:
	virtual bool parseCommandLine(int argc, const char **argv);
	virtual bool initialize();
	virtual bool run();
	virtual bool shutdown();
	
	virtual bool enterMainLoop(float updateStep);
	virtual void pumpEvents();
	virtual void mainLoopQuit();
	virtual void mainLoopUpdateStep(float updateDelta);
	virtual void mainLoopRenderStep();

private:
	bool mainLoopQuit_;
};

} // End of namespace Loden

#define APPLICATION_ENTRY_POINT(applicationClass) \
int main(int argc, char *argv[]) { \
	applicationClass app; \
	return app.main(argc, (const char **)argv); \
} 

#endif //LODEN_APPLICATION_HPP
