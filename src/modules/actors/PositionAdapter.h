#ifndef TNL_POSITION_ADAPTER_H
#define TNL_POSITION_ADAPTER_H

#include <interfaces/IMovementProvider.h>
#include <modules/math/Transform.h>

/// Simplifies IPositionProvider implementation by using a single overridable
/// function.
class PositionAdapterBase : virtual public IPositionProvider
{
protected:
    /// Child classes need to override this
    virtual Matrix3 getOrientation() = 0;
public:
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(  Vector * up,
                                  Vector * right,
                                  Vector * front);
};

class MovementAdapterBase : virtual public PositionAdapterBase,
                            virtual public IMovementProvider
{
};


class SimplePositionAdapter : public MovementAdapterBase
{
    Transform xform;
    Ptr<IPositionProvider> position;
    Ptr<IMovementProvider> movement;
public:
    SimplePositionAdapter();
    virtual ~SimplePositionAdapter();
    
    SimplePositionAdapter & withPositionFrom(Ptr<IPositionProvider>);
    SimplePositionAdapter & withMovementFrom(Ptr<IMovementProvider>);
    SimplePositionAdapter & withPositionAndMovementFrom(Ptr<IMovementProvider>);
    
    SimplePositionAdapter & rotatedBy(const Vector & axis, float angle);
    SimplePositionAdapter & translatedBy(const Vector &);
    
    /// @name IPositionProvider and IMovementProvider implementation
    /// @{
    virtual Vector getLocation();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(  Vector * up,
                                  Vector * right,
                                  Vector * front);
    virtual Vector getMovementVector();
    /// @}
};

#endif

