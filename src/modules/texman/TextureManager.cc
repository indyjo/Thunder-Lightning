#include <fstream>
#include <stdexcept>

#include <interfaces/IConfig.h>
#include <modules/texman/TextureManager.h>
#include <modules/jogi/jogi.h>

#include "TextureManager.h"
#include <tnl.h>

using namespace std;


Ptr<Texture> TextureManager::query(
            const char * name,
            unsigned int hint,
            unsigned int compression,
            bool mipmap)
{
    typedef NameToTexMap::iterator TexIterator;
    TexIterator i = textures_by_name.find(name);
    
    if (i != textures_by_name.end()) {
        Ptr<Texture> tex = i->second.lock();
        
        if (tex) {
            // The texture was found and can be returned.
            return tex;
        }
        
        
        // The texture was found but has been deleted. Remove from list and reload.
        textures_by_name.erase(i);
    }
        
    // The texture needs to be loaded from disk.
    
    ifstream in(name, ios::binary|ios::in);
    JSprite spr;
    if(in) {
        spr.load(in);
        jrtxtid_t tex;
        jError result = renderer.createTexture(&spr.sprite,hint,compression,mipmap,&tex);
        if (result != JERR_OK) {
            ls_error("TextureManager: Error %d creating texture %s.\n", result, name);
            return missing_texture;
        }
        Ptr<Texture> ptex = new Texture(tex, renderer);
        textures_by_name[name] = ptex;
        cache(ptex);
        return ptex;
    } else {
        ls_error("TextureManager: Couldn't open %s.\n", name);
        return missing_texture;
    }
};

TextureManager::TextureManager(IConfig & cfg, JRenderer & renderer)
: renderer(renderer), max_cache_size(cfg.queryInt("TexMan_cache_size", 100))
{
    const char *name = cfg.query("TexMan_missing_texture");
    if (!name) throw runtime_error("TexMan: Config entry 'TexMan_missing_texture' not defined.");
    
    ifstream in(name, ios::binary|ios::in);
    JSprite spr;
    if(in) {
        spr.load(in);
        jrtxtid_t tex;
        jError result = renderer.createTexture(&spr.sprite,0,0,false,&tex);
        if (result != JERR_OK) {
            ls_error("TextureManager: Error %d creating texture %s.\n", result, name);
            throw runtime_error("There was an error creating a texture.");
        }
        missing_texture = new Texture(tex, renderer);
    } else {
        ls_error("TextureManager: Couldn't open %s.\n", name);
        throw runtime_error("There was an error loading the 'missing' texture.");
    }

};

TextureManager::~TextureManager() {
    ls_message("TextureManager finished.\n");
}

void TextureManager::cache(Ptr<Texture> tex) {
    cache_queue.push(tex);
    if (cache_queue.size() > max_cache_size) {
        // We have to take one element out of the cache
        cache_queue.pop();
    }
}

void TextureManager::shutdown()
{
	while(!cache_queue.empty())
		cache_queue.pop();
}

