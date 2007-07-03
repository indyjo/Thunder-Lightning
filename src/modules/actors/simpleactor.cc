#include <IoState.h>
#include "simpleactor.h"
#include <interfaces/ICamera.h>
#include <interfaces/IProjectile.h>
#include <modules/clock/clock.h>
#include <modules/engines/newtonianengine.h>
#include <modules/gunsight/gunsight.h>
#include <modules/scripting/IoScriptingManager.h>
#include <modules/scripting/mappings.h>
#include <modules/weaponsys/Armament.h>
#include <modules/weaponsys/Targeter.h>
#include <remap.h>
#include <Faction.h>
#include "RelativeView.h"

SimpleActor::SimpleActor( Ptr<IGame> game)
:   thegame(game), self(0), state(ALIVE), control_mode(UNCONTROLLED), is_linked(false)
{
    engine = new NewtonianEngine(thegame);
    faction = Faction::basic_factions.none;
    target_info = new TargetInfo(
        "<unnamed SimpleActor>", 1.0f, TargetInfo::NONE);
    controls = new DataNode;
}

SimpleActor::~SimpleActor() {
}

void SimpleActor::createIoObject() {
    setIoObject(wrapObject<Ptr<SimpleActor> >( this, thegame->getIoScriptingManager()->getMainState()));
}

void SimpleActor::setIoObject(IoObject *newself) {
    if (self) {
        IoStateEx_removeCoupling(this, self);
    }
    self = newself;
    if (self) {        
        IoStateEx_coupleLifetime(this, self);
    }
}

void SimpleActor::onLinked() {
    if (!is_linked) {
        if (self && AUTOMATIC == getControlMode()) {
            message("start_ai", IONIL(self));
        }
        is_linked = true;
    }
}
void SimpleActor::onUnlinked()
{
    if (is_linked) {
        if (self && AUTOMATIC == getControlMode()) {
            message("stop_ai", IONIL(self));
        }
        is_linked = false;
    }
}

void SimpleActor::setArmament(Ptr<Armament> armament) {
    this->armament = armament;
}

Ptr<Armament> SimpleActor::getArmament() { return armament; }

void SimpleActor::setTargeter(Ptr<Targeter> targeter) {
    this->targeter = targeter;
}

Ptr<Targeter> SimpleActor::getTargeter() { return targeter; }

Ptr<EventSheet> SimpleActor::getEventSheet() {
    if (event_sheet) {
        return event_sheet;
    } else {
        event_sheet = new EventSheet();
        return event_sheet;
    }
}

void SimpleActor::mapArmamentEvents() {
    Ptr<EventSheet> sheet = getEventSheet();
    sheet->map("+primary", SigC::bind(SigC::slot(*armament, &Armament::trigger), 0));
    sheet->map("-primary", SigC::bind(SigC::slot(*armament, &Armament::release), 0));
}

void SimpleActor::mapTargeterEvents() {
    Ptr<EventSheet> sheet = getEventSheet();
    sheet->map("cycle-primary", SigC::bind(SigC::slot(*armament, &Armament::nextWeapon), 0));
    sheet->map("next-target", SigC::slot(*targeter, &Targeter::selectNextTarget));
    sheet->map("previous-target", SigC::slot(*targeter, &Targeter::selectPreviousTarget));
    sheet->map("next-hostile-target", SigC::slot(*targeter, &Targeter::selectNextHostileTarget));
    sheet->map("previous-hostile-target", SigC::slot(*targeter, &Targeter::selectPreviousHostileTarget));
    sheet->map("next-friendly-target", SigC::slot(*targeter, &Targeter::selectNextFriendlyTarget));
    sheet->map("previous-friendly-target", SigC::slot(*targeter, &Targeter::selectPreviousFriendlyTarget));
    sheet->map("nearest-target", SigC::slot(*targeter, &Targeter::selectNearestTarget));
    sheet->map("nearest-hostile-target", SigC::slot(*targeter, &Targeter::selectNearestHostileTarget));
    sheet->map("nearest-friendly-target", SigC::slot(*targeter, &Targeter::selectNearestFriendlyTarget));
    sheet->map("gunsight-target", SigC::slot(*targeter, &Targeter::selectTargetInGunsight));
}

// IActor
Ptr<TargetInfo> SimpleActor::getTargetInfo() { return target_info; }
Ptr<Faction> SimpleActor::getFaction() { return faction; }
void SimpleActor::setFaction(Ptr<Faction> fac) { faction = fac; }

