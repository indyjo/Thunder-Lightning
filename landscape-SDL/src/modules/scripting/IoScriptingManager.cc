#include <landscape.h>
#include <interfaces/IGame.h>

#include "IoScriptingManager.h"

namespace {
	typedef IoCallbackContext Ctx;
	void globalPrintCallback(Ctx *ctx, const char *str) {
		if (ctx) ctx->printCallback(str);
	}
	void globalExceptionCallback(Ctx *ctx, IoException *e) {
		if(ctx) ctx->exceptionCallback(e);
	}
	void globalExitCallback(Ctx *ctx) {
		if(ctx) ctx->exitCallback();
	}
	struct DefaultCallbackContext : public IoCallbackContext
	{
		virtual void printCallback(const char *str) {
			ls_message("%s",str);
		}
		virtual void exceptionCallback(IoException * e) {
			ls_error("Io Exception: %s - %s\n",
				IoException_name(e), 
				IoException_description(e));
		}
		virtual void exitCallback() {
			ls_warning("Io has called exit - ignored.\n");
		}
	} default_context;
}

void IoCallbackContext::connectTo(IoState *state) {
	IoState_callbackContext_(state, this);
	IoState_printCallback_(state,
		(IoStatePrintCallback*) globalPrintCallback);
	IoState_exceptionCallback_(state,
		(IoStateExceptionCallback*) globalExceptionCallback);
	IoState_exitCallback_(state,
		(IoStateExitCallback*) globalExitCallback);
}

IoScriptingManager::IoScriptingManager(Ptr<IGame> game)
: main_state(createNewState())
{
}

IoScriptingManager::~IoScriptingManager()
{
	IoState_exit(main_state);
}

IoState * IoScriptingManager::createNewState() {
	ls_message("Creating new state.\n");
	IoState * state = IoState_new();
	ls_message("Done creating new state.\n");
	default_context.connectTo(state);
	
	//TODO: export C++ classes
	//IoObject *lobby = IoState_lobby(state);
	//IoObject_clone(lobby
	
	return state;
}

