#include <stdexcept>
#include <string>
#include <cstdio>
#include "image.h"

/* Image type values */
#define NO_IMAGE 0 
#define COLOR_MAPPED 1
#define RGB 2
#define BLACK_AND_WHITE 3
#define RLE_COLOR_MAPPED 9
#define RLE_RGB 10
#define COMPRESSED_BLACK_AND_WHITE 11
#define COMPRESSED_COLOR_MAPPED 32 
#define COMPRESSED_COLOR_MAPPED_4PASS 33

/* Descriptor values */
#define BPP_MASK            0x0000000f
#define ORIGIN_MASK         0x00000020
#define LOWER_LEFT          0
#define UPPER_LEFT          ORIGIN_MASK
#define INTERLEAVE_MASK     0x000000c0
#define INTERLEAVE_NONE     0
#define INTERLEAVE_TWO_WAY  0x00000040
#define INTERLEAVE_FOUR_WAY 0x00000080

typedef struct {
    unsigned char idlength;
    unsigned char cmap_type;
    unsigned char image_type;
    unsigned short cmap_origin;
    unsigned short cmap_length;
    unsigned char  cmap_entry_size;
    unsigned short image_x_origin;
    unsigned short image_y_origin;
    unsigned short image_width;
    unsigned short image_height;
    unsigned char  image_bpp;
    unsigned char  descriptor;
} TGAFileHeader;


/*
typedef struct {
    unsigned char idlength;
    unsigned char cmap_type;
    enum {
        NO_IMAGE=0,
        COLOR_MAPPED,
        RGB,
        BLACK_AND_WHITE,
        RLE_COLOR_MAPPED=9,
        RLE_RGB,
        COMPRESSED_BLACK_AND_WHITE,
        COMPRESSED_COLOR_MAPPED=32,
        COMPRESSED_COLOR_MAPPED_4PASS
    } image_type;
    struct {
        unsigned short origin;
        unsigned short length;
        unsigned char  entry_size;
    } cmap;
    struct Image {
        unsigned short x_origin;
        unsigned short y_origin;
        unsigned short width;
        unsigned short height;
        unsigned char  bpp;
        struct Descriptor {
            unsigned int attr_bpp:4;
            unsigned int reserved:1;
            enum {LOWER_LEFT=0, UPPER_LEFT=1} origin:1;
            enum {NONE=0, TWO_WAY, FOUR_WAY, RESERVED}  interleave:2;
        } descriptor;
    } image;
} TGAFileHeader;
*/

        
Image::Image(int w, int h, bool greyscale)        
    : w(w), h(h), greyscale(greyscale)
{
    if (!greyscale) {
        palette = new unsigned int[MAX_COLORS];
        for(int i=0; i<MAX_COLORS; i++) palette[i] = 0xffff00ff; // Ugly Magenta
    } else {
        palette = 0;
    }
    if (w && h) {
        pixels = new unsigned char[w*h];
        for(int i=0; i<w*h; i++) { pixels[i] = 0; }
    } else {
        pixels = 0;
    }
}
     
Image::~Image() {
    if (palette) {
        delete[] palette;
        palette = 0;
    }
    if (pixels) {
        delete[] pixels;
        pixels = 0;
    }
}

int read_byte(FILE *in) {
    return getc(in);
}

int read_word(FILE *in) {
    int a = read_byte(in);
    int b = read_byte(in);
    return (b << 8) | a;
}

int read_rgb3(FILE *in) {
    int r = read_byte(in);
    int g = read_byte(in);
    int b = read_byte(in);
    return (r << 16) | (g<<8) | b;
}

int read_argb(FILE *in) {
    int a = read_byte(in);
    int r = read_byte(in);
    int g = read_byte(in);
    int b = read_byte(in);
    return (a << 24) | (r << 16) | (g<<8) | b;
}

#define BYTE read_byte(in)
#define WORD read_word(in)
#define RGB3 read_rgb3(in)
#define ARGB read_argb(in)