void SimpleActor::action() {
    /*if (self) {
        IoState_pushRetainPool(IOSTATE);
        IoObject *slot = IOSYMBOL("action");
        if (IoObject_rawGetSlot_(self, slot)) {
            IoObject *message = IoMessage_newWithName_label_(IOSTATE, slot, IOSYMBOL("SimpleActor::action"));
            
            IoMessage_addCachedArg_(message, IONUMBER(thegame->getClock()->getFrameDelta()));

            IoException *ex=0;
            IoObject *result = IoState_tryFunc_(IOSTATE,
                (IoCatchCallback*) IoMessage_locals_performOn_,
                message, IOSTATE->lobby, self,
                &ex);
            IoState_stackRetain_(IOSTATE, result);
            if (ex) {
                IoState_exception_(IOSTATE, ex);
            }
        }
        IoState_popRetainPool(IOSTATE);
    }*/
    
    engine->run();
    if (skeleton) {
        skeleton->setRootBoneTransform(getTransform());
    }
    
    if (armament) {
        armament->action(thegame->getClock()->getFrameDelta());
    }
}
void SimpleActor::kill() {
    state = DEAD;
    if(self) {
        IoState_pushRetainPool(IOSTATE);
        message("kill", IONIL(self));
        IoState_popRetainPool(IOSTATE);
    }
}

IActor::State SimpleActor::getState() { return state; }
float SimpleActor::getRelativeDamage() { return 0.0f; }
void SimpleActor::applyDamage(float damage, int domain, Ptr<IProjectile> projectile) {
    if (self) {
        IoState_pushRetainPool(IOSTATE);
        IoObject *args = IoObject_new(IOSTATE);
        IoState_stackRetain_(IOSTATE, args);

        IoObject_setSlot_to_(args, IOSYMBOL("damage"), IONUMBER(damage));
        IoObject_setSlot_to_(args, IOSYMBOL("domain"), IONUMBER(domain));
        IoObject_setSlot_to_(args, IOSYMBOL("projectile"), wrapObject<Ptr<IActor> >(projectile, IOSTATE) );
        IoObject_setSlot_to_(args, IOSYMBOL("source"), wrapObject<Ptr<IActor> >(projectile->getSource(), IOSTATE) );

        message("applyDamage", args);

        IoState_popRetainPool(IOSTATE);
    }
}

int SimpleActor::getNumViews() { return 4; }

Ptr<IView> SimpleActor::getView(int n) { 
    // TODO: this should be implemented in Io
    
    Ptr<FlexibleGunsight> gunsight = new FlexibleGunsight(thegame);
	gunsight->addDebugInfo(thegame, this);
	if (targeter) {
	    gunsight->addTargeting(this, targeter);
	}
    gunsight->addInfoMessage(thegame);

	switch(n) {
	case 0:
	    gunsight->addBasicCrosshairs();
		return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(1,0,0),
            Vector(0,1,0),
            Vector(0,0,1),
            gunsight);
    case 1:
    	return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(-1,0,0),
            Vector(0,1,0),
            Vector(0,0,-1),
            gunsight);
    case 2:
    	return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(0,0,1),
            Vector(0,1,0),
            Vector(-1,0,0),
            gunsight);
    case 3:
    	return new RelativeView(
            this,
            Vector(0,0,0),
            Vector(0,0,-1),
            Vector(0,1,0),
            Vector(1,0,0),
            gunsight);
    default:
    	return 0;
	}
}

bool SimpleActor::hasControlMode(ControlMode m) {
  return m==UNCONTROLLED;
}
void SimpleActor::setControlMode(ControlMode m) {
    if (self && isLinked()) {
        // send notification messages to script peer
        if (AUTOMATIC!=getControlMode() && AUTOMATIC == m) {
            message("start_ai", IONIL(self));
        }
        if (AUTOMATIC==getControlMode() && AUTOMATIC != m) {
            message("stop_ai", IONIL(self));
        }
    }
    
    if (control_mode==MANUAL && m!=control_mode)  {
        thegame->getEventRemapper()->removeEventSheet(getEventSheet());
    }
    control_mode = m;
    if (m==MANUAL) {
        thegame->getEventRemapper()->addEventSheet(getEventSheet());
    }
}
IActor::ControlMode SimpleActor::getControlMode() {
    return control_mode;
}

