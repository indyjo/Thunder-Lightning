#ifndef GUNSIGHT_H
#define GUNSIGHT_H

#include <vector>
#include <string>

#include <interfaces/ICamera.h>
#include <interfaces/IDrawable.h>
#include <modules/ui/Surface.h>
#include <modules/math/Vector.h>

class IGame;
class ICamera;
class JRenderer;
class FlexibleGunsight;
class Environment;
class FlightInfo;
class Targeter;
class Armament;

class GunsightModule : virtual public Object {
protected:
    float width, height;
    Vector offset;
    std::string name;
public:
    GunsightModule(const char *name, float w, float h);
    
    inline void setOffset(const Vector & ofs) { offset=ofs; }
    inline const Vector & getOffset() { return offset; }
    
    inline float getWidth() { return width; }
    inline float getHeight() { return height; }
    
    inline const std::string & getName() const { return name; }
    virtual void draw(FlexibleGunsight &)=0;
};


class FlexibleGunsight : public IDrawable {
protected:
    UI::Surface surface;
    Ptr<IGame> game;
    Ptr<ICamera> camera;
    Ptr<Environment> env;
    typedef std::vector<Ptr<GunsightModule> > Modules;
    Modules modules;
    JRenderer * renderer;
public:
    enum {
        LEFT=0x01,
        RIGHT=0x02,
        HCENTER=0x00,
        TOP=0x04,
        BOTTOM=0x08,
        VCENTER=0x00
    };

    FlexibleGunsight(Ptr<IGame>);
    void addModule(Ptr<GunsightModule>,
        std::string relative_to="screen",
        int parent_corner=0, int child_corner=0,
        Vector ofs=Vector(0,0,0), bool ofs_in_pixels=true);
        
    inline const UI::Surface & getSurface() { return surface; }
    inline JRenderer* getRenderer() { return renderer; }
    inline Ptr<ICamera> getCamera() { return camera; }
    
    // Gunsight configuration
    void addDebugInfo(Ptr<IGame>, Ptr<IActor>);
    void addBasicCrosshairs();
    void addFlightModules(Ptr<IGame>, FlightInfo &);
    void addTargeting(Ptr<IActor>, Ptr<Targeter>);
    void addArmamentToScreen(Ptr<IGame>, Armament*);
    void addDirectionOfFlight(Ptr<IActor>);
    void addInterception(Ptr<IActor> src, Ptr<IActor> target);
    
    // IDrawable implementation
    virtual void draw();
    
};

struct PlaceholderModule : public GunsightModule {
    inline PlaceholderModule(const char *name, float w, float h)
    : GunsightModule(name,w,h) { }
    
    virtual void draw(FlexibleGunsight &);
};

typedef FlexibleGunsight DefaultGunsight;

#endif
