#ifndef SCRIPTING_COMMANDLINE_H
#define SCRIPTING_COMMANDLINE_H

#include <string>

#include "Environment.h"

namespace Scripting {
    struct CommandLine : public Object {
        Ptr<Environment> env;

        inline CommandLine(Ptr<Environment> env) : env(env) { }
        std::string eval(const std::string & command_line);
    };
} // namespace Scripting

#endif
