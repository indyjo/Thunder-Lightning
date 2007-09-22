#ifndef TNL_DIALOG_H
#define TNL_DIALOG_H

#include <object.h>

namespace UI {
    class MainGUI;

    class Dialog : public SigObject
    {
    public:
        virtual void enable(MainGUI & main_gui)=0;
        virtual void disable(MainGUI & main_gui)=0;
    };
}

#endif

