#include <fstream>

#include <tnl.h>
#include <modules/texman/TextureManager.h>

using namespace std;


TexPtr TextureManager::query(
            const char * name,
            unsigned int hint,
            unsigned int compression,
            bool mipmap)
{
    //ls_message("querying texture %s\n", name);
    TexIterator i = textures_by_name.find(name);
    if (i==textures_by_name.end()) {
    	//ls_message("  --> not found, creating:\n");
        Texture * nt =
                new Texture(this, renderer, name, hint, compression, mipmap);
        //ls_message("      done.\n");
        TexPtr p(nt);
        return p;
    } else {
    	//ls_message("  --> found!\n");
        TexPtr p(i->second);
        return p;
    }
};

namespace {
string cut(int max_length, const string src) {
    if (src.length() > max_length) {
        return "..." + src.substr(src.length() - max_length + 3, max_length-3);
    } else {
        return src;
    }
}
}

TextureManager::TextureManager(JRenderer & renderer)
: renderer(renderer), max_cache_size(100)
{ };

TextureManager::~TextureManager() {
    // Clear the cache
    // This is necessary because otherwise the TexPtr elements of the cache
    // would try to unregister themselves with textures that have already been
    // deleted
    cache.clear();
    while (!cache_queue.empty()) cache_queue.pop();
    ls_message("TextureManager finished.\n");
}

void TextureManager::registerTexture(const char * name, Texture *tex)
{
    //ls_message("registerTexture: %s | %p (total: %d)\n",
    //        cut(40, name).c_str(), tex, textures_by_name.size() + 1);
    textures_by_name[name] = tex;
    names_by_texture[tex] = name;
    cache.insert(TexPtr(tex));
    cache_queue.push(TexPtr(tex));
    if (cache.size() > max_cache_size) {
        // We have to take one element out of the cache
        cache.erase(cache.find(cache_queue.front()));
        cache_queue.pop();
    }
}

bool TextureManager::unregisterTexture(Texture *tex)
{
    NameIterator i = names_by_texture.find(tex);
    if (i != names_by_texture.end()) {
    	//ls_message("unregisterTexture: %s (total: %d)\n",
        //        cut(40, i->second).c_str(), textures_by_name.size() - 1);
        textures_by_name.erase(i->second);
        names_by_texture.erase(i);
        return true;
    } else {
        ls_error("TextureManager: Trying to unregister texture at %p"
                " which is not registered!\n", tex);
        return false;
    }
}

void TextureManager::shutdown()
{
	cache.clear();
	while(!cache_queue.empty())
		cache_queue.pop();
}

Texture::~Texture()
{
    if (texman->unregisterTexture(this)) renderer.destroyTexture(tex);
}

Texture::Texture(Ptr<TextureManager> texman,
                 JRenderer & renderer,
                 const char *filename,
                 unsigned int hint,
                 unsigned int compression,
                 bool mipmap)
    : texman(texman), renderer(renderer), refs(0)
{
    ifstream in(filename, ios::binary|ios::in);
    JSprite spr;
    
    if(!in) {
        ls_error("Texture: Couldn't open %s.\n", filename);
        tex = 1;
    } else {
        spr.load(in);
        in.close();
        renderer.createTexture(&spr.sprite,hint,compression,mipmap,&tex);
        w = spr.sprite.w;
        h = spr.sprite.h;
    }
    
    texman->registerTexture(filename, this);
}


jrtxtid_t Texture::ref() {
    refs++;
    return tex;
}


void Texture::unref() {
    refs--;
    if (refs==0) {
        delete this;
    }
}
