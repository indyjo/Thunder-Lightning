#include "status.h"

using namespace std;

void Status::beginJob(const string & desc, int steps) {
    Job job;
    job.desc = desc;
    job.steps = steps;
    job.finished = 0;
    jobs.push_back(job);
    status_changed.emit(this);
}

void Status::endJob() {
    if (jobs.size() == 0) return;
    jobs.pop_back();
    if(jobs.size() > 0) jobs.back().finished++;
    status_changed.emit(this);
}

void Status::stepFinished() {
    if (jobs.size() == 0) return;
    jobs.back().finished++;
    status_changed.emit(this);
}

double Status::getStatus() {
    double status = 0.0;
    double step = 1.0;
    
    for(list<Job>::iterator i=jobs.begin(); i!=jobs.end(); i++) {
        step /= i->steps;
        status += step * i->finished;
    }
    
    return status;
}

string Status::getDescription() {
    if (jobs.size() == 0) return string();
    return jobs.back().desc;
}
