#include "gunsight.h"
#include <modules/clock/clock.h>
#include <TargetInfo.h>
#include <Faction.h>
#include <modules/ui/Surface.h>
#include <typeinfo>
#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <interfaces/IPlayer.h>
#include <interfaces/ITerrain.h>
#include <interfaces/IFontMan.h>
#include <remap.h>

#define PI 3.14159265358979323846264338327

using namespace std;

Gunsight::Gunsight(IGame *thegame)
{
    this->thegame = thegame;
    ifstream in;
    JSprite spr;
    Ptr<IConfig> config( thegame->getConfig() );
    const char *texture_file=config->query("Gunsight_texture_file");

    player = thegame->getPlayer();
    terrain = thegame->getTerrain();

    this->renderer=thegame->getRenderer();
    enabled=true;
    display_info = true;

    ls_warning("Hi there querying %s!\n", texture_file);
    tex = thegame->getTexMan()->query(texture_file);
    ls_warning("Hi there!\n");
    /*in.open(texture_file);
    if (!in) ls_error("Gunsight: Couldn't open %s\n", texture_file);
    spr.load(in);
    in.close();
    renderer->createTexture(&spr.sprite,0,0,false,&txtid);*/
    //ls_error("error:%d\n",renderer->createTexture(&spr.sprite,0,0,false,&txtid));

    thegame->getEventRemapper()->map("next_target",
            SigC::slot(*this, &Gunsight::selectNextTarget));
    thegame->getEventRemapper()->map("gunsight_target",
            SigC::slot(*this, &Gunsight::selectTargetInGunsight));
    thegame->getEventRemapper()->map("toggle_gunsight_info",
            SigC::slot(*this, &Gunsight::toggleInfo));
}

void Gunsight::enable()
{
    enabled=true;
}

void Gunsight::disable()
{
    enabled=false;
}

Ptr<IActor> Gunsight::getCurrentTarget()
{
    return current_target;
}

void Gunsight::draw()
{
    if (!enabled) return;

    renderer->setCoordSystem(JR_CS_EYE);
    renderer->enableAlphaBlending();
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    renderer->disableZBuffer();
    renderer->setClipRange(0.1, 10.0);

    drawGunsight();
    drawThrustBar();
    drawFlightInfo();
    drawTargetInfo();
    drawTargets();

    Ptr<Environment> env = thegame->getEnvironment();
    renderer->setClipRange(env->getClipMin(), env->getClipMax());
    renderer->enableZBuffer();
    renderer->setCoordSystem(JR_CS_WORLD);
    renderer->disableAlphaBlending();

}

void Gunsight::drawGunsight() {
    /*
    JPoint p1(-0.25f, 0.25f,0.0f);
    JPoint p2( 0.25f, 0.25f,0.0f);
    JPoint p3(-0.25f,-0.25f,0.0f);
    JPoint p4( 0.25f,-0.25f,0.0f);
    jpoint_t move2={0.0f,0.0f,1.0f};
    jvertex_txt v1,v2,v3,v4;

    p1.add(&move2);
    p2.add(&move2);
    p3.add(&move2);
    p4.add(&move2);

    v1.p=p1.point;
    v2.p=p2.point;
    v3.p=p3.point;
    v4.p=p4.point;

    v1.txt.x=0.0f;
    v1.txt.y=0.0f;

    v2.txt.x=128.0f;
    v2.txt.y=0.0f;

    v3.txt.x=0.0f;
    v3.txt.y=128.0f;

    v4.txt.x=128.0f;
    v4.txt.y=128.0f;

    renderer->setVertexMode(JR_VERTEXMODE_TEXTURE);
    renderer->setTexture(tex);

    renderer->begin(JR_DRAWMODE_TRIANGLES);
    {
        renderer->addVertex(&v1);
        renderer->addVertex(&v2);
        renderer->addVertex(&v3);

        renderer->addVertex(&v3);
        renderer->addVertex(&v2);
        renderer->addVertex(&v4);
    }
    renderer->end();
    */

    renderer->disableTexturing();
    renderer->disableAlphaBlending();
    Vector p(0,0,1);
    Vector dx(0.02, 0, 0);
    Vector dy(0, 0.02, 0);
    Vector dx2(0.04, 0, 0);
    Vector dy2(0, 0.04, 0);
    renderer->begin(JR_DRAWMODE_LINES);
    {
        renderer->setColor(Vector(1,0,0));

        *renderer << p+dy << p+dy2 << p+dx << p+dx2
                  << p-dy << p-dy2 << p-dx << p-dx2;
    }
    renderer->end();
}

