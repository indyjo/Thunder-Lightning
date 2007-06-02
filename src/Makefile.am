SUBDIRS = modules boost interfaces

bin_PROGRAMS = tnl-bin

tnl_bin_SOURCES = game.cc game.h landscape.h \
                        Factory.h \
                        object.cc object.h \
                        util.h signals.h remap.h remap.cc \
                        sound.cc sound.h \
                        mtasker.cc mtasker.h \
                        coroutine.cc coroutine.h \
                        TargetInfo.h TargetInfo.cc \
                        voidptr.h \
                        TypedObject.h TypedObject.cc \
                        ActorStage.cc ActorStage.h \
                        Faction.cc Faction.h \
                        debug.h debug.cc \
                        Weak.h \
                        DataNode.h DataNode.cc
tnl_libs = \
    modules/scripting/libscripting.a \
    modules/LoDTerrain/libLoDTerrain.a \
    modules/actors/carrier/libcarrier.a \
    modules/actors/drone/libdrone.a \
    modules/actors/tank/libtank.a \
    modules/actors/projectiles/libprojectiles.a \
    modules/collide/libcollide.a \
    modules/actors/fx/libfx.a \
    modules/actors/libactors.a \
    modules/camera/libcamera.a \
    modules/clock/libclock.a \
    modules/config/libconfig.a \
    modules/engines/libengines.a \
    modules/environment/libenvironment.a \
    modules/flight/libflight.a \
    modules/fontman/libfontman.a \
    modules/gunsight/libgunsight.a \
    modules/jogi/libjogi.a \
    modules/map/libmap.a \
    modules/physics/libphysics.a \
    modules/math/libmath.a \
    modules/model/libmodel.a \
    modules/skybox/libskybox.a \
    modules/ui/libui.a \
    modules/ai/libai.a \
    modules/texman/libtexman.a \
    modules/weaponsys/libweaponsys.a

tnl_bin_LDADD = $(tnl_libs) @SDL_LIBS@ @SIGC_LIBS@ @OPENGL_LIBS@  \
    @OPENAL_LIBS@ @LIBPNG_LIBS@ @IO_LIBS@

INCLUDES = @SDL_CFLAGS@ @SIGC_CFLAGS@ @OPENGL_CFLAGS@ @OPENAL_CFLAGS@
