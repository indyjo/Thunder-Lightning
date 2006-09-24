#include <cstring>
#include <landscape.h>
#include <interfaces/IConfig.h>
#include <interfaces/IGame.h>

#include "IoScriptingManager.h"
#include "mappings.h"

namespace {
	typedef IoCallbackContext Ctx;
	void globalPrintCallback(Ctx *ctx, int length, const char *str) {
		if (ctx) {
		    char * s = new char[length+1];
		    strcpy(s, str);
		    ctx->printCallback(s);
		}
	}
	void globalExceptionCallback(Ctx *ctx, IoObject *e) {
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
		virtual void exceptionCallback(IoObject * e) {
			ls_error("Io Exception has occurred.");
			IoCoroutine_rawPrintBackTrace(e);
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
: game(game), main_state(createNewState())
{
}

IoScriptingManager::~IoScriptingManager()
{
	ls_message("Killing Io.\n");
	IoState_free(main_state);
	ls_message("Done.\n");
}

IoState * IoScriptingManager::createNewState() {
	IoState * state = IoState_new();
	default_context.connectTo(state);
	return state;
}