void Gunsight::drawThrustBar() {
    float throttle = player->getThrottle();
    float w = 0.02, h = 0.25, edge = 0.003, edge2 = 0.002;
    Vector p(0.2, -0.09, 1.0);
    Vector x(1,0,0), y(0,1,0);

    renderer->disableSmoothShading();
    renderer->disableTexturing();
    renderer->disableAlphaBlending();
    renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
    {
        renderer->setColor(Vector(1,0,0)); // Red

        *renderer << p << (p + h*y) << (p + h*y + w*x) << (p + w*x) << p;
    }
    renderer->end();

    Vector red(1,0,0);
    Vector yellow(1,1,0);
    renderer->enableSmoothShading();
    renderer->enableAlphaBlending();
    renderer->begin(JR_DRAWMODE_TRIANGLE_FAN);
    {
        renderer->setColor(red); // Red
        renderer->setAlpha(0.5);

        renderer->vertex(p + edge*x + edge*y);
        renderer->setColor(red + throttle*(yellow-red));
        renderer->vertex(p + edge*x + (h-edge2)*throttle*y);
        renderer->vertex(p + (h-edge2)*throttle*y + (w-edge2)*x);
        renderer->setColor(red); // Red
        renderer->vertex(p + (w-edge2)*x + edge*y);
    }
    renderer->end();
}


void Gunsight::drawFlightInfo() {
    Vector o(-0.7, -0.4, 1.0);
    float dx = (1.0/1024.0)/0.7;
    float dy = (1.0/768.0)/0.7;
    double delta_t = thegame->getClock()->getStepDelta();
    double real_delta_t = thegame->getClock()->getRealFrameDelta();
    double time_factor = thegame->getClock()->getTimeFactor();
    fi.update(delta_t, *player, *terrain);

    Ptr<IFontMan> fontman = thegame->getFontMan();
    fontman->selectFont(IFontMan::FontSpec("dungeon"));
    fontman->setCursor(o , Vector(dx,0,0), Vector(0,-dy,0));
    fontman->setColor(Vector(1,0,0));
    char buf[256];
    snprintf(buf, 256, "FPS: %3.0f\n"
                       "time factor: %3.2f\n"
                       "%s\n"
                       "Speed: %4.0f km/h\n"
                       "AoA: %4.1f?\n"
                       "Roll: %4.1f\n"
                       "Pitch: %4.1f\n"
                       "Height: %5.0f m",
            1.0/real_delta_t,
            time_factor,
            fi.collisionWarning()? "PULL UP!" : "",
            fi.getCurrentSpeed() * 3.6,
            fi.getCurrentAoA()*180/PI,
            fi.getCurrentRoll()*180/PI,
            fi.getCurrentPitch()*180/PI,
            fi.getCurrentHeight());
    fontman->print(buf);
}

