SUBDIRS =       \
    drone       \
    fx          \
    projectiles \
    tank
    
noinst_LIBRARIES = libactors.a

libactors_a_SOURCES = \
	Observer.cc Observer.h \
	RelativeView.h RelativeView.cc \
	RigidActor.h RigidActor.cc \
	simpleactor.cc simpleactor.h \
	SimpleView.h SimpleView.cc 
        
INCLUDES = -I${top_srcdir}/src
