#include <cstdlib>
#include <iostream>
#include "SweepNPrune.h"


using namespace std;

typedef Collide::SweepNPrune<int, float> SNP;
SNP snp;

int main(int argc, char **argv) {
    if (argc==1) {
        cerr << "Usage: snp_test key0 min0 max0 [key1 min1 max1 | -key1 ...]" << endl;
        return -1;
    }

    argv++;
    argc--;

    while(argc > 0) {
        int key = atoi(argv[0]);

        if (key >= 0) {
            float m0 = atof(argv[1]);
            float m1 = atof(argv[2]);
            argv += 3;
            argc -= 3;

            cout << "Setting key " << key << " to [" << m0 << ", " << m1 << "]" << endl;
            snp.set(key, m0, m1);
        } else {
            argv += 1;
            argc -= 1;
            
            cout << "Deleting key " << -key << endl;
            snp.remove(-key);
        }

        SNP::ContactList contacts;

        snp.findContacts(contacts);
        typedef SNP::ContactList::iterator Iter;
        cout << "  " << snp << endl;
        for(Iter i=contacts.begin(); i!=contacts.end(); i++)
            cout << "  contact: keys " << i->first << " and " << i->second << endl;
    }

    return 0;
}

