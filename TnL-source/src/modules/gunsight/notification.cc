#include <algorithm>
#include <vector>
#include <limits>

#include <tnl.h>
#include <interfaces/IConfig.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IFont.h>
#include <modules/actors/simpleactor.h>
#include <modules/scripting/mappings.h>
#include <modules/clock/clock.h>
#include <sound.h>

#include "gunsight.h"

struct MissileWarningModule : public UI::Component, public SigObject {
    std::vector<Ptr<IActor> > missiles;
	Ptr<SimpleActor> actor;
	Ptr<IFont> font;
    Ptr<SoundMan> soundman;
    Ptr<SoundSource> sndsource;
#ifdef HAVE_IO
    SigC::Connection connection;
#endif
	
	MissileWarningModule(const char *name, Ptr<IGame> game, Ptr<SimpleActor> actor)
	:	UI::Component(name, 250, 50),
		actor(actor),
        soundman(game->getSoundMan())
	{
#ifdef HAVE_IO
        connection = actor->message_signal.connect(
            SigC::slot(*this, &MissileWarningModule::onMessage));
        connection.block(true);
#endif
        sndsource = soundman->requestSource();
        sndsource->setLooping(true);
        game->getFontMan()->selectNamedFont("HUD_font_big");
        font = game->getFontMan()->getFont();
    }

#ifdef HAVE_IO
    void onMessage(std::string name, IoObject *args) {
        if (name == "missileShot") {
            ls_message("MissileWarningModule: Handling missileShot event\n");

            IoObject *self = args;
            IoObject *io_missile = IoObject_rawGetSlot_(args, IOSYMBOL("missile"));
            if (!io_missile)
                return;

            Ptr<IActor> missile = unwrapObject<Ptr<IActor> >(io_missile);
            Ptr<TargetInfo> ti = missile->getTargetInfo();
            
            if (ti && ti->isA(TargetInfo::GUIDED_MISSILE)) {
                missiles.push_back(missile);
            }
        } else if (name == "lockLost") {
            ls_message("MissileWarningModule: Handling lockLost event\n");
            
            IoObject *self = args;
            IoObject *io_missile = IoObject_rawGetSlot_(args, IOSYMBOL("missile"));
            if (!io_missile)
                return;

            Ptr<IActor> missile = unwrapObject<Ptr<IActor> >(io_missile);

            // find and erase the missile from the missiles list
            std::vector<Ptr<IActor> >::iterator i =
                find(missiles.begin(), missiles.end(), missile);
            
            if (i != missiles.end()) {
                missiles.erase(i);
            }
            ls_message("lockLost done.\n");
        }
    }
#endif
    void updateMissileList() {
	    int removed=0;
	    for(int i=0; i<missiles.size(); ++i) {
		    Ptr<IActor> a=missiles[i];
            if (!a->isAlive()) {
			    removed++;
		    } else if (removed>0) missiles[i-removed]=a;
	    }
	    missiles.resize(missiles.size()-removed);
        if (sndsource->isPlaying() && missiles.empty())
            sndsource->setLooping(false);
    }

    std::pair<Ptr<IActor>, float> closestMissile() {
        typedef std::vector<Ptr<IActor> >::iterator Iter;
        Ptr<IActor> closest;
        float min_dist = std::numeric_limits<float>::infinity();
        for(Iter i=missiles.begin(); i!= missiles.end(); ++i) {
            float dist = ((*i)->getLocation() - actor->getLocation()).length();
            if (dist < min_dist) {
                min_dist = dist;
                closest = *i;
            }
        }
        return std::make_pair(closest, min_dist);
    }
	
	void draw(UI::Panel & gunsight) {
        updateMissileList();

        std::pair<Ptr<IActor>, float> closest = closestMissile();
        if (!closest.first) return;

        if (!sndsource->isPlaying()) {
            sndsource->play(soundman->querySound("missile-warning.wav"));
            sndsource->setLooping(true);
        }

        float pitch = 2.0f*3000 / (closest.second + 3000);

        sndsource->setPosition(actor->getLocation());
        sndsource->setPitch(pitch);

        UI::Surface surface = gunsight.getSurface();
        surface.translateOrigin(
		    std::floor(offset[0]+width/2),
		    std::floor(offset[1]+height/2));
        JRenderer * r = gunsight.getRenderer();
	    r->pushMatrix();
	    r->multMatrix(surface.getMatrix());
	    
        font->drawString("MISSILE WARNING", Vector2(0), Vector(1,0.2,0), 1, IFont::BOTTOM|IFont::HCENTER);
        
        char buf[32];
        snprintf(buf, 32, "%.1f km", closest.second / 1000);
        font->drawString(buf, Vector2(0), Vector(1,0.7,0), 1, IFont::TOP|IFont::HCENTER);
        
        r->popMatrix();
	}
	
