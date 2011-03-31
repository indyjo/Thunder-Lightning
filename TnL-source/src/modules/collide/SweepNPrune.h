#ifndef SWEEPNPRUNE_H
#define SWEEPNPRUNE_H

#include <iostream>
#include <map>
#include <set>
#include <vector>

namespace Collide {

template<class Key, class Val>
class SweepNPrune {
    struct Bound {
        inline Bound() { }
        inline Bound(Key k, Val v) : key(k), val(v) { }
        Key key;
        Val val;
    };

    typedef std::set<Key> ActiveSet;
    typedef typename ActiveSet::iterator ActiveIter;
    typedef std::vector<Bound> BoundList;
    typedef typename BoundList::iterator BoundIter;
    typedef std::map<Key, std::pair<int, int> > PlacesMap;
    typedef typename PlacesMap::iterator PlacesIter;

    ActiveSet   active_set;
    BoundList   bounds;
    PlacesMap   places;

public:
    inline void set(Key key, Val min, Val max) {
        PlacesIter pi = places.find(key);
        if (pi == places.end()) {
            bounds.push_back(Bound(key,min));
            bounds.push_back(Bound(key,max));
            places[key] = std::make_pair(bounds.size()-2, bounds.size()-1);
            pi = places.find(key);
        } else {
            bounds[pi->second.first].val = min;
            bounds[pi->second.second].val = max;
        }

        reinsert(pi->second.first);
        reinsert(pi->second.second);
    }

    inline void remove(Key key) {
        int count = 0;
        PlacesIter pi = places.find(key);
        if (pi == places.end())
            return;
        int a = pi->second.first, b = pi->second.second;
        for(int i=a; i<b-1; i++) {
            move(bounds[i+1].key, i+1, i);
            bounds[i] = bounds[i+1];
        }
        for(int i=b-1; i<bounds.size()-2; i++) {
            move(bounds[i+2].key, i+2, i);
            bounds[i] = bounds[i+2];
        }
        bounds.resize(bounds.size() - 2);
        places.erase(pi);
    }

    typedef std::vector<std::pair<Key, Key> > ContactList;

    inline void findContacts(ContactList & contacts) {
        active_set.clear();
        for(BoundIter i=bounds.begin(); i!=bounds.end(); i++) {
            ActiveIter ai = active_set.find(i->key);
            if (ai == active_set.end()) {
                for(ai = active_set.begin(); ai!=active_set.end(); ai++)
                    contacts.push_back(std::make_pair(*ai, i->key));
                active_set.insert(i->key);
            } else {
                active_set.erase(ai);
                continue;
            }
        }
    }

    friend inline std::ostream & operator<< (std::ostream & out, const SweepNPrune & s) {
        out << "[ ";
        for (int i=0; i<s.bounds.size(); i++) {
            out << "( " << s.bounds[i].val << " " << s.bounds[i].key << ") ";
        }
        out << "]";
        return out;
    }

private:
    inline void reinsert(int i) {
        while (i>0 && bounds[i-1].val > bounds[i].val) {
            swap(i-1, i);
            --i;
        }

        while (i+1 < bounds.size() && bounds[i].val > bounds[i+1].val) {
            swap(i, i+1);
            ++i;
        }
    }

    inline void swap(int i, int j) {
        move(bounds[i].key, i, j);
        move(bounds[j].key, j, i);
        std::swap(bounds[i], bounds[j]);
    }
    inline void move(Key key, int from, int to) {
        if (places[key].first == from)
            places[key].first = to;
        else
            places[key].second = to;
    }
};

} // namespace Collide

#endif
