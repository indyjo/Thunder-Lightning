#include <iostream>
#include <set>
#include "AStar.h"

using namespace std;
using namespace AI;

class MyTerrain : public AdaptiveTerrain {
    typedef pair<int,int> Coord;
    set<Coord> walls;
    
public:
    MyTerrain() {
        walls.insert(Coord(3,1));
        walls.insert(Coord(3,0));
        walls.insert(Coord(3,-1));
        walls.insert(Coord(3,2));
        walls.insert(Coord(6,2));
        walls.insert(Coord(6,1));
        walls.insert(Coord(6,0));
        walls.insert(Coord(6,-1));
        walls.insert(Coord(5,-1));
        walls.insert(Coord(4,-1));
        walls.insert(Coord(7,-2));
        walls.insert(Coord(8,-3));
        walls.insert(Coord(2,3));
        walls.insert(Coord(7,-4));
    };
    
    virtual float cost(const Node & n1, const Node & n2) {
        if (passable(n1.x, n1.y, n2.x, n2.y)) return estimate(n1,n2);
        else return -1.0f;
    }
    
    bool passable(int x1, int y1, int x2, int y2) {
        if (wallAt(x2,y2)) return false;
        int dx = x2-x1;
        int dy = y2-y1;
        if (dx!=0 && dy!=0) { // diagonal
            return !wallAt(x1+dx,y1) && !wallAt(x1, y1+dy);
        }
        return true;
    }
    
    bool wallAt(int x, int y) {
        return walls.end() != walls.find(Coord(x, y));
    }
    
    friend ostream& operator<<(ostream & out, const MyTerrain & t);
};

ostream& operator<<(ostream & out, const MyTerrain & t)
{
    out << "MyTerrain: (" << endl;
    out << "  origin_x: " << t.origin_x << endl;
    out << "  origin_y: " << t.origin_y << endl;
    out << "  w: " << t.w << endl;
    out << "  h: " << t.h << endl;
    for(int y=t.h-1; y>=0; y--) {
        out << "    ";
        for(int n=0; n<16-t.origin_x; n++) out << " ";
        for(int x=0; x<t.w; x++) {
            Node * n = t.nodes[y*t.w + x];
            char c;
            if (t.walls.end() != t.walls.find(
                MyTerrain::Coord(x-t.origin_x,y-t.origin_y))) { 
                c = '#';
            } else if (n && n->parent) {
                int dx = n->parent->x - n->x;
                int dy = n->parent->y - n->y;
                if (dx+dy == 0) c = '\\';
                else if (dx-dy == 0) c = '/';
                else if (dx == 0) c = '|';
                else c = '-';
            } else {
                c = '·';
            }
            out << c;
        }
        out << endl;
    }
    out << ")";
    
    return out;
}


int main() {
    AdaptiveTerrain t(4,4,2,2);
    cout << t << endl;;
    t.get(0,0);
    cout << t << endl;;
    t.get(-1,0);
    cout << t << endl;;
    t.get(-2,0);
    cout << t << endl;;
    t.get(-3,0);
    cout << t << endl;;
    t.get(-4,0);
    cout << t << endl;;
    t.get(-5,0);
    cout << t << endl;;
    t.get(-6,0);
    cout << t << endl;;
    t.get(-7,0);
    cout << t << endl;;
    t.get(1,0);
    cout << t << endl;;
    t.get(2,0);
    cout << t << endl;;
    t.get(2,1);
    cout << t << endl;;
    t.get(2,2);
    cout << t << endl;;
    t.get(2,3);
    cout << t << endl;;
    t.get(2,-1);
    cout << t << endl;;
    t.get(2,-2);
    cout << t << endl;;
    Node * n = t.get(2,-3);
    cout << t << endl;;
    Neighbors neighbors;
    t.neighbors(*n, &neighbors);
    cout << t << endl;
    
    cout << "---------------------------------------------------------------";
    cout << endl;
    
    MyTerrain t2;
    PathFinder pf(&t2);
    pf.search(*t2.get(0,0), *t2.get(10,0));
    cout << t2 << endl;
    return 0;
}
