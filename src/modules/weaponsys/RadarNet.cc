#include <interfaces/IActor.h>
#include <debug.h>
#include "Targeter.h"
#include "RadarNet.h"

#define SECONDS_TILL_REVERIFICATION 2
#define SECONDS_LOST_TILL_REMOVAL   5
#define OPERATIONS_PER_SECOND       20

/// Global set of radar networks definition
std::set<RadarNet*> RadarNet::radar_nets;

RadarNet::Enumerator::Enumerator(Ptr<RadarNet> rn) {
    radarnet = rn;
    iter = radarnet->all_contacts.begin();
    if (!atEnd()) {
        actor = (*iter)->actor.lock();
        ++(*iter)->usecount;
        if (!actor || isZombie()) {
            next();
        }
    }
}

RadarNet::Enumerator::Enumerator(const RadarNet::Enumerator & other) {
    radarnet = other.radarnet;
    iter = other.iter;
    actor = other.actor;
    if (!atEnd()) ++(*iter)->usecount;
}

RadarNet::Enumerator::~Enumerator() {
    if (!atEnd()) --(*iter)->usecount;
}

void RadarNet::Enumerator::next() {
    do {
        advance();
    } while ( (!actor || isZombie()) && !atEnd() );
}

void RadarNet::Enumerator::cycle() {
    Enumerator old = *this;
    next();
    if (atEnd()) {
        toBegin();
    }
    while ( !atEnd() && !(actor = (*iter)->actor.lock()) ) {
        advance();
    }
}

bool RadarNet::Enumerator::atEnd() const { return iter == radarnet->all_contacts.end(); }
Ptr<IActor> RadarNet::Enumerator::getActor() const { return actor; }
bool RadarNet::Enumerator::isVerified() const { return (*iter)->state == Contact::VERIFIED; }
bool RadarNet::Enumerator::isZombie() const { return (*iter)->state == Contact::ZOMBIE; }
float RadarNet::Enumerator::ageOfInformation() const { return (*iter)->age; }
Vector RadarNet::Enumerator::lastKnownPosition() const { return (*iter)->position; }
bool RadarNet::Enumerator::operator ==(const RadarNet::Enumerator & other) {
    return other.iter == iter;
}
bool RadarNet::Enumerator::operator !=(const RadarNet::Enumerator & other) {
    return other.iter != iter;
}
//const Enumerator & operator =(const Enumerator &);

void RadarNet::Enumerator::advance() {
    if (atEnd()) return;
    
    --(*iter)->usecount;
    ++iter;
    if (atEnd()) {
        actor = 0;
    } else {
        actor = (*iter)->actor.lock();
        ++(*iter)->usecount;
    }
}

void RadarNet::Enumerator::toBegin() {
    if (!atEnd()) --(*iter)->usecount;
    iter = radarnet->all_contacts.begin();
    if (atEnd()) {
        actor = 0;
    } else {
        actor = (*iter)->actor.lock();
        ++(*iter)->usecount;
        if (!actor || isZombie()) {
            next();
        }
    }
}

RadarNet::RadarNet() {
    all_iter = all_contacts.begin();
    
    radar_nets.insert(this);
}

RadarNet::~RadarNet() {
    radar_nets.erase(this);
}

void RadarNet::updateAllRadarNets(float delta_t) {
    typedef std::set<RadarNet*>::iterator Iter;
    for(Iter i= radar_nets.begin(); i!= radar_nets.end(); ++i) {
        (*i)->update(delta_t);
    }
}

void RadarNet::reportPossibleContact(Ptr<IActor> actor, Ptr<Targeter> witness) {
    ContactsByActor::iterator i = contacts_by_actor.find(actor);

    if (i == contacts_by_actor.end()) {
        // This is a new, i.e. formerly unreported possible radar contact.
        // If the LOS test succeeds, we can add it to verified_contacts
        // and all_contacts
        if (witness->hasLineOfSightTo(actor)) {
            Ptr<Contact> contact = new Contact;
            contacts_by_actor[actor] = contact;
            contact->actor      = actor;
            contact->witness    = witness;
            contact->position   = actor->getLocation();
            contact->age        = 0;
            contact->usecount   = 0;
            contact->state      = Contact::VERIFIED;
            
            all_contacts.push_back(contact);
            contact->all_iter   = --all_contacts.end();
        }
    } else {
        // We already know this contact. If it is a lost or zombie contact, perform a
        // LOS test. If that succeeds, we can mark this contact as verified
        // again.
        Ptr<Contact> contact = i->second;
        if (contact->state != Contact::VERIFIED && witness->hasLineOfSightTo(actor)) {
            contact->witness = witness;
            contact->age = 0;
            contact->state = Contact::VERIFIED;
        }
    }
}

void RadarNet::reportSelf(Ptr<Targeter> witness) {
    Ptr<IActor> actor = &witness->getSubjectActor();
    ContactsByActor::iterator i = contacts_by_actor.find(actor);

    if (i == contacts_by_actor.end()) {
        // This is the first time the contact reports itself
        
        Ptr<Contact> contact = new Contact;
        contacts_by_actor[actor] = contact;
        contact->actor      = actor;
        contact->witness    = witness;
        contact->position   = actor->getLocation();
        contact->age        = 0;
        contact->usecount   = 0;
        contact->state      = Contact::VERIFIED;
        
        all_contacts.push_back(contact);
        contact->all_iter   = --all_contacts.end();
    } else {
        // We already know this contact. Just refresh it.
        Ptr<Contact> contact = i->second;
        contact->witness = witness;
        contact->age = 0;
        contact->state = Contact::VERIFIED;
    }
}

void RadarNet::update(float delta_t) {
    for(ContactsIter i= all_contacts.begin(); i!= all_contacts.end(); ++i) {
        (*i)->age += delta_t;
    }
    
    size_t num_ops = (size_t) (0.9999f + OPERATIONS_PER_SECOND*delta_t);
    num_ops = std::min(num_ops, all_contacts.size());
    
    // perform the calculated number of contact operations
    while(num_ops-- > 0) {
        if (all_iter == all_contacts.end()) {
            all_iter = all_contacts.begin();
        }
        
        // contacts whose actor has died are removed
        Ptr<IActor> actor = (*all_iter)->actor.lock();
        if ( !actor || !actor->isAlive()) {
            (*all_iter)->state = Contact::ZOMBIE;
        }
        
        // verified contacts old enough are reverified
        if ((*all_iter)->state == Contact::VERIFIED &&
            (*all_iter)->age > SECONDS_TILL_REVERIFICATION)
        {
            verifyContact();
        }
        
        // lost contacts old enough are removed
        if ((*all_iter)->state == Contact::LOST &&
            (*all_iter)->age > SECONDS_LOST_TILL_REMOVAL)
        {
            (*all_iter)->state = Contact::ZOMBIE;
        }
        
        // zombie contacts with zero usecount are erased from the list
        if ((*all_iter)->state == Contact::ZOMBIE &&
            (*all_iter)->usecount == 0)
        {
            contacts_by_actor.erase((*all_iter)->actor);
            all_contacts.erase(all_iter++);
        } else
        {
            ++all_iter;
        }
    }
}

void RadarNet::verifyContact() {
    Ptr<Contact> contact = *all_iter;
    Ptr<IActor> actor = contact->actor.lock();
    Ptr<Targeter> witness = contact->witness.lock();
    
    if (!actor) return;
    
    if (witness && witness->hasLineOfSightTo(actor)) {
        contact->age = 0;
    } else {
        // this is now a lost contact
        contact->witness = 0;
        contact->age = 0;
        contact->state = Contact::LOST;
    }
}

