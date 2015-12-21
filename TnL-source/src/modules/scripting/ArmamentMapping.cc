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
        static const char *const id;
        
        static void addMapping(Ptr<IGame> game, IoState * state) {
            IoObject *lobby = state->lobby;
            
            IoObject *self = proto(state);
            IoState_registerProtoWithId_(state, self, id);
            IoObject_setSlot_to_(lobby, IOSYMBOL("Armament"), self);
        }
        
        static IoObject *proto(void *state) {
            IoMethodTable methodTable[] = {
                {"weaponNames", weaponNames},
                {"weapon", weapon},
                {"selectWeapon", selectWeapon},
                {NULL, NULL}
            };
            IoObject *self = IoObject_new(state);
            IoObject_tag_(self, tag(state, "Armament"));
            IoObject_setDataPointer_(self, 0);
            IoObject_addMethodTable_(self, methodTable);
            return self;
        }
        
        CREATE_FUNC(Armament, id)
        
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

        static IoObject *selectWeapon(IoObject *self, IoObject *locals, IoObject *m) {
            BEGIN_FUNC("Armament.selectWeapon")
            IOASSERT(IoMessage_argCount(m) == 1, "selectWeapon accepts 1 argument");
            Ptr<Weapon> w = unwrapObject<Ptr<Weapon> >(IoMessage_locals_valueArgAt_(m, locals, 0));
            getObject(self)->selectWeapon(0, w);
            return IOSTATE->ioNil;
        }
    };
    
    const char *const ArmamentMapping::id = "Armament";
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

