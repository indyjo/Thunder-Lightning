# convert.cc convert.h exportPNG.cc exportPNG.h main.cc

CXXFLAGS+=-I.
CXXFLAGS+=`pkg-config --cflags freetype2 libpng`
LDFLAGS+=`pkg-config --libs --static freetype2 libpng`

all: fontopus

fontopus: convert.o exportPNG.o main.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -f fontopus *.o

