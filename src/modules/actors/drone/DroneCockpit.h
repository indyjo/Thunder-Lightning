#ifndef TNL_DRONECOCKPIT_H
#define TNL_DRONECOCKPIT_H

#include <vector>
#include <modules/jogi/JRenderer.h>
#include <object.h>

struct ICamera;

class Drone;
class Model;
class Texture;
class RenderPass;

class MfdModule;

class DroneCockpit : public SigObject {
    WeakPtr<IGame> thegame;
    WeakPtr<Drone> drone;
    Ptr<Model> interior_model, mfd_model;
    Ptr<Texture> mfd_tex;
    Ptr<RenderPass> mfd_pass;
    
    typedef std::vector<Ptr<MfdModule> > MfdModules;
    MfdModules mfd_modules;
    int current_mfd_module;
    
public:
    DroneCockpit(WeakPtr<IGame> game, JRenderer *, Ptr<Drone> drone, Ptr<Model> interior_model, Ptr<Model> mfd_model);
    ~DroneCockpit();
    
    void switchMfdMode();
    
    Ptr<RenderPass> createRenderPass(Ptr<ICamera> cam);
    
private:
    void drawCockpit(Ptr<RenderPass>, Ptr<ICamera>);
};

#endif

