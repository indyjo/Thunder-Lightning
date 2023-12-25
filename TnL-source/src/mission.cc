#include <modules/actors/drone/drone.h>
#include <interfaces/IGame.h>

void setup_mission(Ptr<IGame> game) {
    Ptr<Drone> drone = new Drone(game);
    drone->init();
    drone->setLocation(Vector(11341,400,-1008));
    drone->setMovementVector(Vector(0, 0, 90));
    game->addActor(drone);
    game->setCurrentView(drone->getView(4));
    game->setCurrentlyControlledActor(drone);
}