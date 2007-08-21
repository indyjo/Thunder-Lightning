SUBDIRS =       \
    carrier     \
    drone       \
    fx          \
    projectiles \
    tank
    
noinst_LIBRARIES = libactors.a

libactors_a_SOURCES = \
	Observer.cc Observer.h \
	RigidActor.h RigidActor.cc \
	simpleactor.cc simpleactor.h \
	SimpleView.h SimpleView.cc 
        
INCLUDES = -I${top_srcdir}/src
