#include <cstddef>
#include <cstring>
#include <cassert>
#include <vector>
#include <tnl.h>
#include <interfaces/IConfig.h>
#include <interfaces/IGame.h>
#include <UArray.h>

#include "IoScriptingManager.h"
#include "mappings.h"

struct IoStateEx {
    IoState io_state;
    std::vector<std::pair<Object*,IoObject*> > coupled_objects;
    IoObject *lifetime_coupler;
};

namespace {
	typedef IoCallbackContext Ctx;
	
	void globalPrintCallback(Ctx *ctx, const UArray * uarray) {
		if (ctx) switch (uarray->encoding) {
		case CENCODING_ASCII:
		case CENCODING_UTF8:
		    ctx->printCallback((const char *) uarray->data);
		    break;
		default:
		    ctx->printCallback("<binary data>");
		}
	}
	
	bool global_inside_exception = false;
	void globalExceptionCallback(Ctx *ctx, IoObject *e) {
	    if (global_inside_exception) {
	        ls_error("Error: exception occurred inside exception handler.\n");
	        if(ctx) ctx->exceptionCallback(e);
	        //exit(-1);
	    } else {
	        global_inside_exception = true;
		    if(ctx) ctx->exceptionCallback(e);
		    global_inside_exception = false;
		}
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
			ls_error("Io Exception has occurred.\n");
			IoCoroutine_rawPrintBackTrace(e);
		}
		virtual void exitCallback() {
			ls_warning("Io has called exit.\n");
			exit(-1);
		}
	} default_context;
	
	// Callback for the lifetime coupler Io object in every IoStateEx.
	// Called during garbage collection's mark phase.
	void lifetime_coupler_mark(IoObject *self) {
	    typedef std::vector<std::pair<Object*,IoObject*> >::iterator Iter;
	    IoStateEx *state = reinterpret_cast<IoStateEx*>(IOSTATE);
	    
	    // We iterate through all pairs and mark the IoObject, if the C++
	    // object has at least 2 references, i.e. not only the one caused by
	    // the IoObject.
	    // In other words: if the C++ object's refcount sinks down to 1, we
	    // can be sure that the only reference left stems from the IoObject,
	    // so we don't need to artificially keep the IoObject alive anymore.
	    for(Iter i=state->coupled_objects.begin(); i!=state->coupled_objects.end(); ++i) {
	        if (i->first->getRefs() >= 2) {
	            IoObject_shouldMark(i->second);	        
                //ls_message("C++ object @%p has %d refs so IoObject @%p->%p gets marked\n",
                //    i->first, i->first->getRefs(), i->second, i->second->object);
	        } else {
                //ls_message("C++ object @%p has %d refs so IoObject @%p->%p gets NOT marked\n",
                //    i->first, i->first->getRefs(), i->second, i->second->object);
	        }
	    }
	}
	
	// Callback for the lifetime coupler Io object in every IoStateEx.
	// Called whenever one of the coupled objects has been free'd
	void lifetime_coupler_notification(IoObject *self, IoObject *removed) {
	    //ls_message("Notification: %p collected.\n", removed);
	    // Delete the entry with the removed object from the list.
	    typedef std::vector<std::pair<Object*,IoObject*> >::iterator Iter;
	    IoStateEx *state = reinterpret_cast<IoStateEx*>(IOSTATE);
	    for(Iter i=state->coupled_objects.begin(); i!=state->coupled_objects.end(); ++i) {
	        if (i->second == removed) {
                //ls_message("Coupling between C++ object @%p and IoObject @%p->%p removed because the latter was free'd.\n",
                //    i->first, i->second, i->second->object);
	            state->coupled_objects.erase(i);
	            break;
	        }
	    }
	}
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
: game(game), main_state(createIoStateEx())
{
}

IoScriptingManager::~IoScriptingManager()
{
	ls_message("Killing Io.\n");
	removeIoStateEx(main_state);
	ls_message("Done.\n");
}

IoState * IoScriptingManager::createIoStateEx() {
    IoStateEx* state = new IoStateEx;
    IoState* io_state = reinterpret_cast<IoState*>(state);
    assert(io_state == &state->io_state);
    
    // IoState initializing relies at some points on IoState having been
    // zeroed on allocation.
    memset(io_state, 0, sizeof(IoState));
    
    IoState_new_atAddress(io_state);
    
    // now the initialization of our lifetime coupler
    IoState_pushRetainPool(io_state);
    IoObject *self = IoObject_new(io_state);
    
    IoTag *tag = IoTag_newWithName_("LifetimeCoupler");
    tag->state = io_state;
    tag->markFunc = (IoTagMarkFunc*) lifetime_coupler_mark;
    tag->notificationFunc = (IoTagNotificationFunc*) lifetime_coupler_notification;
    IoObject_tag_(self, tag);
    
    state->lifetime_coupler = self;
    IoState_retain_(io_state, state->lifetime_coupler);
    IoState_popRetainPool(io_state);
    
	default_context.connectTo(io_state);
	return io_state;
}

void IoScriptingManager::removeIoStateEx(IoState *io_state) {
    IoStateEx* state = reinterpret_cast<IoStateEx*>(io_state);
    
    IoState_done(io_state);    
    delete state;
}

void IoStateEx_coupleLifetime(Object* cpp_object, IoObject* io_object) {
    IoState* io_state = IoObject_state(io_object);
    IoStateEx* state = reinterpret_cast<IoStateEx*>(io_state);
    state->coupled_objects.push_back(std::make_pair(cpp_object, io_object));
    IoObject_addListener_(io_object, state->lifetime_coupler);
    IoObject_addingRef_(state->lifetime_coupler, io_object);
    //ls_message("Coupling lifetime between C++ object @%p and IoObject @%p->%p\n",
    //    cpp_object, io_object, io_object->object);
}

void IoStateEx_removeCoupling(Object* cpp_object, IoObject* io_object) {
    typedef std::vector<std::pair<Object*,IoObject*> >::iterator Iter;
    
    IoState* io_state = IoObject_state(io_object);
    IoStateEx* state = reinterpret_cast<IoStateEx*>(io_state);
    
    //ls_message("De-coupling lifetime between C++ object @%p and IoObject @%p->%p\n",
    //    cpp_object, io_object, io_object->object);
    for(Iter i=state->coupled_objects.begin(); i!=state->coupled_objects.end(); ++i) {
        if (i->first == cpp_object && i->second == io_object) {
            state->coupled_objects.erase(i);
            break;
        }
    }
}
	

