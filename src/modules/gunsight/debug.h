#ifndef GUNSIGHT_DEBUG_H
#define GUNSIGHT_DEBUG_H

#include <deque>
#include <vector>
#include <modules/math/Vector.h>
#include "gunsight.h"

struct IFontMan;
struct IGame;
class DataNode;

class FPSModule : public UI::Component {
	Ptr<IGame> game;
	float fps;
public:
	FPSModule(Ptr<IGame> game);
    void draw(UI::Panel &);
};

class TargetInfoModule : public UI::Component {
	Ptr<IGame> game;
	Ptr<IActor> actor;
public:
	TargetInfoModule(Ptr<IGame>, Ptr<IActor>);
	void draw(UI::Panel &);
};

class TimeGraphModule : public UI::Component {
public:
    struct IValueProvider : public Object {
        virtual float value() = 0;
    };
private:
    struct TimeSeries : public Object {
        Ptr<IValueProvider> provider;
        std::deque<float> values;
        Vector color;
        
        void addValue(float);
        float maxElement();
    };
    
    typedef std::vector<Ptr<TimeSeries> > SeriesList;
    SeriesList series_list;
	Ptr<IGame> game;
    
    void updateSeries();
public:
    TimeGraphModule(Ptr<IGame> game, const char*name="timegraph");
    
    void watchData(Ptr<DataNode> datanode, const std::string & key, const Vector& color=Vector(1,1,1));
	void draw(UI::Panel &);
};


#endif

