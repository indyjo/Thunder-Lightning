#ifndef TNL_INTERPOLATOR_H
#define TNL_INTERPOLATOR_H

#include <stdexcept>
#include <vector>

/// Implements a piecewise linear interpolator function, where between neighboring
/// nodes (x(n), y(n)) and (x(n+1),y(n+1)), y values are interpolated. For x < x(0)
/// and x > x(max), corresponding y values are extrapolated.

template<class X, class Y>
class Interpolator {
    typedef std::pair<X,Y> Node;
    typedef std::vector<Node> Nodes;
    
    Nodes nodes;
    
    static Y interp(const X& x, const Node & a, const Node & b) {
        X t = (x - a.first) / (b.first - a.first);
        return a.second + t * (b.second - a.second);
    }
    
public:
    Interpolator() { }
    Interpolator(const Y & y) {
        node(0, y);
    }
    
    Interpolator& node(const X& x, const Y& y) {
        if (!nodes.empty() && nodes.back().first >= x) {
            throw std::runtime_error("Interpolator: x values must increase monotonically");
        }
        nodes.push_back(Node(x,y));
        return *this;
    }
    
    Y operator() (const X& x) const {
        if (nodes.empty()) {
            throw std::runtime_error("Interpolator: Need at least one (x,y) tuple");
        }
        if (nodes.size() == 1) {
            return nodes[0].second;
        }
        
        typename Nodes::const_iterator i;
        
        i = nodes.begin();
        while ((i+2) != nodes.end() && x > i[1].first) {
            ++i;
        }
        
        return interp(x, i[0], i[1]);
    }
};

#endif

