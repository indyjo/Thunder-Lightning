#ifndef STATUS_H
#define STATUS_H

#include <sigc++/sigc++.h>
#include <string>
#include <list>

class Status {
    struct Job {
        std::string desc;
        int steps;
        int finished;
    };
    std::list<Job> jobs;
    SigC::Signal1<void, Status *> status_changed;
public:

    inline SigC::Signal1<void, Status *> & getSignal() {return status_changed;}

    void beginJob(const std::string & desc, int steps);
    void stepFinished();
    void endJob();

    double getStatus();
    std::string getDescription();
};


#endif
