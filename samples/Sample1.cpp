#include "SDL.h"
#include "Loden/Application.hpp"

using namespace Loden;

class Sample1: public Application
{
protected:
	virtual bool initialize();
	virtual bool run();
	virtual bool shutdown();
};

APPLICATION_ENTRY_POINT(Sample1);

bool Sample1::initialize()
{
	return true;
}

bool Sample1::run()
{
	return true;
}

bool Sample1::shutdown()
{
	return Application::shutdown();
}
