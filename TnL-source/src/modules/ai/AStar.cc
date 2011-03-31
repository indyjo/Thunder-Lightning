#include <cmath>
#include <iostream>
#include "AStar.h"

using namespace std;

namespace AI {

AdaptiveTerrain::AdaptiveTerrain(int init_w, int init_h,
                                 int origin_x, int origin_y)
:   w(init_w), h(init_h), origin_x(origin_x), origin_y(origin_y),
    nodes(init_w*init_h)
{ }

AdaptiveTerrain::~AdaptiveTerrain() {
    for(int y=0; y<h; y++)
        for(int x=0; x<w; x++)
            if (nodes[y*w+x]) delete nodes[y*w+x];
}

Node * AdaptiveTerrain::root() {
    return get(0,0);
}

void AdaptiveTerrain::neighbors(const Node & node,
                                        Neighbors * neighbors)
{
    int x=node.x;
    int y=node.y;
    neighbors->n[0] = get(x, y+1);
    neighbors->n[1] = get(x+1, y+1);
    neighbors->n[2] = get(x+1, y);
    neighbors->n[3] = get(x+1, y-1);
    neighbors->n[4] = get(x, y-1);
    neighbors->n[5] = get(x-1, y-1);
    neighbors->n[6] = get(x-1, y);
    neighbors->n[7] = get(x-1, y+1);
}
    

float AdaptiveTerrain::estimate(const Node & n1, const Node & n2) {
    float dx = n2.x - n1.x;
    float dy = n2.y - n1.y;
    return sqrt(dx*dx+dy*dy);
}

float AdaptiveTerrain::cost(const Node & n1, const Node & n2) {
    return estimate(n1, n2);
}

Node * AdaptiveTerrain::get(int x, int y) {
    while (x + origin_x >= w) growEast();
    while (x + origin_x < 0) growWest();
    while (y + origin_y >= h) growNorth();
    while (y + origin_y < 0) growSouth();
    
    Node *n = nodes[(y+origin_y)*w + x+origin_x];
    if (!n) {
        n = new Node(x,y);
        nodes[(y+origin_y)*w + x+origin_x] = n;
    }
    return n;
}

void AdaptiveTerrain::growWest() {
    int w_new = 2*w;
    nodes.resize(w_new*h, 0);
    for(int y=h-1; y>=0; y--) {
        for(int x=0; x<w; x++) {
            swap(nodes[y*w + x],
                 nodes[y*w_new + x + w]);
        }
    }
    origin_x+=w;
    w = w_new;
}

void AdaptiveTerrain::growEast() {
    int w_new = 2*w;
    nodes.resize(w_new*h, 0);
    for(int y=h-1; y>=0; y--) {
        for(int x=0; x<w; x++) {
            swap(nodes[y*w + x],
                 nodes[y*w_new + x]);
        }
    }
    w = w_new;
}

void AdaptiveTerrain::growNorth() {
    int h_new = 2*h;
    nodes.resize(w*h_new, 0);
    h = h_new;
}

void AdaptiveTerrain::growSouth() {
    int h_new = 2*h;
    nodes.resize(w*h_new, 0);
    for (int s=0; s<w*h; s++)
        swap(nodes[s], nodes[s+w*h]);
    origin_y += h;
    h = h_new;
}


ostream& operator<<(ostream & out, const AdaptiveTerrain & t)
{
    out << "AdaptiveTerrain: (" << endl;
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
            if (x == t.origin_x && y == t.origin_y) {
                c = n?'@':'O';
            } else {
                c = n?'x':'·';
            }
            out << c;
        }
        out << endl;
    }
    out << ")";
    
    return out;
}


// An implementation of the A* algorithm.
// See http://www.gamasutra.com/features/19970801/pathfinding.htm
bool PathFinder::search(Node & start, Node & goal) {
    Neighbors neighbors;
    
    //open.clear();
    start.g = 0.0f;
    start.h = terrain->estimate(start, goal);
    start.f = start.g + start.h;
    start.parent = 0;
    start.in_open = true;
    open.push(&start);
    
    int count=0;
    while (!open.empty()) {
        Node *n = open.top();
        open.pop();
        n->in_open = false;
        
        if (n->x == goal.x && n->y == goal.y) return true;
        
        if (count++ > 1000) return false;
        terrain->neighbors(*n, &neighbors);
        for(int i=0; i<8; i++) {
            Node *n2 = neighbors.n[i];
            float cost = terrain->cost(*n, *n2);
            if (cost < 0.0f)
                continue;
            float newg = n->g + cost;
            if ((n2->in_open || n2->in_closed) && n2->g <= newg)
                continue;
            n2->parent = n;
            n2->g = newg;
            n2->h = terrain->estimate(*n2, goal);
            n2->f = n2->g + n2->h;
            n2->in_closed = false;
            if (!n2->in_open) {
                n2->in_open = true;
                open.push(n2);
            }
        }
        n->in_closed = true;
    }
    return false;
}


} // namespace AStar