void Gunsight::drawTargetInfo() {
    float aspect = 1.3;
    float focus = thegame->getCamera()->getFocus();
    UI::Surface surface = UI::Surface::FromCamera(aspect, focus, 1024, 768);
    renderer->pushMatrix();
    renderer->multMatrix(surface.getMatrix());

    Vector o(1024-300,768-200,0);
    Vector px(1,0,0);
    Vector py(0,1,0);


    renderer->enableAlphaBlending();
    renderer->disableTexturing();
    renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
    {
        renderer->setColor(Vector(0,1,0));
        renderer->setAlpha(0.5);
        *renderer << o << (o+=280*px) << (o+=180*py) << (o-=280*px)
                << (o-=180*py);
    }
    renderer->end();

    renderer->begin(JR_DRAWMODE_TRIANGLE_FAN);
    {
        renderer->setColor(Vector(0,1,0));
        renderer->setAlpha(0.2);
        *renderer << o << (o+=280*px) << (o+=180*py) << (o-=280*px);
    }
    renderer->end();
    o-=180*py;

    o+=5*px+5*py;
    Ptr<IFontMan> fontman = thegame->getFontMan();
    fontman->selectFont(IFontMan::FontSpec("dungeon"));
    fontman->setCursor(o , px, py);
    fontman->setColor(Vector(0.5,1,0.5));
    fontman->setAlpha(1.0);
    fontman->print("TARGET INFO\n");

    if (current_target && current_target->getTargetInfo()) {
        Ptr<TargetInfo> ti = current_target->getTargetInfo();

        char buf[256];
    	fontman->setAlpha(0.5);
        fontman->setCursor(o , px, py);
        fontman->print("\n[NAME]");
        fontman->setCursor(o + 80*px , px, py);
        fontman->setAlpha(1.0);
        fontman->print("\n");
        fontman->print(ti->getTargetName().c_str());

    	fontman->setAlpha(0.5);
        fontman->setCursor(o , px, py);
        fontman->print("\n\n[TYPE]");
        fontman->setCursor(o + 80*px , px, py);
        fontman->setAlpha(1.0);
        fontman->print("\n\n");
        fontman->print(ti->getTargetClass().name.c_str());

    	fontman->setAlpha(0.5);
        fontman->setCursor(o , px, py);
        fontman->print("\n\n\n[C++ class]");
        fontman->setCursor(o + 80*px , px, py);
        fontman->setAlpha(1.0);
        fontman->print("\n\n\n");
        fontman->print(typeid(*current_target).name());

    	fontman->setAlpha(0.5);
        fontman->setCursor(o , px, py);
        fontman->print("\n\n\n\n[Faction]");
        fontman->setCursor(o + 80*px , px, py);
        fontman->setAlpha(1.0);
        fontman->print("\n\n\n\n");
        fontman->print(current_target->getFaction()->getName().c_str());
        Faction::Attitude attitude = player->getFaction()->
        	getAttitudeTowards(current_target->getFaction());
        switch (attitude) {
        case Faction::FRIENDLY:
        	fontman->print(" (FRIENDLY ");
        	break;
        case Faction::NEUTRAL:
        	fontman->print(" (NEUTRAL ");
        	break;
        case Faction::HOSTILE:
        	fontman->print(" (HOSTILE ");
        	break;
        }
        fontman->print("towards ");
        fontman->print(player->getFaction()->getName().c_str());
        fontman->print(")");

    	fontman->setAlpha(0.5);
        fontman->setCursor(o , px, py);
        fontman->print("\n\n\n\n\n[INFO]\n");
        fontman->setAlpha(1.0);
        fontman->selectFont(IFontMan::FontSpec("dungeon",8));
        fontman->print(ti->getTargetInfo().c_str());
    } else {
        fontman->print("\n\n[NO TARGET SELECTED]\n");
    }

    renderer->popMatrix();
}

#include <modules/math/Plane.h>

