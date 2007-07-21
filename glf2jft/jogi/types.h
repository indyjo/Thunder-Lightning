#ifndef _JOGI_TYPES_H
#define _JOGI_TYPES_H

typedef             int     jBool;
typedef             int     jError;
typedef             int     j32;
typedef             short   j16;
typedef signed      char    j8;
typedef unsigned    int     ju32;
typedef unsigned    short   ju16;
typedef unsigned    char    ju8;

/*
 This is the type entry that specifies the pixel layout of a sprite *file*
 */
typedef enum {
    JSPRITE_TYPE_ARGB      = 0x0100,
    JSPRITE_TYPE_ABGR      = 0x0200,
    JSPRITE_TYPE_BGRA      = 0x0001,
    JSPRITE_TYPE_RGBA      = 0x0002
} jsprite_type_t;

/*
 This header is followed by the sprite's Data
 in BGRA format, so that we can read it as 32 bit ints x86 machines
 */
typedef struct
{
    j8  magic[4]; /* ="JSPR", also serves as big/low-endian detection */
    j16 w,h;      /* Width and height of the Sprite */
    j16 x,y;      /* The sprite's hotspot */
    j16 type;
} jspriteheader_t;

/*
 This ist the actual sprite structure which is used by JSprite in memory.
 The pixel layout is always native ARGB, meaning that the alpha mask is
 0xFF000000
 */
typedef struct
{
    j16   w,h;
    j16   x,y;
    ju32 *buf;
} jsprite_t;

#endif