	void enable() {
#ifdef HAVE_IO
        connection.block(false);
#endif
	}
	
	void disable() {
#ifdef HAVE_IO
        connection.block(true);
#endif
	    sndsource->stop();
	}
};

void FlexibleGunsight::addMissileWarning(
	Ptr<IGame> game, Ptr<SimpleActor> actor)
{
    addModule(new MissileWarningModule("missile-warning", game, actor),
        "screen", HCENTER | BOTTOM, HCENTER | BOTTOM, Vector(0,-25,0));
}

namespace {
    template<class T> T smooth(T x) { return (-2*x+3)*x*x; }
}
#define FADEIN_END    0.5f
#define FADEOUT_BEGIN 6.0f
#define FADEOUT_END   8.0f

struct InfoMessageModule : public UI::Component, public SigObject {

    struct TimestampedString {
        std::string     text;
        Vector          color;
        double          age;
        TimestampedString(const char *text, const Vector & color)
            : text(text)
            , color(color)
            , age(0)
        { }
    };
    typedef std::deque<TimestampedString> TimestampedStrings;
    
    TimestampedStrings messages;
    Ptr<Clock> clock;
	Ptr<IFont> font;
    SigC::Connection connection;
    bool realtime;
	
	InfoMessageModule(const char *name, Ptr<IGame> game)
        :	UI::Component(name, game->getScreenSurface().getWidth(), game->getScreenSurface().getHeight()*0.4f)
	{
        connection = game->info_message_signal.connect(
            SigC::slot(*this, &InfoMessageModule::onMessage));
        connection.block(true);
        clock = game->getClock();
        game->getFontMan()->selectNamedFont("Game_info_message_font");
        font = game->getFontMan()->getFont();
        realtime = game->getConfig()->queryBool("InfoMessages_realtime", true);
    }

    void onMessage(const char *text, const Vector & color) {
        ls_message("InfoMessageModule message: %s\n", text);
        messages.push_front(TimestampedString(text,color));
    }

    void update(double delta_t) {
        for (TimestampedStrings::iterator it=messages.begin(); it!=messages.end(); ++it)
            it->age += delta_t;

        while (!messages.empty() && messages.back().age > FADEOUT_END) messages.pop_back();
    }

	void draw(UI::Panel & gunsight) {
	    if (realtime) {
            update(clock->getRealFrameDelta());
        } else {
            update(clock->getFrameDelta());
        }
        
        if (messages.empty())
            return;

        UI::Surface surface = gunsight.getSurface();
        surface.translateOrigin(
		    offset[0]+width/2,
		    offset[1]+height);
        JRenderer * r = gunsight.getRenderer();
	    r->pushMatrix();
	    r->multMatrix(surface.getMatrix());

        Vector2 cursor_pos(0,0);
        for (TimestampedStrings::iterator it=messages.begin(); it!=messages.end(); ++it) {
            const std::string & msg = it->text;
            const Vector & msgcol = it->color;
            float msgage = (float)it->age;
            float msgheight = 0;
            font->getStringDims(msg.c_str(), 0, &msgheight);
            float alpha = 1.0f;
            if (msgage < FADEIN_END) {
                alpha = msgage / FADEIN_END;
                msgheight *= smooth(alpha);
            } else if (msgage > FADEOUT_BEGIN) {
                alpha = 1-(msgage-FADEOUT_BEGIN)/(FADEOUT_END-FADEOUT_BEGIN);
            }
            font->drawString(msg.c_str(), cursor_pos+Vector2(+1,+1), Vector(0,0,0), 0.2*alpha, IFont::BOTTOM|IFont::HCENTER);
            font->drawString(msg.c_str(), cursor_pos+Vector2(-1,-1), Vector(0,0,0), 0.2*alpha, IFont::BOTTOM|IFont::HCENTER);
            font->drawString(msg.c_str(), cursor_pos+Vector2(-1,+1), Vector(0,0,0), 0.2*alpha, IFont::BOTTOM|IFont::HCENTER);
            font->drawString(msg.c_str(), cursor_pos+Vector2(+1,-1), Vector(0,0,0), 0.2*alpha, IFont::BOTTOM|IFont::HCENTER);
            font->drawString(msg.c_str(), cursor_pos, Vector(0,0,0), 0.2*alpha, IFont::BOTTOM|IFont::HCENTER);
            font->drawString(msg.c_str(), cursor_pos, msgcol, alpha, IFont::BOTTOM|IFont::HCENTER);

            cursor_pos[1] -= msgheight;
        }
        
        r->popMatrix();
	}
	
	void enable() {
        connection.block(false);
	}
	
	void disable() {
        connection.block(true);
        messages.clear();
	}
};

void FlexibleGunsight::addStaticInfoMessage(Ptr<IGame> game)
{
    addModule(new InfoMessageModule("info-messages", game),
        "root", HCENTER | TOP, HCENTER | TOP);
}