void Gunsight::drawTargets() {
    typedef IActorStage::ActorList List;
    typedef List::const_iterator Iter;

    const List & actors = thegame->getActorList();
    float planes[6][4];

    if (current_target && current_target->getState() == IActor::DEAD) {
        current_target = 0;
    }

    thegame->getCamera()->getFrustumPlanes(planes);

    Vector cam = thegame->getCamera()->getLocation();
    Vector up, right, front;
    thegame->getCamera()->getOrientation(&up, &right, &front);

    for(Iter i=actors.begin(); i!=actors.end(); i++) {
        if (!(*i)->getTargetInfo()) continue;
        TargetInfo::TargetClass tclass = (*i)->getTargetInfo()->getTargetClass();
        if (!tclass.is_radar_detectable && !tclass.is_navigational) continue;
        Vector p = (*i)->getLocation();
        bool in_view = true;
        for(int j=0; j<6; j++) {
            if ( j!= PLANE_PLUS_Z && Plane(planes[j]).d( p ) < 0.0) {
                in_view = false;
                break;
            }
        }
        if (in_view) {
            Vector dummy;
            //in_view = !thegame->getTerrain()->lineCollides(cam, p, &dummy);
            in_view = true;
        }

        if (!in_view) continue;

//         ls_message("Target:\n"
//                 "\tclass=%d\n"
//                 "\tname=%s\n"
//                 "\ttype=%s\n"
//                 "\tpos=(%5.2f, %5.2f, %5.2f)\n",
//                 (*i)->getTargetClass(),
//                 (*i)->getTargetName().c_str(),
//                 (*i)->getTargetType().c_str(),
//                 (*i)->getTargetPosition()[0],
//                 (*i)->getTargetPosition()[1],
//                 (*i)->getTargetPosition()[2]);

        Vector dir = p - cam;
        Vector screen_pos( right * dir, up * dir, front * dir);

        float size = (*i)->getTargetInfo()->getTargetSize() / screen_pos[2];
        size = max(0.01f, size);

        screen_pos[0] /= screen_pos[2];
        screen_pos[1] /= screen_pos[2];
        screen_pos[2] = 1;

        Vector dx(size,0,0);
        Vector dy(0,size,0);
        renderer->disableSmoothShading();
        renderer->disableAlphaBlending();
        renderer->disableTexturing();

        renderer->setCullMode(JR_CULLMODE_NO_CULLING);

        if (tclass.class_id == TargetInfo::CLASS_AIMINGHELPER.class_id) {
            const float s2 = sqrt(2.0)/3.0;
            renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
            {
                renderer->setColor(Vector(0,1,0));
                *renderer << screen_pos - s2*dx + dy
                          << screen_pos + s2*dx + dy
                          << screen_pos + dx + s2*dy
                          << screen_pos + dx - s2*dy
                          << screen_pos + s2*dx - dy
                          << screen_pos - s2*dx - dy
                          << screen_pos - dx - s2*dy
                          << screen_pos - dx + s2*dy
                          << screen_pos - s2*dx + dy;

            }
            renderer->end();
        } else if (tclass.class_id == TargetInfo::CLASS_DEBUG.class_id) {
            renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
            {
                renderer->setColor(Vector(1,0,0));
                *renderer << screen_pos + dy
                          << screen_pos + dx
                          << screen_pos - dy
                          << screen_pos - dx
                          << screen_pos + dy
                          << screen_pos - dy
                          << screen_pos - dx
                          << screen_pos + dx;

            }
            renderer->end();
        } else if (tclass.is_weapon) {
            renderer->enableSmoothShading();
            renderer->begin(JR_DRAWMODE_TRIANGLES);
            {
                renderer->setColor(Vector(1,0,0));
                *renderer << screen_pos - 0.5*dx + dy
                          << screen_pos + 0.5*dx + dy;
                renderer->setColor(Vector(1,1,0));
                *renderer << screen_pos + 0.5*dy;

                renderer->setColor(Vector(1,0,0));
                *renderer << screen_pos - 0.5*dx - dy
                          << screen_pos + 0.5*dx - dy;
                renderer->setColor(Vector(1,1,0));
                *renderer << screen_pos - 0.5*dy;

                renderer->setColor(Vector(1,0,0));
                *renderer << screen_pos - dx - 0.5*dy
                          << screen_pos - dx + 0.5*dy;
                renderer->setColor(Vector(1,1,0));
                *renderer << screen_pos - 0.5*dx;

                renderer->setColor(Vector(1,0,0));
                *renderer << screen_pos + dx - 0.5*dy
                          << screen_pos + dx + 0.5*dy;
                renderer->setColor(Vector(1,1,0));
                *renderer << screen_pos + 0.5*dx;
            }
            renderer->end();
        } else {
            renderer->begin(JR_DRAWMODE_LINES);
            {
                renderer->setColor(Vector(1,0,0));
                *renderer << screen_pos - dx + 0.7*dy
                          << screen_pos - dx + dy
                          << screen_pos - dx + dy
                          << screen_pos - 0.7 * dx + dy;

                *renderer << screen_pos + 0.7*dx + dy
                          << screen_pos + dx + dy
                          << screen_pos + dx + dy
                          << screen_pos + dx + 0.7*dy;

                *renderer << screen_pos - dx - 0.7*dy
                          << screen_pos - dx - dy
                          << screen_pos - dx - dy
                          << screen_pos - 0.7 * dx - dy;

                *renderer << screen_pos + 0.7*dx - dy
                          << screen_pos + dx - dy
                          << screen_pos + dx - dy
                          << screen_pos + dx - 0.7*dy;
            }
            renderer->end();
        }

        if (current_target == *i) {
            dx *=1.2;
            dy *=1.2;
            renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
            {
                renderer->setColor(Vector(1,0,0));
                *renderer << screen_pos - dx + dy
                          << screen_pos + dx + dy
                          << screen_pos + dx - dy
                          << screen_pos - dx - dy
                          << screen_pos - dx + dy;
            }
            renderer->end();

            if (tclass.is_damageable) {
                renderer->enableAlphaBlending();
                renderer->begin(JR_DRAWMODE_TRIANGLE_STRIP);
                {
                    float health = 1.0f - current_target->getRelativeDamage();

                    renderer->setAlpha(0.5f);
                    if (health > 0.5f) {
                        renderer->setColor(Vector(2-2*health,1,0));
                    } else {
                        renderer->setColor(Vector(1,2*health,0));
                    }
                    *renderer << screen_pos - dx + 1.4*dy
                            << screen_pos - dx + 1.1*dy
                            << screen_pos -dx + (health*2*dx) + 1.4*dy
                            << screen_pos -dx + (health*2*dx) + 1.1*dy;
                }
                renderer->end();
                renderer->disableAlphaBlending();
                renderer->begin(JR_DRAWMODE_CONNECTED_LINES);
                {
                    float health = 1.0f - current_target->getRelativeDamage();

                    renderer->setColor(Vector(1,0,0));
                    *renderer << screen_pos - dx + 1.4*dy
                            << screen_pos - dx + 1.1*dy
                            << screen_pos + dx + 1.1*dy
                            << screen_pos + dx + 1.4*dy
                            << screen_pos - dx + 1.4*dy;
                }
                renderer->end();
            }

            if (display_info) {
                Ptr<IFontMan> fontman = thegame->getFontMan();
                fontman->selectFont(IFontMan::FontSpec("dungeon"));

                Vector px(1.5/1024.0,0,0);
                Vector py(0,1.5/1024.0,0);
                char buf[256];
                snprintf(buf, 256, "%s (%s)\n%s",
                        current_target->getTargetInfo()->getTargetName().c_str(),
                        tclass.name.c_str(),                        current_target->getTargetInfo()->getTargetInfo().c_str());
                fontman->setCursor(screen_pos + dx*1.1 + dy*0.8 , px, -py);
                fontman->setColor(Vector(1,1,0));
                fontman->print(buf);
            }
        }
    }
}

