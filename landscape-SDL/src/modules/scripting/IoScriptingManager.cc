#include <cstring>
#include <landscape.h>
#include <interfaces/IConfig.h>
#include <interfaces/IGame.h>

#include "IoScriptingManager.h"
#include "mappings.h"

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
				IoString_asCString(IoException_name(e)), 
				IoString_asCString(IoException_description(e)));
			ls_error("%s\n",
				IoString_asCString(IoException_backTraceString(e, NULL, NULL)));
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
	IoState_exit(main_state);
}

IoState * IoScriptingManager::createNewState() {
	IoState * state = IoState_new();
	default_context.connectTo(state);

	addMappings(game, state);

	char buf[256];
	strncpy(buf,game->getConfig()->query("Io_init_script"),256);
	IoState_doFile_(state, buf);
	
	return state;
}

