noinst_LIBRARIES = libai.a
noinst_PROGRAMS = test_astar

INCLUDES = -I${top_srcdir}/src

libai_a_SOURCES =                       \
    AStar.cc AStar.h                    \
    Command.cc Command.h                \
    Idea.cc Idea.h                      \
    PathManager.cc PathManager.h

test_astar_SOURCES = test_astar.cc AStar.cc AStar.h