Image::Image(const char *filename)
{
    int i;
    FILE *in;
    TGAFileHeader header;

    in=fopen(filename,"rb");
    if (in == NULL)
        throw std::runtime_error(std::string("Image::Image: Error loading ")+filename);
    /* Read the various header fields */
    // fread(&header, 1, 18, in);
    header.idlength = BYTE;
    header.cmap_type = BYTE;
    header.image_type = BYTE;
    header.cmap_origin = WORD;
    header.cmap_length = WORD;
    header.cmap_entry_size = BYTE;
    header.image_x_origin = WORD;
    header.image_y_origin = WORD;
    header.image_width = WORD;
    header.image_height = WORD;
    header.image_bpp = BYTE;
    header.descriptor = BYTE;
    
    printf("idlength=%d ct=%d it=%d cmo=%d cml=%d cme=%d xo=%d yo=%d w=%d h=%d bpp=%d des=%d\n",
        (int)header.idlength, (int)header.cmap_type, (int)header.image_type, (int)header.cmap_origin,
        (int)header.cmap_length,
        (int)header.cmap_entry_size, (int)header.image_x_origin, (int)header.image_y_origin,
        (int)header.image_width, (int)header.image_height, (int)header.image_bpp, (int)header.descriptor);


    if (header.image_type == BLACK_AND_WHITE) {
        greyscale = true;
    } else if (header.image_type == COLOR_MAPPED) {
        greyscale = false;
    } else {
        throw std::runtime_error("Image::Image: Unsupported image type.");
    }

    /* Allocate space for image */
    w = header.image_width;
    h = header.image_height;
    if (!greyscale) palette = new unsigned int[MAX_COLORS];
    pixels = new unsigned char[w*h];

    /* Skip ID field */
    fseek(in, header.idlength, SEEK_CUR);
    /* Read Color pallette */
    if (!greyscale) {
        if (header.cmap_entry_size == 32) {
            for(i=0; i<header.cmap_length; i++) {
                /* Read color in BGRA order and store as ARGB int */
                palette[i] = ARGB;
            }
        } else if (header.cmap_entry_size == 24) {
            for(i=0; i<header.cmap_length; i++) {
                /* Read color in RGB order and store as ARGB int with A=255 */
                palette[i] = 0xff000000 | RGB3;
            }
        } else {
            printf("Unsupported color map format: %d\n", header.cmap_entry_size);
        }
    }

    /* Finally read the pixel data, flip if necessary ( we want the origin
       to be in the bottom-left corner) */
    if( (header.descriptor & ORIGIN_MASK) == UPPER_LEFT ) {
        for(i=0; i<h; i++) {
            fread(&pixels[(h-i-1)*w], 1, w, in);
        }
    } else {
        fread(pixels, 1, w * h, in);
    }

    /* Close file */
    fclose(in);
    //dump();
}

#undef RGB3
#undef ARGB
#undef BYTE
#undef WORD


void write_byte(FILE *out, int x) {
    putc(x & 0xff, out);
}

void write_word(FILE *out, int x) {
    write_byte(out, x);
    write_byte(out, x >> 8);
}

void write_rgb3(FILE *out, int x) {
    write_byte(out, x >> 16);
    write_byte(out, x >> 8);
    write_byte(out, x);
}

void write_argb(FILE *out, int x) {
    write_byte(out, x >> 24);
    write_byte(out, x >> 16);
    write_byte(out, x >> 8);
    write_byte(out, x);
}

#define BYTE(x) write_byte(out,x);
#define WORD(x) write_word(out,x);
#define RGB3(x) write_rgb3(out,x);
#define ARGB(x) write_argb(out,x);

void Image::saveTo(const char *filename) {
    FILE *out;
    TGAFileHeader header;
    
    header.idlength = 0;
    header.cmap_type = greyscale?0:1;
    header.image_type = greyscale?BLACK_AND_WHITE:COLOR_MAPPED;
    header.cmap_origin = 0;
    header.cmap_length = greyscale?0:MAX_COLORS;
    header.cmap_entry_size = greyscale?0:24;
    header.image_x_origin = 0;
    header.image_y_origin = 0;
    header.image_width = w;
    header.image_height = h;
    header.image_bpp = 8;
    header.descriptor = UPPER_LEFT | INTERLEAVE_NONE;
    
    out=fopen(filename,"w+b");
    /* Write the various header fields */
    //fwrite(&header, 18, 1, out);
    BYTE(header.idlength);
    BYTE(header.cmap_type);
    BYTE(header.image_type);
    WORD(header.cmap_origin);
    WORD(header.cmap_length);
    BYTE(header.cmap_entry_size);
    WORD(header.image_x_origin);
    WORD(header.image_y_origin);
    WORD(header.image_width);
    WORD(header.image_height);
    BYTE(header.image_bpp);
    BYTE(header.descriptor);
    
    if (!greyscale) {
        for(int i=0; i<MAX_COLORS; i++) {
            RGB3(palette[i]);
        }
    }
    //fwrite(&pixels, w*h, 1, out);
    for(int y=0; y<h; y++) for (int x=0; x<w; x++) BYTE(pixelAt(x,y));
    
    fclose(out);
}

#undef RGB3
#undef ARGB
#undef BYTE
#undef WORD
#undef TRIPLE
#undef LONG



void Image::dump() {
    printf("Image @ %p: w=%d h=%d\n", this, w,h);
    for (int y=0; y<20; y++) {
        for (int x=0; x<20; x++) printf("%2x ", pixelAt(x,y));
        printf("\n");
    }
}

