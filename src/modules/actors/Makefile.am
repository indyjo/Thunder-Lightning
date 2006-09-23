SUBDIRS =       \
    drone       \
    fx          \
    projectiles \
    tank
    
noinst_LIBRARIES = libactors.a

libactors_a_SOURCES = simpleactor.cc simpleactor.h \
	RelativeView.h RelativeView.cc \
	RigidActor.h RigidActor.cc \
	SimpleView.h SimpleView.cc 
        
INCLUDES = -I${top_srcdir}/src
