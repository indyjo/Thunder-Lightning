#ifndef AI_PATHMANAGER_H
#define AI_PATHMANAGER_H

#include <tnl.h>
#include <deque>
#include "AStar.h"

namespace AI {

class PathEvaluator;

class PathManager : public Object, protected AdaptiveTerrain {
public:
    PathManager(float xres, float zres);

    bool findPath(const Vector & start,
                  const Vector & goal,
                  PathEvaluator * evaluator);
    
    void getPath(std::deque<Vector> * path);
    
protected:
    virtual float cost(const Node & n1, const Node & n2);
    virtual float estimate(const Node & n1, const Node & n2);
    PathEvaluator * pe;
    Vector ref;
    int start_x, start_y, goal_x, goal_y;
    float xres, zres;
};

class PathEvaluator {
public:
    virtual float cost(const Vector & a, const Vector & b) = 0;
};

} // namespace AI

#endif
