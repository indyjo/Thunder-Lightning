#ifndef _LANDSCAPE_H
#define _LANDSCAPE_H

#include <cstdio>

#ifdef  __GNUC__
#define ls_message(format, args...) do {\
  printf("\033[0;32m");                 \
  printf(format, ##args);               \
  printf("\033[0;39m");                 \
  fflush(stdout);                       \
  } while(0)
#define ls_warning(format, args...) do {\
  printf("\033[1;33m");                 \
  printf(format, ##args);               \
  printf("\033[0;39m");                 \
  fflush(stdout);                       \
  } while(0)
#define ls_error(format, args...) do {  \
  printf("\033[1;31m");                 \
  printf(format, ##args);               \
  printf("\033[0;39m");                 \
  fflush(stdout);                       \
  } while(0)
#else   /* !__GNUC__ */
#define ls_message printf
#define ls_warning printf
#define ls_error printf
#endif

#define ENABLE_TERRAIN 0
#define ENABLE_LOD_TERRAIN 1
#define ENABLE_SKY 0
#define ENABLE_SKYBOX 1
#define ENABLE_HORIZON 0
#define ENABLE_MAP 1
#define ENABLE_GUNSIGHT 1
#define ENABLE_DEBUG_TRIANGLE 1

#define TIME_ACCUM 5.0
#define MOUSE_ACCUM 5
#define INITIAL_ALTITUDE 600.0f
#define TERRAIN_HEIGHT 1500.0f
#define TERRAIN_LENGTH 72000.0f
#define HFPIXELS_TO_DRAW 16
#define SKY_HEIGHT 1500.0f
#define SKY_SEGMENT_SIZE 2000.0f

#define TILE_SIZE 4
#define TILE_NUM ((512/TILE_SIZE)*(512/TILE_SIZE))

#define HORIZON_SEGMENTS 16
#define HORIZON_RADIUS CLIP_RANGE
#define HORIZON_TOP_HEIGHT (5400.0f*CLIP_RANGE/5000.0f)
#define HORIZON_MID_HEIGHT (2460.0f*CLIP_RANGE/5000.0f)
#define HORIZON_ZERO_HEIGHT (1000.0f*CLIP_RANGE/5000.0f)
#define HORIZON_BOT_HEIGHT (HORIZON_RADIUS * (-1000.0f))

#define FOG_MIDCOLOR_RED   176.0f
#define FOG_MIDCOLOR_GREEN 216.0f
#define FOG_MIDCOLOR_BLUE  240.0f

#define SKY_COLOR_RED    96.0f
#define SKY_COLOR_GREEN 143.0f
#define SKY_COLOR_BLUE  207.0f


#define SKY_BLUE_HEIGHT 5000.0f
#define SKY_BLACK_HEIGHT 15000.0f

#define PLANE_MINUS_Z 0
#define PLANE_PLUS_Z 1
#define PLANE_MINUS_X 2
#define PLANE_PLUS_X 3
#define PLANE_MINUS_Y 4
#define PLANE_PLUS_Y 5


#include <sigc++/sigc++.h>
#include "object.h"
#include "Factory.h"
#include <modules/math/SpecialMatrices.h>
#include <modules/jogi/jogi.h>
#include "util.h"
#include "resman.h"
#include "signals.h"
#include "remap.h"
#include "sound.h"
#include "interfaces.h"

#endif
