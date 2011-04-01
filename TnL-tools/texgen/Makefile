all: perlinmodule.so

perlinmodule.so: perlin.c perlin_wrap.c
	gcc -shared -I /usr/lib/python1.5/config/ -I /usr/include/python1.5/ \
            -o perlinmodule.so perlin.c perlin_wrap.c

perlin_wrap.c: perlin.c perlin.h perlin.i
	swig -python perlin.i

clean:
	rm -f *.o *.pyo *.pyc *.so

