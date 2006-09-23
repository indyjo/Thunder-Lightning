// ---------------------------------------------------------------
// |  ArmamentMapping                                            |
// ---------------------------------------------------------------


#include <modules/weaponsys/Armament.h>

#include <IoList.h>

#include "mappings.h"

namespace {
    
    struct ArmamentMapping
        :   public TemplatedObjectMapping<Armament, ReinterpretCastMapping<Armament> >
    {
        static void addMapping(Ptr<IGame> game, IoState * state) {
            IoObject *lobby = state->lobby;
            
            IoObject *self = proto(state);
            IoState_registerProtoWithFunc_(state, self, proto);
            IoObject_setSlot_to_(lobby, IOSYMBOL("Armament"), self);
        }
        
        static IoObject *proto(void *state) {
            IoMethodTable methodTable[] = {
                {"weaponNames", weaponNames},
                {"weapon", weapon},
                {NULL, NULL}
            };
            IoObject *self = IoObject_new(state);
            self->tag = tag(state, "Armament");
            self->data = 0;
            IoObject_addMethodTable_(self, methodTable);
            return self;
        }
        
        CREATE_FUNC(Armament)
        
        static IoObject *weaponNames(IoObject *self, IoObject*locals, IoObject*m) {
            BEGIN_FUNC("Armament.weaponNames")
            typedef std::vector<Ptr<Weapon> > Weapons;
            Weapons weapons = getObject(self)->getAllWeapons();
            IoObject *list = IoList_new(IOSTATE);
            for (Weapons::iterator i = weapons.begin(); i!= weapons.end(); ++i) {
                IoList_rawAppend_(list,
                    IoState_symbolWithCString_(IOSTATE, (*i)->getName()));
            }
            return list;
        }
        
        static IoObject *weapon(IoObject *self, IoObject*locals, IoObject*m) {
            BEGIN_FUNC("Armament.weapon")
            char * weapon_name = IoMessage_locals_cStringArgAt_(m,locals,0);
            Ptr<Weapon> w = getObject(self)->getWeapon(weapon_name);
            return wrapObject<Ptr<Weapon> >(w, IOSTATE);
        }
    };
} // namespace

template<>
void addMapping<Armament>(Ptr<IGame> game, IoState *state) {
    ArmamentMapping::addMapping(game,state);
}


template<>
Ptr<Armament> unwrapObject<Ptr<Armament> >(IoObject * self) {
    return ArmamentMapping::getObject(self);
}

template<>
IoObject * wrapObject<Ptr<Armament> >(Ptr<Armament> armament, IoState *state) {
    if (!armament) return state->ioNil;
    IoObject *obj = ArmamentMapping::create(armament,state);
    IoState_stackRetain_(state, obj);
    return obj;
}

