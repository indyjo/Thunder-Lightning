noinst_LIBRARIES = libcollide.a
#noinst_PROGRAMS = bb_test snp_test prim_test

INCLUDES = -I${top_srcdir}/src

libcollide_a_SOURCES =                         \
        BoundingBox.cc BoundingBox.h           \
        BoundingGeometry.cc BoundingGeometry.h \
        BoundingNode.cc BoundingNode.h         \
        Collidable.h Collidable.cc             \
        Contact.h Contact.cc                   \
        ContactPartner.h                       \
        GeometryInstance.h GeometryInstance.cc \
        PossibleContact.h PossibleContact.cc   \
        Primitive.cc Primitive.h               \
        SweepNPrune.h                          \
        CollisionManager.cc CollisionManager.h


#prim_test_SOURCES = prim_test.cc Primitive.h Primitive.cc

#bb_test_SOURCES = BoundingBox.cc BoundingBox.h bb_test.cc
#bb_test_LDFLAGS = ../math/libmath.a

#snp_test_SOURCES = SweepNPrune.h snp_test.cc
