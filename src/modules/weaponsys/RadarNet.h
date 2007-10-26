#ifndef TNL_RADARNET_H
#define TNL_RADARNET_H

#include <set>
#include <map>
#include <list>
#include <modules/math/Vector.h>
#include <Weak.h>

struct IActor;
class Targeter;

class RadarNet : public Object {
    struct Contact;
    typedef std::map<WeakPtr<IActor>, Ptr<Contact> > ContactsByActor;
    typedef std::list<Ptr<Contact> > ContactsList;
    typedef ContactsList::iterator ContactsIter;
public:
    /// This should be called periodically from within the game's main loop
    static void updateAllRadarNets(float delta_t);

    RadarNet();
    ~RadarNet();

private:
    struct Contact : public Object {
        WeakPtr<IActor> actor;      //< Actor which was reported
        /// Targeter of actor which last reported this contact or 0 for LOST and
        /// ZOMBIE contacts
        WeakPtr<Targeter> witness;
        Vector position;            //< Last known position of contact
        float age;                  //< Time passed since last valid LOS test
        ContactsIter all_iter;      //< Iterator to position in all_contacts list
        int usecount;               //< Counts by how many enumerators this contact is used
        /// The contact's verification state.
        ///  - VERIFIED is for contacts which recently have passed an LOS test by a witness
        ///  - LOST is for contacts which have recently failed an LOS test
        ///  - ZOMBIE is for contacts which are long lost but haven't yet been deleted.
        ///    They shouldn't turn up when iterating using Enumerator
        enum State {ZOMBIE,VERIFIED,LOST} state;
    };
public:
    /// Represents an iterator into the contacts list. While iterating, Enumerator
    /// will always skip over zombie contacts (long-lost contacts not yet erased
    /// from the list). However, the currently pointed-to contact may become
    /// zombie during the lifetime of the enumerator. It is therefore advised
    /// to check for contact validity using isValid() before acessing contacts
    /// from long-lived enumerators.
    /// Enumerator supports stepping through the list in both directions.
    class Enumerator {
        Ptr<RadarNet> radarnet;
        Ptr<IActor>   actor;
        ContactsIter  iter;
        
    public:
        /// Called by RadarNet::getEnumerator()
        Enumerator(Ptr<RadarNet> rn);
        Enumerator(Ptr<RadarNet> rn, ContactsIter);
        Enumerator(const Enumerator&);
        ~Enumerator();
    
        /// Advances the iterator to the next or previous valid position or to the end
        void next(bool backward=false);
        
        /// Like next(), but will wrap around the end, repeating endlessly.
        /// Will stop at the end if the list is empty.
        void cycle(bool backward=false);
        
        bool atEnd() const;
        
        /// Returns the contact's actor if valid and 0 otherwise
        Ptr<IActor> getActor() const;
        /// Returns the contact's actor regardless of validity.
        Ptr<IActor> getActorRaw() const;
        
        /// Returns whether the enumerator points to a valid position (not end)
        /// and asserts that the target is not zombie.
        bool isValid() const;
        bool isVerified() const;
        bool isLost() const;
        bool isZombie() const;
        float ageOfInformation() const;
        Vector lastKnownPosition() const;
        
        bool operator ==(const Enumerator & other);
        bool operator !=(const Enumerator & other);
        const Enumerator & operator =(const Enumerator &);

        void toBegin(bool backward=false);
        void toEnd();
    private:
        void advance(bool backward);
    };
    friend class Enumerator;
    
    /// Returns an enumerator to the beginning of the all_contacts list
    inline Enumerator getEnumerator() { return Enumerator(this); }
    /// Returns an enumerator for the given actor. It has to be checked afterwards
    /// whether the returned Enumerator is valid.
    /// @see Enumerator::isValid
    Enumerator getEnumeratorForActor(Ptr<IActor>);
    
    /// Accepts a possible contact. The RadarNet will then decide whether the
    /// contact needs a LOS test and, if yes, call the witnesses
    /// Targeter::hasLineOfSightTo() function.
    void reportPossibleContact(Ptr<IActor> actor, Ptr<Targeter> witness);
    
    /// Targeters will report their own position to the radar network, so there
    /// is no need to perform line-of-sight tests.
    void reportSelf(Ptr<Targeter> witness);
    
private:
    /// This function must be called periodically to trigger network synchronization
    /// and maintenance. Instead of updating every RadarNet sequentially, all
    /// are updated at once with a call to static member updateAllRadarNets()
    void update(float delta_t);

    void verifyContact();
    
private:
    /// Contains all contacts, whether verified or lost
    ContactsByActor contacts_by_actor;
    /// Contains all contacts, i.e. VERIFIED, LOST and ZOMBIE contacts
    ContactsList    all_contacts;
    
    /// Rotating maintenance iterator
    ContactsIter    all_iter;
    
    /// Static house-keeping container of all radar networks
    static std::set<RadarNet*> radar_nets;
};

#endif

