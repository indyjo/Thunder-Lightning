#ifndef PASSIVE_ACTOR_H
#define PASSIVE_ACTOR_H

#include "simpleactor.h"

class Model;

class PassiveActor: public SimpleActor {
public:
	PassiveActor(Ptr<IGame> game);
	
	inline void setModel(Ptr<Model> model) { this->model = model; }
	
	virtual void action();
	virtual void draw();
private:
	Ptr<Model> model;
};

#endif
