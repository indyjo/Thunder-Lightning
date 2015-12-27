#include <algorithm>
#include <string>
#include <interfaces/IActor.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IGame.h>
#include <modules/jogi/JRenderer.h>
#include <modules/clock/clock.h>
#include <DataNode.h>
#include <TargetInfo.h>
#include "debug.h"

void FlexibleGunsight::addDebugInfo(
	Ptr<IGame> game,
	Ptr<IActor> actor)
{
    addModule(new TargetInfoModule(game,actor),
        "screen", LEFT | BOTTOM, LEFT | BOTTOM, Vector(5,-5,0));
}

void FlexibleGunsight::addStaticDebugInfo(Ptr<IGame> game)
{
    addModule(new FPSModule(game),
        "screen", LEFT | TOP, LEFT | TOP, Vector(5,5,0));
}

void FlexibleGunsight::addProfilingGraph(Ptr<IGame> game) {
    Ptr<DataNode> debugdata = game->getDebugData();
    Ptr<TimeGraphModule> mod = new TimeGraphModule(game);
    mod->setWidth(500);
    mod->setHeight(100);
    mod->watchData(debugdata, "mainloop_1", Vector(0,0,1));
    mod->watchData(debugdata, "mainloop_8", Vector(1,0,0));
    mod->watchData(debugdata, "mainloop_9", Vector(1,0,1));
    mod->watchData(debugdata, "mainloop_11", Vector(0,1,0));
    mod->watchData(debugdata, "mainloop_sum", Vector(.5,.5,.5));
    mod->watchData(debugdata, "render_terrain", Vector(1,.5,.2));
    mod->watchData(debugdata, "render_water", Vector(0,1,1));
    mod->watchData(debugdata, "render_actors", Vector(1,1,1));
    addModule(mod, "screen", HCENTER|TOP, HCENTER|TOP);

}

FPSModule::FPSModule(Ptr<IGame> game)
:	UI::Component("fps",100,20),game(game), fps(15)
{
}

void FPSModule::draw(UI::Panel & gunsight) {
    //if (!game->debugMode()) return;
	UI::Surface surface = gunsight.getSurface();
	Ptr<IFontMan> fontman = game->getFontMan();
	Ptr<Clock> clock = game->getClock();

	surface.translateOrigin(offset[0],offset[1]);
	
	fontman->selectNamedFont("HUD_font_medium");
	
	fontman->setCursor(
		surface.getOrigin(),
		surface.getDX(),
		surface.getDY());
	fontman->setAlpha(1);
	fontman->setColor(Vector(0,1,0));
	
	char buf[16];
	float delta_t = clock->getRealFrameDelta();
	if (delta_t > 0) {
	    fps = 0.9 * fps + 0.1 / delta_t;
	}
	snprintf(buf,16,"%3.1f fps", fps);
	fontman->print(buf);
}


TargetInfoModule::TargetInfoModule(Ptr<IGame> game, Ptr<IActor> actor)
:	UI::Component("target-info", 320, 200),
	game(game),
	actor(actor)
{
}

void TargetInfoModule::draw(UI::Panel & gunsight) {
    if (!game->debugMode()) return;
	UI::Surface surface = gunsight.getSurface();
	JRenderer *r=game->getRenderer();

	surface.translateOrigin(offset[0],offset[1]);
	r->pushMatrix();
	r->multMatrix(surface.getMatrix());
	r->enableAlphaBlending();
	
	r->setColor(Vector(0,1,0));
	r->setAlpha(0.33);
	
	r->begin(JR_DRAWMODE_TRIANGLE_FAN);
	*r << Vector(0,0,0) << Vector(width,0,0)
	   << Vector(width, height,0) << Vector(0,height,0);
	r->end();
	
	r->setAlpha(1);
	r->begin(JR_DRAWMODE_CONNECTED_LINES);
	*r << Vector(0,0,0) << Vector(width-1,0,0)
	   << Vector(width-1, height-1,0) << Vector(0,height-1,0)
	   << Vector(0,0,0);
	r->end();
	
	Ptr<IFontMan> fontman = game->getFontMan();
	fontman->selectNamedFont("HUD_font_small");
	
	fontman->setCursor(
		Vector(5,5,0),
		Vector(1,0,0),
		Vector(0,1,0));
	fontman->setAlpha(1);
	fontman->setColor(Vector(1,1,1));
	fontman->print(actor->getTargetInfo()->getTargetInfo().c_str());
	
	r->disableAlphaBlending();
	r->popMatrix();
}

#define TIME_SAMPLES 500

void TimeGraphModule::TimeSeries::addValue(float val) {
    values.push_back(val);
    if (values.size() > TIME_SAMPLES) values.pop_front();
}

float TimeGraphModule::TimeSeries::maxElement() {
    if (values.empty()) return 1;
    return *std::max_element(values.begin(), values.end());
}

namespace {
    struct DataNodeValueProvider : public TimeGraphModule::IValueProvider {
        DataNodeValueProvider(Ptr<DataNode> datanode, const std::string& key)
            : datanode(datanode)
            , key(key)
        { }
        float value() { return datanode->getInt(key); }
        
        Ptr<DataNode> datanode;
        std::string key;
    };
}

void TimeGraphModule::updateSeries() {
    for(SeriesList::iterator i=series_list.begin(); i!=series_list.end(); ++i) {
        (*i)->addValue((*i)->provider->value());
    }
}

TimeGraphModule::TimeGraphModule(Ptr<IGame> game, const char *name)
    : UI::Component(name)
    , game(game)
{ }

void TimeGraphModule::watchData(Ptr<DataNode> datanode, const std::string& key, const Vector& color) {
    Ptr<TimeSeries> series = new TimeSeries;
    series->provider = new DataNodeValueProvider(datanode, key);
    series->color = color;
    
    series_list.push_back(series);
}

#define RUNNING_AVERAGE 3

void TimeGraphModule::draw(UI::Panel & panel) {
    updateSeries();
    
    if (!game->debugMode()) return;
    
    float max_value=0.01;
    for (SeriesList::iterator i=series_list.begin(); i!=series_list.end(); ++i) {
        Ptr<TimeSeries> series = *i;
        if (!series->values.empty() && series->maxElement() > max_value) {
            max_value = series->maxElement();
        }
    }
    
    UI::Surface surface = panel.getSurface();
    surface.translateOrigin(getOffset()[0], getOffset()[1]);
    surface.setWidth(getWidth());
    surface.setHeight(getHeight());
    
    surface.translateOrigin(0, surface.getHeight());
    surface.setDY( -surface.getDY() );

    surface.resize(TIME_SAMPLES, max_value);
    
	JRenderer *renderer = panel.getRenderer();
	
	renderer->pushMatrix();
	renderer->multMatrix(surface.getMatrix());
	
    for (SeriesList::iterator i=series_list.begin(); i!=series_list.end(); ++i) {
        Ptr<TimeSeries> series = *i;
        
        renderer->setAlpha(0.7);
        renderer->setColor(series->color);
        renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
        
        for(int x=RUNNING_AVERAGE-1; x<series->values.size(); ++x) {
            float y = 0;
            for (int avg=0; avg < RUNNING_AVERAGE; ++avg) y += series->values[x-avg];
            y /= RUNNING_AVERAGE;
            renderer->vertex(Vector(x,y,0));
        }
        renderer->end();
    }
    
	renderer->popMatrix();
    
}

