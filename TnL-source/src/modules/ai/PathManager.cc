#include "PathManager.h"

namespace AI {

PathManager::PathManager(float xres, float zres)
:   xres(xres), zres(zres)
{  }

bool PathManager::findPath(const Vector & start,
                           const Vector & goal,
                           PathEvaluator * evaluator)
{
    this->pe = evaluator;
    this->ref = 0.5f * (start+goal);
    float dx=start[0]-ref[0], dz=start[2]-ref[2];
    start_x = (int)(dx / xres);
    start_y = (int)(dz / zres);
    goal_x = (int)(-dx / xres);
    goal_y = (int)(-dz / zres);
    PathFinder pf(this);
    return pf.search(*get(start_x, start_y), *get(goal_x, goal_y));
}

void PathManager::getPath(std::deque<Vector> * path)
{
    Node * n = get(goal_x, goal_y);
    do {
        path->push_front(Vector(ref[0]+xres*n->x, 0.0f, ref[2]+zres*n->y));
    } while(n = n->parent);
}

float PathManager::cost(const Node & n1, const Node & n2) {
    return pe->cost(
            Vector(ref[0]+xres*n1.x, 0.0f, ref[2]+zres*n1.y),
            Vector(ref[0]+xres*n2.x, 0.0f, ref[2]+zres*n2.y));
}

float PathManager::estimate(const Node & n1, const Node & n2) {
    int dx = n2.x - n1.x;
    int dy = n2.y - n1.y;
    return Vector(ref[0]+xres*dx, 0.0f, ref[2]+zres*dy).length();
}

} // namespace AI
