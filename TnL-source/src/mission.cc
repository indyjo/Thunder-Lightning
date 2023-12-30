#include <modules/actors/drone/drone.h>
#include <modules/actors/carrier/Carrier.h>
#include <interfaces/IGame.h>

void setup_mission(Ptr<IGame> game) {
    Ptr<Drone> drone = new Drone(game);
    drone->init();
    auto player_pos = Vector(11341,400,-1008);
    drone->setLocation(player_pos);
    drone->setMovementVector(Vector(0, 0, 90));
    game->addActor(drone);
    game->setCurrentView(drone->getView(4));
    game->setCurrentlyControlledActor(drone);

    Ptr<Carrier> carrier = new Carrier(game);
    carrier->setLocation((player_pos + Vector(0, -400, 90*25)));
    carrier->setOrientation(Quaternion::Rotation(Vector(0,1,0), 15 * M_PI / 180));
    game->addActor(carrier);
}