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
	ls_message("Making Thread.\n");
	mtasker.makeThread((MTasker<>::tfunc_t*)startCoRoutine, this);
	is_running = true;
}

void CoRoutine::interrupt() {
	if (!is_running) return;
	interrupt_requested = true;
}

void CoRoutine::yield() throw(InterruptedException&) {
	mtasker.yield();
	if (interrupt_requested) {
		throw InterruptedException();
	}
}

void CoRoutine::startCoRoutine(CoRoutine * cr) {
	ls_message("in coroutine!\n");
	try {
		cr->run();
	} catch (InterruptedException & ex) {
		// Do nothing. This is the usual case.
	}
	cr->is_running = false;
	cr->interrupt_requested = false;
	cr->unref();
}
