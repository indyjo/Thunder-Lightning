#include <IoNil.h>
#include <IoNumber.h>
#include <IoState.h>
#include <IoObject.h>
#include <interfaces/IGame.h>
#include <modules/scripting/IoScriptingManager.h>
#include <modules/scripting/mappings.h>
#include "controls.h"

using namespace std;

Controls::Controls(IGame *thegame) {
    IoState * state = thegame->getIoScriptingManager()->getMainState();
    self = IoObject_new(state);
    IoState_retain_(state, self);
}

Controls::Controls(IoState *state) {
    self = IoObject_new(state);
    IoState_retain_(state, self);
}

Controls::~Controls() {
    // TODO: I am not quite sure how to handle the case where the IoStage object has been deleted.
    //       In that case,  the self object has been deleted,  too,  which we can't prevent, since
    //       retaining works via IoState_retain_ and retained variables are kept in the IoState.
    //       Maybe some global test whether the IoState is alive would be possible.
    //       Testing for magic addresses is a hack and not guaranteed to work, esp. cross platform.
    if (IOSTATE != (IoState*)0xDDDDDDDD && IOSTATE != 0x00000000) {
        //ls_message("Removing Io object from state %p\n",IOSTATE);
        IoState_release_(IOSTATE, self);
    }
}

bool Controls::getBool(const string & name) const throw(NoSuchControl) {
    IoObject *slot = IoObject_rawGetSlot_(self, IOSYMBOL(name.c_str()));
    if (!slot) {
        throw NoSuchControl();
    }
    return !ISNIL(slot);
}

bool Controls::getBool(const string & name, bool default_value) {
    try { return getBool(name); }
    catch(NoSuchControl) { return default_value; }
}

void Controls::setBool(const string & name, bool val) {
    IoObject_setSlot_to_(self, IOSYMBOL(name.c_str()),
        val?IONOP(self):IONIL(self));
}

int Controls::getInt(const string & name) const throw(NoSuchControl) {
    IoObject *slot = IoObject_rawGetSlot_(self, IOSYMBOL(name.c_str()));
    if (!slot || !ISNUMBER(slot))
        throw NoSuchControl();
    return IoNumber_asInt(slot);
}

int Controls::getInt(const string & name, int default_value) {
    try { return getInt(name); }
    catch(NoSuchControl) { return default_value; }
}

void Controls::setInt(const string & name, int val) {
    IoObject_setSlot_to_(self, IOSYMBOL(name.c_str()),
        IONUMBER(val));
}

float Controls::getFloat(const string & name) const throw(NoSuchControl) {
    IoObject *slot = IoObject_rawGetSlot_(self, IOSYMBOL(name.c_str()));
    if (!slot || !ISNUMBER(slot))
        throw NoSuchControl();
    return IoNumber_asFloat(slot);
}

float Controls::getFloat(const string & name, float default_value) {
    try { return getFloat(name); }
    catch(NoSuchControl) { return default_value; }
}

void Controls::setFloat(const string & name, float val) {
    IoObject_setSlot_to_(self, IOSYMBOL(name.c_str()),
        IONUMBER(val));
    //IoObject_print(self);
}

Vector Controls::getVector(const string & name) const throw(NoSuchControl) {
    IoObject *slot = IoObject_rawGetSlot_(self, IOSYMBOL(name.c_str()));
    if (!slot)
        throw NoSuchControl();
    return unwrapObject<Vector>(slot);
}

Vector Controls::getVector(const string & name, Vector default_value) {
    try { return getVector(name); }
    catch(NoSuchControl) { return default_value; }
}

void Controls::setVector(const string & name, Vector val) {
    IoObject_setSlot_to_(self, IOSYMBOL(name.c_str()),
        wrapObject<Vector>(val, IOSTATE));
    //IoObject_print(self);
}
