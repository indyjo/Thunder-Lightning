#ifndef UI_COMPONENT_H
#define UI_COMPONENT_H

#include <modules/math/Vector.h>
#include <object.h>

namespace UI {

class Panel;

class Component : virtual public Object {
protected:
    float width, height;
    Vector2 offset;
    std::string name;
public:
    Component(const char *name, float w=256, float h=256);
    
    inline void setOffset(const Vector2 & ofs) { offset=ofs; }
    inline const Vector2 & getOffset() { return offset; }
    
    virtual float getWidth();
    virtual float getHeight();
    
    inline void setWidth(float f) { width = f; }
    inline void setHeight(float f) { height = f; }
    
    inline const std::string & getName() const { return name; }
    
    /// Do the actual drawing. The default implementation draws nothing.
    virtual void draw(Panel &);
    
    // These are called when the panel containing this component is enabled
    // or disabled. Default is to do nothing.
    // This might be used to store state only relevant
    // while the component is shown.
    virtual void enable();
    virtual void disable();

    /// Called by Panel when layouting.
    /// Components may override this to dynamically calculate their dimensions.
    /// Default implementation does nothing.
    virtual void onLayout(Panel &);
};

} // namespace UI

#endif

