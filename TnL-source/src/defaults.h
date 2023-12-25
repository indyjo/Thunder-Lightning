#ifndef DEFAULTS_H
#define DEFAULTS_H
#include <object.h>

class EventRemapper;
class IConfig;

void initialize_config(Ptr<IConfig> config);
void initialize_input(Ptr<EventRemapper> remapper);

#endif