void Gunsight::selectNextTarget() {
    typedef IActorStage::ActorList List;
    typedef List::const_iterator Iter;

    const List & actors = thegame->getActorList();
    List viable;
    Iter i, current_i;

    for(i=actors.begin(); i!=actors.end(); i++) {
        if (targetSelectable(*i)) {
            viable.push_back(*i);
        }
    }

    if (!current_target) {
        current_target = viable.front();
        return;
    }

    // First search the current target in the list
    for(i=viable.begin(); i!=viable.end(); i++) {
        if (*i == current_target) break;
    }

    // Either current target not in viable list or viable list empty
    if (i==viable.end()) {
        current_target = 0;
        return;
    }

    if(++i != viable.end()) {
        current_target = *i;
    } else {
        current_target = viable.front();
    }
}

bool Gunsight::targetSelectable(Ptr<IActor> target) {
    return true;
    if (!target->getTargetInfo()) return false;
    TargetInfo::TargetClass tc = target->getTargetInfo()->getTargetClass();
    if (tc.is_virtual) return false;
    if (!tc.is_radar_detectable) return false;
    return true;
}

void Gunsight::selectTargetInGunsight() {
    typedef IActorStage::ActorList List;
    typedef List::const_iterator Iter;

    const List & actors = thegame->getActorList();
    Iter i, current_i;
    float nearest_angle=PI;

    Vector cam = thegame->getCamera()->getLocation();
    Vector front = thegame->getCamera()->getFrontVector();

    for(i=actors.begin(); i!=actors.end(); i++) {
        if (targetSelectable(*i)) {
            Vector pos = (*i)->getLocation();
            float angle = acos((pos - cam).normalize() * front);
            if (angle < nearest_angle) {
                nearest_angle = angle;
                current_target = *i;
            }
        }
    }

}

void Gunsight::toggleInfo() {
    display_info = !display_info;
}
