#ifndef TNL_TEXTUREMANAGER_H
#define TNL_TEXTUREMANAGER_H

#include <string>
#include <map>
#include <queue>
#include <modules/jogi/jogi.h>
#include <Weak.h>

struct IConfig;
class TextureManager;

class Texture : public Object, public Weak
{
    jrtxtid_t tex;
    JRenderer &renderer;
    
protected:
    ~Texture();
    
public:
    Texture(jrtxtid_t t, JRenderer &r);

    inline int getWidth() { return renderer.getTextureWidth(tex); }
    inline int getHeight() { return renderer.getTextureHeight(tex); }
    
    inline jrtxtid_t getTxtid() { return tex; }
};

typedef Ptr<Texture> TexPtr;

class TextureManager : virtual public Object
{
    typedef std::map<std::string, WeakPtr<Texture> > NameToTexMap;

    int max_cache_size;
    std::queue<Ptr<Texture> > cache_queue;
    NameToTexMap textures_by_name;
    
    JRenderer & renderer;
    
    Ptr<Texture> missing_texture;
    
public:
    TextureManager(IConfig & cfg, JRenderer & renderer);
    ~TextureManager();
    
    /// Requests a texture with the given file name. Additionally a JRenderer
    /// hint and (derecated) compression number can be passed. It is also possible
    /// to request that mipmap layers be generated.
    Ptr<Texture> query( const char *,
                        unsigned int hint=0,
                        unsigned int compression=0,
                        bool mipmap=false);
    /// Tells the TextureManager to insert a texture into its cache, keeping it
    /// alive until
    ///  - it is kicked out of the cache by other textures
    ///  - shutdown() is called
    void cache( Ptr<Texture> tex );
    void shutdown();
};

#endif
