// ---------------------------------------------------------------
// |  WeaponMapping                                              |
// ---------------------------------------------------------------


#include <modules/weaponsys/Weapon.h>

#include <IoList.h>

#include "mappings.h"

namespace {
    
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

