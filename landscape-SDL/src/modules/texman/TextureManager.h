#ifndef RESMAN_H
#define RESMAN_H

#include <string>
#include <map>
#include <set>
#include <queue>
#include <modules/jogi/jogi.h>

class TextureManager;
class Texture;
class TexPtr;

class TextureManager : virtual public Object
{
    friend class Texture;
    typedef std::map<std::string, Texture *> NameToTexMap;
    typedef NameToTexMap::iterator TexIterator;
    typedef std::map<Texture *, std::string> TexToNameMap;
    typedef TexToNameMap::iterator NameIterator;
    

    int max_cache_size;
    std::set<TexPtr> cache;
    std::queue<TexPtr> cache_queue;
    NameToTexMap textures_by_name;
    TexToNameMap names_by_texture;
    JRenderer & renderer;
    
public:
    TextureManager(JRenderer & renderer);
    ~TextureManager();
    TexPtr query(const char *,
            unsigned int hint=0,
            unsigned int compression=0,
            bool mipmap=false);
private:
    void registerTexture(const char *, Texture *);
    bool unregisterTexture(Texture *);
};

class Texture
{
    int refs;
    jrtxtid_t tex;
    TextureManager & texman;
    JRenderer &renderer;
    int w,h;
    
protected:
    ~Texture();
    
public:
    Texture(TextureManager &, JRenderer &,
        const char *, unsigned int, unsigned int, bool);

    jrtxtid_t ref();
    void unref();
    inline int getWidth() { return w; }
    inline int getHeight() { return h; }
};

class TexPtr
{
    jrtxtid_t txtid;
    Texture *tex;
    bool valid;
    
public:
    inline TexPtr() : valid(false) { };

    inline TexPtr(const TexPtr &other) : valid(other.valid), tex(other.tex)
    { if (valid) txtid=tex->ref(); };
    
    inline TexPtr(Texture * tex) : valid(true), tex(tex)
    { txtid = tex->ref(); };
        
    inline ~TexPtr() { if (valid) tex->unref(); };
    
    inline int getWidth() { return tex->getWidth(); }
    inline int getHeight() { return tex->getHeight(); }
    
    inline TexPtr & operator= (const TexPtr & other)
    {
        if (other.valid) txtid = other.tex->ref();
        if (valid) tex->unref();
        valid = other.valid;
        tex = other.tex;
        
        return *this;
    }
    
    inline operator bool() { return valid; }
    
    inline bool operator< (const TexPtr & other) const
    { return tex < other.tex; }
    
    inline operator jrtxtid_t() { return txtid; };
};

#endif
