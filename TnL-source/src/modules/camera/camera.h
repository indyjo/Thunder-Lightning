#include "SimpleCamera.h"
#include <interfaces/IConfig.h>
#include <interfaces/IGame.h>

/// The legacy camera module, a heavyweight object living for the whole lifetime
/// of the game and listening for game events to implement zoom in and out.
///
/// The game maintains a reference to this object which can be queried with
/// Game::getCamera().
class Camera: public SimpleCamera, public SigObject
{
public:
    Camera(IGame *thegame);
    void zoomIn();
    void zoomOut();
};

