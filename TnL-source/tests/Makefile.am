dist-hook:
	mkdir $(distdir)/cxxtest \
	    cp -p $(srcdir)/cxxtest/* $(distdir)/cxxtest

noinst_PROGRAMS = tnltest


runner.cc: Makefile
	python $(srcdir)/cxxtest/cxxtestgen.py --error-printer -o $@ $(srcdir)/*.h
	
tnltest_SOURCES = DummySuite.h CollidePrimitivesSuite.h
nodist_tnltest_SOURCES = runner.cc

BUILT_SOURCES = runner.cc
CLEANFILES = runner.cc

tnltest_libs = \
    ../src/modules/scripting/libscripting.a \
    ../src/modules/LoDTerrain/libLoDTerrain.a \
    ../src/modules/actors/carrier/libcarrier.a \
    ../src/modules/actors/drone/libdrone.a \
    ../src/modules/actors/tank/libtank.a \
    ../src/modules/collide/libcollide.a \
    ../src/modules/actors/libactors.a \
    ../src/modules/camera/libcamera.a \
    ../src/modules/clock/libclock.a \
    ../src/modules/config/libconfig.a \
    ../src/modules/engines/libengines.a \
    ../src/modules/environment/libenvironment.a \
    ../src/modules/flight/libflight.a \
    ../src/modules/fontman/libfontman.a \
    ../src/modules/gunsight/libgunsight.a \
    ../src/modules/jogi/libjogi.a \
    ../src/modules/physics/libphysics.a \
    ../src/modules/math/libmath.a \
    ../src/modules/model/libmodel.a \
    ../src/modules/skybox/libskybox.a \
    ../src/modules/ui/libui.a \
    ../src/modules/ai/libai.a \
    ../src/modules/texman/libtexman.a \
    ../src/modules/weaponsys/libweaponsys.a \
    ../src/modules/actors/projectiles/libprojectiles.a \
    ../src/modules/actors/fx/libfx.a \
    ../src/modules/drawing/libdrawing.a \
    ../src/debug.o

tnltest_LDADD = $(tnltest_libs) @SDL_LIBS@ @SIGC_LIBS@ @OPENGL_LIBS@  \
    @OPENAL_LIBS@ @ALUT_LIBS@ @LIBPNG_LIBS@ @IO_LIBS@

INCLUDES = -I$(srcdir)/cxxtest -I$(srcdir)/../src @SDL_CFLAGS@ @SIGC_CFLAGS@ @OPENGL_CFLAGS@ @OPENAL_CFLAGS@

tnltest: runner.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) runner.cc $(tnltest_LDADD) $(LDFLAGS) -o $@

test: clean tnltest
	./tnltest

