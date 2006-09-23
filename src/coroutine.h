#ifndef COROUTINE_H
#define COROUTINE_H

#include <landscape.h>
#include <mtasker.h>
#include <object.h>


class CoRoutine : virtual public Object {
	bool interrupt_requested;
	bool is_running;
	MTasker<> & mtasker;
	
	struct InterruptedException {
		virtual ~InterruptedException();
	};

public:
	CoRoutine(MTasker<> & mt);
	
	virtual void run() = 0;
	
	void start();
	void interrupt();
	
	inline bool isRunning() { return is_running; }
	
protected:
	void yield() throw(InterruptedException&);
private:
	static void startCoRoutine(CoRoutine *);
};

#endif
