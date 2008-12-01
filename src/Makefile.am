SUBDIRS = modules boost interfaces

bin_PROGRAMS = tnl-bin

tnl_bin_SOURCES = game.cc game.h tnl.h \
                        glew.c glew.h glxew.h wglew.h \
                        CEGUIEventFilter.cc CEGUIEventFilter.h \
                        Factory.h \
                        object.cc object.h \
                        util.h \
                        signals.h \
                        initControls.cc \
                        remap.h remap.cc \
                        sound.cc sound.h \
                        TargetInfo.h TargetInfo.cc \
                        voidptr.h \
                        TypedObject.h TypedObject.cc \
                        ActorStage.cc ActorStage.h \
                        Faction.cc Faction.h \
                        debug.h debug.cc \
                        Weak.h \
                        DataNode.h DataNode.cc \
                        profile.h \
                        RenderContext.cc RenderContext.h \
                        RenderPass.cc RenderPass.h \
                        SceneRenderPass.cc SceneRenderPass.h \
                        AxisTransform.cc AxisTransform.h \
                        TnlCeguiLogger.cc TnlCeguiLogger.h
                     
tnl_libs = \
    modules/scripting/libscripting.a \
    modules/LoDTerrain/libLoDTerrain.a \
    modules/actors/carrier/libcarrier.a \
    modules/actors/drone/libdrone.a \
    modules/actors/tank/libtank.a \
    modules/collide/libcollide.a \
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
    modules/physics/libphysics.a \
    modules/math/libmath.a \
    modules/model/libmodel.a \
    modules/skybox/libskybox.a \
    modules/ui/libui.a \
    modules/ai/libai.a \
    modules/texman/libtexman.a \
    modules/weaponsys/libweaponsys.a \
    modules/actors/projectiles/libprojectiles.a \
    modules/actors/fx/libfx.a \
    modules/drawing/libdrawing.a

tnl_bin_LDADD = $(tnl_libs) @SDL_LIBS@ @SIGC_LIBS@ @OPENGL_LIBS@  \
    @OPENAL_LIBS@ @ALUT_LIBS@ @LIBPNG_LIBS@ @IO_LIBS@

INCLUDES = @SDL_CFLAGS@ @SIGC_CFLAGS@ @OPENGL_CFLAGS@ @OPENAL_CFLAGS@ @ALUT_CFLAGS@

# Compile the lonely .c file (glew.c) like the .cc files
AM_CFLAGS=$(CXXFLAGS)

