#ifndef IOSCRIPTINGMANAGER_H
#define IOSCRIPTINGMANAGER_H

#include "IoIncludes.h"
#include <object.h>

struct IGame;

struct IoCallbackContext {
	virtual void printCallback(const char *)=0;
	virtual void exceptionCallback(IoException *)=0;
	virtual void exitCallback()=0;
	void connectTo(IoState *state);
};

class IoScriptingManager : public Object
{
public:
	IoScriptingManager(Ptr<IGame>);
	virtual ~IoScriptingManager();

	inline IoState * getMainState() { return main_state; }
	IoState * createNewState();
private:
	IoState * main_state;
};


#endif
