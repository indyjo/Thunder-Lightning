#include "mappings.h"

class Clock;
struct IConfig;
struct IActorStage;
struct IPositionProvider;
struct IMovementProvider;
struct IPositionReceiver;
struct IMovementReceiver;
class Faction;
class Drone;
class Tank;
class Environment;
class SimpleActor;
struct IModelMan;
class Model;
class Armament;
class Weapon;
class Targeter;

void addBasicMappings(Ptr<IGame> game, IoState * state) {
	addMapping<IGame>(game, state);
	addMapping<IConfig>(game,state);
}

void addMappings(Ptr<IGame> game, IoState * state) {
	addMapping<Clock>(game,state);
	
    addMapping<IPositionProvider>(game,state);
	addMapping<IMovementProvider>(game,state);
	addMapping<IPositionReceiver>(game,state);
	addMapping<IMovementReceiver>(game,state);
	
    addMapping<IActor>(game, state);
	addMapping<SimpleActor>(game,state);
	
    addMapping<Drone>(game,state);
	addMapping<Tank>(game,state);
	
    addMapping<EventRemapper>(game,state);
	addMapping<IActorStage>(game,state);
	addMapping<Faction>(game,state);
	addMapping<Environment>(game,state);
	addMapping<IModelMan>(game,state);
	addMapping<Model>(game,state);
    addMapping<Armament>(game,state);
    addMapping<Weapon>(game,state);
    addMapping<Targeter>(game,state);
}
