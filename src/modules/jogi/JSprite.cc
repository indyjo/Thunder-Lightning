#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <png.h>
#include "jogi.h"
#include <tnl.h>

using namespace std;

JSprite::JSprite()
{
    sprite.w=0;
    sprite.h=0;
    sprite.buf=NULL;
}


JSprite::~JSprite()
{
    unload();
}


jError JSprite::save(ostream &out)
{
    jspriteheader_t header;

    header.magic[0]='J';
    header.magic[1]='S';
    header.magic[2]='P';
    header.magic[3]='R';
    header.w=sprite.w;
    header.h=sprite.h;
    header.x=sprite.x;
    header.y=sprite.y;
    header.type=JSPRITE_TYPE_ARGB;

    out.write((char*) (&header.magic),sizeof(header));
    out.write((char*) sprite.buf, sprite.w*sprite.h*4);

    return JERR_OK;
}


jError JSprite::load(istream &in)
{
    
    jspriteheader_t header;
    char sample[4], spr_magic[4]={'J','S','P','R'};

    unload();

    in.read((char*) sample, 4);
    
    if (memcmp(sample, spr_magic, 4)==0) {
        return loadSPR(in);
    } else if (0 == png_sig_cmp((png_bytep)sample, 0, 4)) {
        return loadPNG(in);
    } else return JERR_FILEFORMAT;
}

jError JSprite::loadSPR(istream & in)
{
    jspriteheader_t header;
    
    // we read the rest of the header, skipping the 4 bytes of signature
    in.read((char *)&header.w, sizeof(header)-4);
    
    if (header.type!=JSPRITE_TYPE_ARGB) {
        return JERR_FILEFORMAT;
    }

    sprite.w=header.w;
    sprite.h=header.h;
    sprite.x=header.x;
    sprite.y=header.y;

    sprite.buf=(ju32 *) malloc(sprite.w*sprite.h*4);
    if (sprite.buf==NULL) {
        return JERR_NOT_ENOUGH_MEMORY;
    }

    in.read((char*) sprite.buf, sprite.w*sprite.h*4);
    return JERR_OK;
}

namespace {
    void read_from_stream(png_structp png_ptr, png_bytep data, unsigned int len)
    {
        istream & in = *((istream *) png_get_io_ptr(png_ptr));
        in.read((char*)data, len);
    }
}

// Reads a PNG file from a stream. loadPNG assumes that four bytes have already
// been read.
jError JSprite::loadPNG(istream & in)
{
    png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0,0,0);
    if (!png_ptr) return JERR_NOT_ENOUGH_MEMORY;
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, 0, 0);
        return JERR_NOT_ENOUGH_MEMORY;
    }
    
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        return JERR_NOT_ENOUGH_MEMORY;
    }

    // We read already 4 bytes of signature
    png_set_sig_bytes(png_ptr, 4);
    // We have to register our own reading funtction in order to read from a
    // C++ istream
    png_set_read_fn(png_ptr, &in, &read_from_stream);
    
    // Now we can begin reading the info header
    png_read_info(png_ptr, info_ptr);
    
    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    
    if (width != height || ((width-1) & width) != 0) {
        printf("WARNING: bad image size: %d x %d.\n", width, height);
    }
    
    if (bit_depth < 8) {
        png_set_expand(png_ptr);
    } else if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }
    
    if (color_type == PNG_COLOR_TYPE_RGB) {
        png_set_bgr(png_ptr);
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
    } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
        png_set_bgr(png_ptr);
    } else if (color_type == PNG_COLOR_TYPE_GRAY) {
        png_set_gray_to_rgb(png_ptr);
        png_set_bgr(png_ptr);
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
    } else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
        png_set_bgr(png_ptr);
    } else {
        printf("ERROR: Color format not supported for PNG sprites.");
        return JERR_FILEFORMAT;
    }
    
    png_read_update_info(png_ptr, info_ptr);
    
    sprite.w=width;
    sprite.h=height;
    sprite.x=0;
    sprite.y=0;

    sprite.buf=(ju32 *) malloc(sprite.w*sprite.h*4);
    if (!sprite.buf) {
        return JERR_NOT_ENOUGH_MEMORY;
    }
    
    png_bytep* row_pointers = new png_bytep[height];
    for(int i=0; i<height; i++)
        row_pointers[i] = (png_bytep) &sprite.buf[i*width];
    png_read_image(png_ptr, row_pointers);
    delete [] row_pointers;
    
    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    
    return JERR_OK;
}

void JSprite::unload()
{
    if (sprite.buf==NULL) {
        return;
    }

    free(sprite.buf);
    sprite.w=0;
    sprite.h=0;
    sprite.buf=NULL;
}

