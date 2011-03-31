// ---------------------------------------------------------------
// |  WeaponMapping                                              |
// ---------------------------------------------------------------

#include <modules/weaponsys/Weapon.h>

#include "mappings.h"

namespace {

    struct ConnectionContainer : public Object {
        virtual ~ConnectionContainer() { }
        
        SigC::Connection connection;
    };
    
    void onFireCallback(Ptr<IWeapon> weapon, IoObject * self, Ptr<ConnectionContainer> cc) {
        cc->connection.disconnect();
        
        IoMessage *message = IoMessage_newWithName_label_(IOSTATE, IOSYMBOL("weaponFired"),
            IOSYMBOL("fire callback"));
        
        IoObject * result = IoState_tryToPerform(IOSTATE, self, self, message);
        if (result) IoState_stackRetain_(IOSTATE, result);
        IoState_stopRetaining_(IOSTATE, self);
    }
    
    struct WeaponMapping
        :   public TemplatedObjectMapping<Weapon, ReinterpretCastMapping<Weapon> >
    {
        static void addMapping(Ptr<IGame> game, IoState * state) {
            IoObject *lobby = state->lobby;
            
            IoObject *self = proto(state);
            IoState_registerProtoWithFunc_(state, self, proto);
            IoObject_setSlot_to_(lobby, IOSYMBOL("Weapon"), self);
        }
        
        static IoObject *proto(void *state) {
            IoMethodTable methodTable[] = {
                {"name", getName},
                {"trigger", trigger},
                {"release", release},
                {"canFire", canFire},
                {"maxRounds", getMaxRounds},
                {"setMaxRounds", setMaxRounds},
                {"roundsLeft", getRoundsLeft},
                {"setRoundsLeft", setRoundsLeft},
                {"isTriggered", isTriggered},
                {"isGuided", isGuided},
                {"maxRange", maxRange},
                {"referenceSpeed", referenceSpeed},
                {"timeOfAcceleration", timeOfAcceleration},
                {"lastFiredRound", lastFiredRound},
                {"onFireNotify", onFireNotify},
                {NULL, NULL}
            };
            IoObject *self = IoObject_new(state);
            IoObject_tag_(self, tag(state, "Weapon"));
			IoObject_setDataPointer_(self, 0);
            IoObject_addMethodTable_(self, methodTable);
            return self;
        }
        
        CREATE_FUNC(Weapon)
        
        GET_STRING(getName)
        VOID_FUNC(trigger)
        VOID_FUNC(release)
        GET_BOOLEAN(canFire)
        GET_NUMBER(getMaxRounds)
        SET_INT(setMaxRounds)
        GET_NUMBER(getRoundsLeft)
        SET_INT(setRoundsLeft)
        GET_BOOLEAN(isTriggered)
        GET_BOOLEAN(isGuided)
        GET_NUMBER(maxRange)
        GET_NUMBER(referenceSpeed)
        GET_NUMBER(timeOfAcceleration)
        
        static IoObject * lastFiredRound(IoObject *self, IoObject *locals, IoMessage *m) {
            Ptr<IActor> round = getObject(self)->lastFiredRound().lock();
            if (round) {
                return wrapObject<Ptr<IActor> >(round, IOSTATE);
            } else {
                return IONIL(self);
            }
        }

        static IoObject * onFireNotify(IoObject *self, IoObject *locals, IoMessage *m) {
    		BEGIN_FUNC("onFireNotify")
    		IOASSERT(IoMessage_argCount(m) == 1, "Expected one argument")
    		IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, 0);
    		IoState_retain_(IOSTATE, arg);
    		
    		Ptr<ConnectionContainer> cc = new ConnectionContainer;
    		cc->connection = getObject(self)->onFireSig().connect(
    		    SigC::bind(SigC::slot(&onFireCallback), arg, cc));
    		return IONIL(self);
        }
    };
} // namespace

template<>
void addMapping<Weapon>(Ptr<IGame> game, IoState *state) {
    WeaponMapping::addMapping(game,state);
}


template<>
Ptr<Weapon> unwrapObject<Ptr<Weapon> >(IoObject * self) {
    return WeaponMapping::getObject(self);
}

template<>
IoObject * wrapObject<Ptr<Weapon> >(Ptr<Weapon> weapon, IoState *state) {
    if (!weapon) return state->ioNil;
    IoObject *obj = WeaponMapping::create(weapon,state);
    IoState_stackRetain_(state, obj);
    return obj;
}

