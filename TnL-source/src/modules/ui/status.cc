#include <algorithm>
#include <debug.h>
#include "status.h"

using namespace std;

Status::StatusPrinter::StatusPrinter() {
    last_desc = "<NO DESC>";
    last_stat = 0;
}
        
void Status::StatusPrinter::update(Status * stat) {
    if (stat->getStatus() - last_stat > 0.01
        || stat->getDescription() != last_desc)
    {
        last_desc = stat->getDescription();
        last_stat = stat->getStatus();
        ls_message("[%2.0f%%] %s\n", 100 * last_stat, last_desc.c_str());
    }
}

Status::Status() {
    status_changed.connect(SigC::slot(printer, &StatusPrinter::update));
}

void Status::beginJob(const string & desc, int steps) {
    ls_message("Beginning job: %s\n", desc.c_str());
    Job job;
    job.desc = desc;
    job.steps = steps;
    job.finished = 0;
    if (!jobs.empty() && jobs.back().finished == jobs.back().steps) {
        ls_warning("Status: job %s causes job %s to overflow.\n",
            job.desc.c_str(), jobs.back().desc.c_str());
    }
    
    jobs.push_back(job);
    status_changed.emit(this);
}

void Status::endJob() {
    if (jobs.size() == 0) return;
    Job & job = jobs.back();
    if (job.finished + 1 < job.steps) {
        ls_warning("Status: job %s ended after %d steps. Declared: %d\n",
            job.desc.c_str(), job.finished+1, job.steps);
    }
    jobs.pop_back();
    stepFinished();
}

void Status::stepFinished() {
    if (jobs.size() == 0) return;
    Job & job = jobs.back();
    job.finished++;
    if (job.finished > job.steps) {
        ls_warning("Status: job %s has overflown. (%d/%d)\n",
            job.desc.c_str(), job.finished, job.steps);
    }
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
