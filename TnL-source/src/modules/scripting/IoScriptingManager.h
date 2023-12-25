#ifndef IOSCRIPTINGMANAGER_H
#define IOSCRIPTINGMANAGER_H
#ifdef HAVE_IO

#include "IoIncludes.h"
#include <object.h>

struct IGame;

struct IoCallbackContext {
	virtual void printCallback(const char *)=0;
	virtual void exceptionCallback(IoObject *)=0;
	virtual void exitCallback()=0;
	void connectTo(IoState *state);
};

/// Extended IoState subclasses an IoState and adds some data
struct IoStateEx;

class IoScriptingManager : public Object
{
public:
	IoScriptingManager(Ptr<IGame>);
	virtual ~IoScriptingManager();

	inline IoState * getMainState() { return main_state; }
	
	IoState* createIoStateEx();
	void removeIoStateEx(IoState*);
	
private:
	Ptr<IGame> game;
	IoState * main_state;
};

void IoStateEx_coupleLifetime(Object* cpp_object, IoObject* io_object);
void IoStateEx_removeCoupling(Object* cpp_object, IoObject* io_object);
	

#endif
#endif