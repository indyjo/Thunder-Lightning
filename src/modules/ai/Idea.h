#ifndef AI_IDEA_H
#define AI_IDEA_H

#include <list>
#include <string>
#include <vector>
#include <tnl.h>

namespace AI {

class Idea : virtual public Object {
public:
    Idea(const std::string & name);

    void setEnabled(bool e);
    inline bool isEnabled() { return enabled; }

    void think();

    inline const std::string & getName() { return name; }
    inline const std::string & getInfo() { return info; }

protected:
    virtual void onEnabled();
    virtual void onDisabled();
    virtual void onThink();

protected:
    std::string name, info;
    bool enabled;
};


template<class Env, class Idea>
class IdeaBase {
    typedef std::vector<Idea*> Children;
    typedef typename Children::iterator CIter;

protected:
    Idea *parent;
private:
    Idea *old_parent;
    Children children, old_children;

protected:
    IdeaBase() : parent(0), old_parent(0)
    { }

    void activate(Idea & b) {
        children.push_back(&b);
        b.parent = reinterpret_cast<Idea*> (this);
    }

    void disable(Env & env) {
        onDisabled(env);
        for(CIter i=children.begin(); i!=children.end(); i++)
            (*i)->disable(env);
        children.clear();
    }

private:
    inline void reset() {
        parent = 0;
    }

    inline void compare(Env & env) {
        if (parent == old_parent) return;
        if (old_parent) disable(env);
        if (parent) onEnabled(env);
        old_parent = parent;
    }

public:
    virtual void think(Env & env) { }
    virtual void act(Env & env) { }
    virtual void onEnabled(Env & env) { }
    virtual void onDisabled(Env & env) { }
    void run(Env & env) {
        std::swap(children, old_children);
        children.clear();

        for(CIter i=old_children.begin(); i!=old_children.end(); i++)
            (*i)->reset();

        think(env);

        for(CIter i=old_children.begin(); i!=old_children.end(); i++)
            (*i)->compare(env);
        for(CIter i=children.begin(); i!=children.end(); i++)
            (*i)->compare(env);
        for(CIter i=children.begin(); i!=children.end(); i++)
            (*i)->run(env);

        act(env);
    }
};

} // namespace AI


#endif
