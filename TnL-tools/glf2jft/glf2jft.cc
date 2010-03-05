#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>

// JOGI includes
#include "jogi/types.h"
#include "jogi/JSprite.h"

using namespace std;

class Exception { };

//glFont character structure
struct GlFontChar
{
	float dx, dy;
	float tx1, ty1;
	float tx2, ty2;
};

ostream & operator<< (ostream & out, const GlFontChar & c) {
    out << c.dx << " " << c.dy << "\t";
    out << c.tx1 << " " << c.ty1 << "\t" << c.tx2 << " " << c.ty2;
    return out;
}

//glFont structure
struct GlFont
{
	int TexWidth, TexHeight;
	int IntStart, IntEnd;
	GlFontChar *Char;
    char * TexBytes;
    
    GlFont(const char *FileName);
    ~GlFont();
    
    void saveTo(const char *jft_filename, const char *spr_filename);
};

GlFont::GlFont(const char *FileName)
{
    ifstream in(FileName);
	int Num;

    if (!in) {
        cerr << "File " << FileName << " couldn't be opened." << endl;
        throw Exception();
    }
    
	//Read glFont structure
    in.ignore(4);
    in.read((char*) &TexWidth, 4);
    in.read((char*) &TexHeight, 4);
    in.read((char*) &IntStart, 4);
    in.read((char*) &IntEnd, 4);
    in.ignore(4);

	//Get number of characters
	Num = IntEnd - IntStart + 1;
	
	//Allocate memory for characters
    Char = new GlFontChar[Num];
    if (!Char) {
        cerr << "Couldn't allocate " << Num << " bytes." << endl;
        throw Exception();
    }
	
	//Read glFont characters
    in.read((char*)Char, sizeof(GlFontChar) * Num);

	//Get texture size
	Num = TexWidth * TexHeight * 2;
	
	//Allocate memory for texture data
    TexBytes = new char[Num];
    if (!TexBytes) {
        cerr << "Couldn't allocate " << Num << " bytes." << endl;
        throw Exception();
    }
	
	//Read texture data
    in.read(TexBytes, Num);
}

GlFont::~GlFont()
{
	delete [] TexBytes;
    delete [] Char;
}

void GlFont::saveTo(const char *jft_filename, const char *spr_filename) {
    ofstream jft(jft_filename);
    if (!jft) {
        cerr << "Couldn't open file " << jft_filename << " for writing.";
        cerr << endl;
        throw Exception();
    }
    
    jft << spr_filename << endl;
    int n = IntEnd - IntStart + 1;
    jft << n << endl;
    for (int i=0; i<n; i++) jft << i + IntStart << " " << Char[i] << endl;
    jft.close();
    if (!jft) {
        cerr << "An Error has occured while writing to " << jft_filename;
        cerr << endl;
        throw Exception();
    }
    
    JSprite S;
    S.sprite.buf = (ju32*) malloc(TexWidth * TexHeight * 4);
    S.sprite.x = S.sprite.y = 0;
    S.sprite.w = TexWidth;
    S.sprite.h = TexHeight;
    ofstream spr(spr_filename);
    for(int y=0; y<TexHeight; y++) for(int x=0; x<TexWidth; x++) {
        S.sprite.buf[y*TexWidth+x] = 0xffffff | (TexBytes[(y*TexWidth+x)*2] << 24);
    }
    S.save(spr);
    if (!spr) {
        cerr << "An Error has occured while writing to " << spr_filename;
        cerr << endl;
        throw Exception();
    }
}    

void usage() {
    cout << "glf2jft - Convert font data in glf format into jft format."<<endl;
    cout << "Usage: glf2jft <glf-file> <jft-file> <spr-file>" << endl;
    cout << "Where" << endl;
    cout << "\t<glf-file> is the glFont file that contains the"
            " original font data." << endl;
    cout << "\t<jft-file> is the JoFont file that you want to produce." << endl;
    cout << "\t<spr-file> is the font sprite that contains the glyphs." << endl;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        usage();
        return 0;
    }
    
    try {
        GlFont glf(argv[1]);
        glf.saveTo(argv[2], argv[3]);
    } catch (Exception) {
        cerr << "Aborting." << endl;
        return -1;
    }
    
    return 0;
}
