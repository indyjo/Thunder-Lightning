// ---------------------------------------------------------------
// |  TargeterMapping                                            |
// ---------------------------------------------------------------


#include <modules/weaponsys/Targeter.h>

#include <IoList.h>

#include "mappings.h"

namespace {
    
    struct TargeterMapping
        :   public TemplatedObjectMapping<Targeter, ReinterpretCastMapping<Targeter> >
    {
        static void addMapping(Ptr<IGame> game, IoState * state) {
            IoObject *lobby = state->lobby;
            
            IoObject *self = proto(state);
            IoState_registerProtoWithFunc_(state, self, proto);
            IoObject_setSlot_to_(lobby, IOSYMBOL("Targeter"), self);
        }
        
        static IoObject *proto(void *state) {
            IoMethodTable methodTable[] = {
                {"maxRange", getMaxRange},
                {"setMaxRange", setMaxRange},
                {"currentTarget", getCurrentTarget},
                {"setCurrentTarget", setCurrentTarget},
                {"clearCurrentTarget", clearCurrentTarget},
                {"selectNextTarget", selectNextTarget},
                {"selectPreviousTarget", selectPreviousTarget},
                {"selectNextHostileTarget", selectNextHostileTarget},
                {"selectNextFriendlyTarget", selectNextFriendlyTarget},
                {"selectPreviousHostileTarget", selectPreviousHostileTarget},
                {"selectPreviousFriendlyTarget", selectPreviousFriendlyTarget},
                {"selectNearestTarget", selectNearestTarget},
                {"selectNearestHostileTarget", selectNearestHostileTarget},
                {"selectNearestFriendlyTarget", selectNearestFriendlyTarget},
                {"selectTargetInGunsight", selectTargetInGunsight},
                {NULL, NULL}
            };
            IoObject *self = IoObject_new(state);
            IoObject_tag_(self, tag(state, "Targeter"));
			IoObject_setDataPointer_(self, 0);
            IoObject_addMethodTable_(self, methodTable);
            return self;
        }
        
        CREATE_FUNC(Targeter)
        
        GET_NUMBER(getMaxRange)
        SET_FLOAT(setMaxRange)
        GETTER(Ptr<IActor>, getCurrentTarget)
        SETTER(Ptr<IActor>, setCurrentTarget)
        VOID_FUNC(clearCurrentTarget)
        VOID_FUNC(selectNextTarget)
        VOID_FUNC(selectPreviousTarget)
        VOID_FUNC(selectNextHostileTarget)
        VOID_FUNC(selectPreviousHostileTarget)
        VOID_FUNC(selectNextFriendlyTarget)
        VOID_FUNC(selectPreviousFriendlyTarget)
        VOID_FUNC(selectNearestTarget)
        VOID_FUNC(selectNearestHostileTarget)
        VOID_FUNC(selectNearestFriendlyTarget)
        VOID_FUNC(selectTargetInGunsight)
        
    };
} // namespace

template<>
void addMapping<Targeter>(Ptr<IGame> game, IoState *state) {
    TargeterMapping::addMapping(game,state);
}


template<>
Ptr<Targeter> unwrapObject<Ptr<Targeter> >(IoObject * self) {
    return TargeterMapping::getObject(self);
}

template<>
IoObject * wrapObject<Ptr<Targeter> >(Ptr<Targeter> targeter, IoState *state) {
    if (!targeter) return state->ioNil;
    IoObject *obj = TargeterMapping::create(targeter,state);
    IoState_stackRetain_(state, obj);
    return obj;
}