IoObject* SimpleActor::message(std::string name, IoObject *args) {
    IoObject * result = ((IoState*)IoObject_tag(args)->state)->ioNil;
    
    if (self) {
        //IoState_pushRetainPool(IOSTATE);

        IoMessage *message = IoMessage_newWithName_label_(IOSTATE, IOSYMBOL("onMessage"),
            IOSYMBOL("SimpleActor::message"));
        
        IoMessage_addCachedArg_(message, IOSYMBOL(name.c_str()));
        IoMessage_addCachedArg_(message, args);

        result = IoState_tryToPerform(IOSTATE, self, IOSTATE->lobby, message);
        if (result) IoState_stackRetain_(IOSTATE, result);
        //IoState_popRetainPoolExceptFor_(IOSTATE, result);
    }

    message_signal.emit(name, args);

    return result;
}

IoObject* SimpleActor::getIoObject() {
    return self;
}

// IPositionProvider
Vector SimpleActor::getLocation() { return engine->getLocation(); }
Vector SimpleActor::getFrontVector() { return engine->getFrontVector(); }
Vector SimpleActor::getRightVector() { return engine->getRightVector(); }
Vector SimpleActor::getUpVector() { return engine->getUpVector(); }
void SimpleActor::getOrientation(Vector *up, Vector *right, Vector *front) {
    engine->getOrientation(up, right, front);
}
Matrix3 SimpleActor::getOrientationAsMatrix() {
    Vector up, right, front;
    engine->getOrientation(&up, &right, &front);
    return MatrixFromColumns(right, up, front);
}
Quaternion SimpleActor::getOrientationAsQuaternion() {
    Quaternion q;
    q.fromMatrix(getOrientationAsMatrix());
    return q;
}
Transform SimpleActor::getTransform() {
    return Transform(getOrientationAsQuaternion(), getLocation());
}

// IMovementProvider
Vector SimpleActor::getMovementVector() { return engine->getMovementVector(); }

// IPositionReceiver
void SimpleActor::setLocation(const Vector & p) { engine->setLocation(p); }
void SimpleActor::setOrientation(   const Vector & up,
                                    const Vector & right,
                                    const Vector & front) {
    engine->setOrientation(up, right, front);
}
void SimpleActor::setOrientation(   const Matrix3& m) {
    engine->setOrientation(m*Vector(0,1,0), m*Vector(1,0,0), m*Vector(0,0,1));
}
void SimpleActor::setOrientation(   const Quaternion& q) {
    engine->setOrientation(q.rot(Vector(0,1,0)), q.rot(Vector(1,0,0)), q.rot(Vector(0,0,1)));
}
void SimpleActor::setTransform( const Transform & xform) {
    setOrientation(xform.quat());
    setLocation(xform.vec());
}

// IMovementReceiver
void SimpleActor::setMovementVector(const Vector & v) {
    engine->setMovementVector(v);
}

// IDrawable
void SimpleActor::draw() {
    JRenderer *renderer = thegame->getRenderer();
    
    float bounding_radius = 1.0;
    if (skeleton) {
        bounding_radius = skeleton->getBoundingRadius();
    } else if (getTargetInfo()) {
        bounding_radius = getTargetInfo()->getTargetSize();
    }
    
    float frustum[6][4];
    float dist = 0.0;
    Vector p = getLocation();

    thegame->getCamera()->getFrustumPlanes(frustum);
    for(int plane=0; plane<6; plane++) {
        float d = 0;
        for(int i=0; i<3; i++) d += frustum[plane][i]*p[i];
        d += frustum[plane][3];
        if (d < -bounding_radius) return; // Out of frustum -> cull!
        if (plane == PLANE_MINUS_Z) dist = d;
    }
    
    float max_model_distance = bounding_radius*renderer->getFocus() * renderer->getHeight() / 4;
    float max_point_distance = 2*max_model_distance;
    
    if (dist > max_point_distance) {
        return; // too far away -> cull
    } else if ( dist > max_model_distance) {
        // draw as point
        renderer->disableTexturing();
        renderer->enableAlphaBlending();
        renderer->setColor(Vector(0,0,0));
        renderer->setAlpha(1.0 - (dist-max_model_distance) /
                (max_point_distance - max_model_distance));
        renderer->begin(JR_DRAWMODE_POINTS);
        renderer->vertex(p);
        renderer->end();
        renderer->disableAlphaBlending();
        return;
    }

    
    renderer->enableLighting();
    if (model) {
        Vector right, up, front;
        getOrientation(&up, &right, &front);
        Matrix Translation = TranslateMatrix<4,float>(getLocation());
        Matrix Rotation    = Matrix::Hom(
            MatrixFromColumns(right, up, front));

        Matrix Mmodel  = Translation * Rotation;
        model->draw(*renderer, Mmodel, Rotation);
        renderer->disableLighting();
    }
    if (skeleton) {
        skeleton->draw(*renderer);
    }
    renderer->disableLighting();

}

