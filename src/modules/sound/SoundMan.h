#ifndef SOUND_SOUNDMAN_H
#define SOUND_SOUNDMAN_H

#include <modules/math/Vector.h>
#include <TypedObject.h>
#include <object.h>

class Sound;
class SoundSource;

class SoundMan : public Object, public TypedObject
{
public:
    const static Type type;
    
    virtual ~SoundMan() = 0;

    virtual Ptr<Sound> querySound(const std::string & name) = 0;
    virtual Ptr<SoundSource> requestSource() = 0;
    
    virtual void update(float delta_t) = 0;
    
    virtual void manage(Ptr<SoundSource>) = 0;
    virtual void removeFromManaged(Ptr<SoundSource>) = 0;
    
    virtual void setListenerPosition( const Vector & pos ) = 0;
    virtual void setListenerVelocity( const Vector & vel ) = 0;
    virtual void setListenerOrientation( const Vector & up, const Vector & front ) = 0;
    
    virtual Vector getListenerPosition() = 0;
    virtual Vector getListenerVelocity() = 0;
    virtual void   getListenerOrientation( Vector & up, Vector & front) = 0;
};

#endif

