#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <landscape.h>
#include "status.h"

class LoadingScreen : public SigC::Object {
public:
    LoadingScreen(IGame *, const std::string & bgtex_name);
    void update(Status *);
private:
    IGame *thegame;
    float last_status;
    TexPtr background;
    Ptr<IFontMan> fontman;
};

#endif
