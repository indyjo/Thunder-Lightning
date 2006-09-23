#include <vector>
#include <limits>

#include <landscape.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IFont.h>
#include <modules/actors/simpleactor.h>
#include <modules/scripting/mappings.h>
#include <modules/clock/clock.h>
#include <sound.h>

#include "gunsight.h"

struct MissileWarningModule : public GunsightModule, public SigObject {
    std::vector<Ptr<IActor> > missiles;
	Ptr<SimpleActor> actor;
	Ptr<IFontMan> fontman;
    Ptr<SoundMan> soundman;
    Ptr<SoundSource> sndsource;
	
	MissileWarningModule(const char *name, Ptr<IGame> game, Ptr<SimpleActor> actor)
	:	GunsightModule(name, 200, 25),
		actor(actor),
        fontman(game->getFontMan()),
        soundman(game->getSoundMan())
	{
        actor->message_signal.connect(
            SigC::slot(*this, &MissileWarningModule::onMessage));
        sndsource = soundman->requestSource();
        sndsource->setLooping(true);
    }

    void onMessage(std::string name, IoObject *args) {
        if (name != "missileShot")
            return;

        ls_message("MissileWarningModule: Handling missileShot event\n");

        IoObject *self = args;
        IoObject *io_missile = IoObject_rawGetSlot_(args, IOSYMBOL("missile"));
        if (!io_missile)
            return;

        Ptr<IActor> missile = unwrapObject<Ptr<IActor> >(io_missile);

        missiles.push_back(missile);

        if (!sndsource->isPlaying()) {
            sndsource->play(soundman->querySound("missile-warning.wav"));
            sndsource->setLooping(true);
        }
    }

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
	
	void draw(FlexibleGunsight & gunsight) {
        updateMissileList();

        std::pair<Ptr<IActor>, float> closest = closestMissile();
        if (!closest.first) return;

        float pitch = 2.0f*3000 / (closest.second + 3000);

        sndsource->setPosition(actor->getLocation());
        sndsource->setPitch(pitch);

		UI::Surface surface = gunsight.getSurface();
		surface.translateOrigin(offset[0],offset[1]);
		
		fontman->selectFont(IFontMan::FontSpec(
            "dungeon", 12, IFontMan::FontSpec::BOLD));
		
		fontman->setCursor(
			surface.getOrigin(),
			surface.getDX(),
			surface.getDY());
		fontman->setAlpha(1);
		fontman->setColor(Vector(1,0.2,0));
		
		char buf[256];
        snprintf(buf,256,"MISSILE WARNING\n", closest.second);
		fontman->print(buf);
	}
};

void FlexibleGunsight::addMissileWarning(
	Ptr<IGame> game, Ptr<SimpleActor> actor)
{
    addModule(new MissileWarningModule("missile-warning", game, actor),
        "screen", HCENTER | BOTTOM, HCENTER | BOTTOM, Vector(0,5,0));
}

namespace {
    template<class T> T smooth(T x) { return (-2*x+3)*x*x; }
}
#define FADEIN_END    0.5f
#define FADEOUT_BEGIN 6.0f
#define FADEOUT_END   8.0f

struct InfoMessageModule : public GunsightModule, public SigObject {

    typedef std::pair<std::string, double> TimestampedString;
    typedef std::deque<TimestampedString> TimestampedStrings;
    
    TimestampedStrings messages;
    Ptr<Clock> clock;
	Ptr<IFont> font;
	
	InfoMessageModule(const char *name, Ptr<IGame> game, Ptr<SimpleActor> actor)
        :	GunsightModule(name, game->getScreenSurface().getWidth(), game->getScreenSurface().getHeight()*0.4f)
	{
        actor->message_signal.connect(
            SigC::slot(*this, &InfoMessageModule::onMessage));
        clock = game->getClock();
        game->getFontMan()->selectFont(IFontMan::FontSpec("dungeon", 12, IFontMan::FontSpec::BOLD));
        font = game->getFontMan()->getFont();
    }

    void onMessage(std::string name, IoObject *args) {
        ls_message("InfoMessageModule: Handling %s event\n", name.c_str());
        if (name != "infoMessage")
            return;

        ls_message("InfoMessageModule: Handling infoMessage event\n");

        IoObject *self = args;
        IoObject *io_text = IoObject_rawGetSlot_(args, IOSYMBOL("text"));
        if (!io_text)
            return;

        std::string text = unwrapObject<std::string>(io_text);

        messages.push_front(std::make_pair(text, 0.0));

    }

    void update(double delta_t) {
        for (TimestampedStrings::iterator it=messages.begin(); it!=messages.end(); ++it)
            it->second += delta_t;

        while (!messages.empty() && messages.back().second > FADEOUT_END) messages.pop_back();
    }

	void draw(FlexibleGunsight & gunsight) {
        update(clock->getRealFrameDelta());
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
            const std::string & msg = it->first;
            float msgheight = 0;
            font->getStringDims(msg.c_str(), 0, &msgheight);
            float msgage = (float)it->second;
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
            font->drawString(msg.c_str(), cursor_pos, Vector(1,1,1), alpha, IFont::BOTTOM|IFont::HCENTER);

            cursor_pos[1] -= msgheight;
        }
        
        r->popMatrix();


	}
};

void FlexibleGunsight::addInfoMessage(Ptr<IGame> game, Ptr<SimpleActor> actor)
{
    addModule(new InfoMessageModule("info-messages", game, actor),
        "screen", HCENTER | TOP, HCENTER | TOP);
}
