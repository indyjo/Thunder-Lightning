noinst_LIBRARIES = libmath.a
noinst_PROGRAMS = test_solver test_quat test_ballist test_interval test_xvec test_quat_interval

INCLUDES = -I${top_srcdir}/src

libmath_a_SOURCES =                     \
        Ballistic.cc Ballistic.h        \
        Collide.cc Collide.h            \
	Feedback.cc Feedback.h		\
        Interval.cc Interval.h          \
        Line.cc Line.h                  \
	Matrix.h SpecialMatrices.h MatrixVector.h \
		PIDController.h PIDController.cc \
        Plane.h                         \
        Polynomial.cc Polynomial.h      \
        Quaternion.h                    \
        Rendezvous.cc Rendezvous.h      \
        Solve.cc Solve.h                \
        Transform.h                     \
	Vector.cc Vector.h

test_solver_SOURCES = test_solver.cc Solve.cc Solve.h
test_quat_SOURCES = test_quat.cc
test_interval_SOURCES = test_interval.cc Interval.cc
test_quat_interval_SOURCES = test_quat_interval.cc Interval.cc Vector.cc
test_xvec_SOURCES = test_xvec.cc Vector.h Matrix.h Vector.cc Interval.cc
test_ballist_SOURCES = Ballistic.cc Ballistic.h Solve.h Solve.cc test_ballist.cc
