#include "coroutine.h"

CoRoutine::InterruptedException::~InterruptedException()
{
}

CoRoutine::CoRoutine(MTasker<> & mt)
:	is_running(false),
	interrupt_requested(false),
	mtasker(mt)
{ }

void CoRoutine::start() {
	if (is_running) return;
	this->ref();
	//ls_message("New coroutine %p.\n", this);
	mtasker.makeThread((MTasker<>::tfunc_t*)startCoRoutine, this);
	is_running = true;
}

void CoRoutine::interrupt() {
	if (!is_running) return;
	interrupt_requested = true;
}

void CoRoutine::yield() throw(InterruptedException&) {
	//ls_message("Coro %p yielding\n", this);
	mtasker.yield();
	//ls_message("returned from yielding to coro %p\n", this);
	if (interrupt_requested) {
		//ls_message("interrupting.\n");
		throw InterruptedException();
	}
}

void CoRoutine::startCoRoutine(CoRoutine * cr) {
	//ls_message("in coroutine %p!\n",cr);
	try {
		cr->run();
	} catch (InterruptedException & ex) {
		// Do nothing. This is the usual case.
	}
	cr->is_running = false;
	cr->interrupt_requested = false;
	cr->unref();
}
