CXXFLAGS+=`pkg-config --cflags libpng`
LDFLAGS+=`pkg-config --libs --static libpng`

all: glf2jft

glf2jft: sprite
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o glf2jft glf2jft.cc jogi/JSprite.o

sprite:
	cd jogi && make

clean:
	rm -f glf2jft jogi/JSprite.o

