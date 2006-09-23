// signals.h
#ifndef SIGNALS_H
#define SIGNALS_H

// ActionSignal
typedef SigC::Signal0<void> ActionSignal;
typedef SigC::Slot0<void> ActionSlot;

// SwitchSignal
// on/off = true/false
typedef SigC::Signal1<void, bool> SwitchSignal;
typedef SigC::Slot1<void, bool> SwitchSlot;

// KeyboardSignal
// Keycode, pressed
typedef SigC::Signal2<void, int, bool> KeyboardSignal;
typedef SigC::Slot2<void, int, bool> KeyboardSlot;

// MouseMoveSignal
// x,y
typedef SigC::Signal2<void, int, int> MouseMoveSignal;
typedef SigC::Slot2<void, int, int> MouseMoveSlot;

#endif
