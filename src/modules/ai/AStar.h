#ifndef AI_ASTAR_H
#define AI_ASTAR_H

#include <iostream>
#include <vector>
#include <queue>


namespace AI {

struct Node {
    int x,y;
    bool in_open;
    bool in_closed;
    float f,g,h;
    Node * parent;
    
    inline Node(int x, int y)
    :   x(x), y(y), in_open(false), in_closed(false), f(0), g(0), h(0),
        parent(0)
    { }
    
    struct Compare : public std::binary_function<Node *, Node *, bool> 
    {
        inline bool operator() (const Node *a, const Node *b) const {
            return a->f > b->f;
        }
    };
};

struct Neighbors {
    Node * n[8];
};

class Terrain {
public:
    virtual Node *root() = 0;
    virtual void neighbors(const Node & node, Neighbors * neighbors) = 0;
    
    virtual float estimate(const Node & n1, const Node & n2) = 0;
    virtual float cost(const Node & n1, const Node & n2) = 0;
};

class AdaptiveTerrain : public Terrain{
public:
    AdaptiveTerrain(int init_w=32, int init_h=32,
                    int origin_x=16, int origin_y=16);
    ~AdaptiveTerrain();

    virtual Node *root();
    virtual void neighbors(const Node & node, Neighbors * neighbors);
    
    virtual float estimate(const Node & n1, const Node & n2);
    virtual float cost(const Node & n1, const Node & n2);
    
    Node * get(int x, int y);
    
    friend std::ostream& operator<<(std::ostream &, const AdaptiveTerrain &);
protected:
    void growWest();
    void growEast();
    void growNorth();
    void growSouth();
    std::vector<Node*> nodes;
    int origin_x, origin_y, w,h;
};


class PathFinder {
public:
    inline PathFinder(Terrain *t) : terrain(t) { };
    bool search(Node & start, Node & goal);
protected:
    Terrain * terrain;
    std::priority_queue<Node*, std::vector<Node*>, Node::Compare> open;
};

} // namespace AI

#endif
