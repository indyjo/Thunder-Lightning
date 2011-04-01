CXXFLAGS=-Wall

texgen2: texscript.o parser.o texgen.o image.o perlin.o
	g++ $^ -o $@

texscript.o: texscript.cc texscript.h

parser.o: parser.cc lex.yy.cc

image.o: image.cc image.h

texgen.o: texgen.cc texgen.h perlin.h

perlin.o: perlin.cc perlin.h

parser.cc: script.y texscript.h
	bison -o parser.cc script.y
        
lex.yy.cc: script.lex
	flex++ script.lex

clean:
	rm -f texgen2 *.o

