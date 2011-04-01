#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include "texgen.h"
#include "texscript.h"
#include "Vector.h"
#include "perlin.h"

using namespace std;

bool parse(TexGen * tg, ScriptHost *sh, const char *file);

void HeightSource::addImage(Image *img, int u, int v) {
    images.push_back( make_pair(
            Rect(u,v,img->getWidth(), img->getHeight()),
            img));
}

int HeightSource::heightAt(int u, int v) {
    for(Images::iterator i=images.begin(); i!=images.end(); i++) {
        Rect r(i->first);
        if (r.contains(u,v)) {
            return (i->second->pixelAt(u - r.u, v - r.v) * scale) / 256;
        }
    }
    return 0;
}

class Float_Height : public Expression {
    TexGen & texgen;
public:
    Float_Height(TexGen & tg) : texgen(tg) { }
    Data eval() {
        int u = texgen.u;
        int v = texgen.v;
        HeightSource & hs = texgen.heightsource;
        return (float)(texgen.dims.w * hs.heightAt(u,v) / 65536.0);
    }
};

class Float_AverageHeightInt : public Expression {
    TexGen & texgen;
public:
    Float_AverageHeightInt(TexGen & tg) : texgen(tg) { }
    Data eval() {
        int radius = params[0]->eval();
        int uc = texgen.u;
        int vc = texgen.v;
        int h = 0;
        HeightSource & hs = texgen.heightsource;
        for (int v=vc-radius; v<=vc+radius; v++) {
            for (int u=uc-radius; u<=uc+radius; u++) {
                h += hs.heightAt(u,v);
            }
        }
        float avg_height = (float) h / (float) ((2*radius+1)*(2*radius+1));
        return (float)(texgen.dims.w * avg_height / 65536.0);
    }
};


class Float_HeightIntInt : public Expression {
    TexGen & texgen;
public:
    Float_HeightIntInt(TexGen & tg) : texgen(tg) { }
    Data eval() {
        int u = params[0]->eval();
        int v = params[1]->eval();
        HeightSource & hs = texgen.heightsource;
        return (float)(texgen.dims.w * hs.heightAt(u,v) / 65536.0);
    }
};

class Int_U : public Expression {
    TexGen & texgen;;
public:
    Int_U(TexGen & tg) : texgen(tg) { }
    Data eval() {
        return texgen.u;
    }
};

class Int_V : public Expression {
    TexGen & texgen;;
public:
    Int_V(TexGen & tg) : texgen(tg) { }
    Data eval() {
        return texgen.v;
    }
};


#define PI 3.14159265358979323846
class Float_Slope : public Expression {
    TexGen & texgen;
public:
    Float_Slope(TexGen & tg) : texgen(tg) { }
    Data eval() {
        int u = texgen.u;
        int v = texgen.v;
        HeightSource & hs = texgen.heightsource;
        float su = texgen.dims.u / (float) texgen.width;
        float sv = texgen.dims.v / (float) texgen.height;
        float h00 = texgen.dims.w * (float)hs.heightAt(u,v) / 65536.0;
        float h10 = texgen.dims.w * (float)hs.heightAt(u+1,v) / 65536.0;
        float h01 = texgen.dims.w * (float)hs.heightAt(u,v+1) / 65536.0;
        float h11 = texgen.dims.w * (float)hs.heightAt(u+1,v+1) / 65536.0;
        
        Vector v1 = (Vector(su,0,h10-h00) % Vector(0,sv,h01-h00)).normalize();
        Vector v2 = (Vector(-su,0,h01-h11) % Vector(0,-sv,h10-h11)).normalize();
        Vector n = (v1+v2).normalize();
        
        return (float) (acos(n[2]) * 180.0 / PI);
    }
};

class Float_SlopeIntInt : public Expression {
    TexGen & texgen;
public:
    Float_SlopeIntInt(TexGen & tg) : texgen(tg) { }
    Data eval() {
        int u = params[0]->eval();
        int v = params[1]->eval();
        HeightSource & hs = texgen.heightsource;
        float su = texgen.dims.u / (float) texgen.width;
        float sv = texgen.dims.v / (float) texgen.height;
        float h00 = texgen.dims.w * (float)hs.heightAt(u,v) / 65536.0;
        float h10 = texgen.dims.w * (float)hs.heightAt(u+1,v) / 65536.0;
        float h01 = texgen.dims.w * (float)hs.heightAt(u,v+1) / 65536.0;
        float h11 = texgen.dims.w * (float)hs.heightAt(u+1,v+1) / 65536.0;
        
        Vector v1 = (Vector(su,0,h10-h00) % Vector(0,sv,h01-h00)).normalize();
        Vector v2 = (Vector(-su,0,h01-h11) % Vector(0,-sv,h10-h11)).normalize();
        Vector n = (v1+v2).normalize();
        
        return (float) (acos(n[2]) * 180.0 / PI);
    }
};

class Float_Fract : public Expression {
    TexGen & texgen;
public:
    Float_Fract(TexGen & tg) : texgen(tg) { }
    Data eval() {
        float u = (float) texgen.u / (float) texgen.width;
        float v = (float) texgen.v / (float) texgen.height;
        float persistance = 0.5;
        float amplitude = 1.0;
        int steps = 8;
        float result = 0.0;
        for(int i=0; i<steps; i++) {
            result += amplitude * perlin::noise2(u,v);
            amplitude *= persistance;
            u *= 2.0;
            v *= 2.0;
        }
        return result;
    }
};

class Float_FractIntFloatFloatFloatFloat : public Expression {
    TexGen & texgen;
public:
    Float_FractIntFloatFloatFloatFloat(TexGen & tg) : texgen(tg) { }
    Data eval() {
        float u = (float) texgen.u / (float) texgen.width;
        float v = (float) texgen.v / (float) texgen.height;
        int steps =         params[0]->eval();
        float persistance = params[1]->eval();
        float size =        params[2]->eval();
        float du =          params[3]->eval();
        float dv =          params[4]->eval();
        float amplitude = 1.0;
        float result = 0.0;
        for(int i=0; i<steps; i++) {
            result += amplitude * perlin::noise2(size*u+du,size*v+dv);
            amplitude *= persistance;
            size *= 2.0;
        }
        return result;
    }
};

template<class T, class P1>
class Factory1 : public ExpressionFactory {
    P1 p1;
public:
    inline Factory1(P1 p1) : p1(p1) { }
    Expression * create() { return new T(p1); }
};

TexGen::TexGen(int argc, char **argv) : is_ok(true) {
    if (argc <= 1) {
        is_ok = false;
        usage();
        return;
    }
    
    // Skip first parameter (binary name)
    argc--;
    argv++;
    
    // Standard values or initialisation reminders
    bool scriptfile_given = false;
    width = height = 512;
    bool outfile_given = false;
    
    
    while (argc) switch( parseParam(&argc, &argv) ) {
        case 0:
        case 'u': usage(); is_ok = false; return;
        case 'o':
            if (!parseString(&argc, &argv, &outfile)) {
                printf("-o or --output needs a parameter.\n");
                is_ok = false; return;
            }
            outfile_given = true;
            break;
        case 's':
            if (!parseString(&argc, &argv, &scriptfile)) {
                printf("-s or --script needs a parameter.\n");
                is_ok = false; return;
            }
            scriptfile_given = true;
            break;
        case 'w':
            if (!parseInt(&argc, &argv, &width)) {
                printf("-w or --width needs a parameter.\n");
                is_ok = false; return;
            }
            break;
        case 'h':
            if (!parseInt(&argc, &argv, &height)) {
                printf("-h or --height needs a parameter.\n");
                is_ok = false; return;
            }
            break;
        case 'i':
            if (!parseInfile(&argc, &argv)) {
                printf("-i or --input needs a parameter.\n");
                is_ok = false; return;
            }
            break;
        default:
            usage(); is_ok = false; return;
    }
    
    if (!scriptfile_given) {
        printf("You must give a script file with the -s or --script parameter.\n");
        is_ok = false;
    } else if (!ifstream(scriptfile.c_str())) {
        printf("The given script file doesn't exist: %s\n", scriptfile.c_str());
        is_ok = false;
    }
    if (!outfile_given) {
        printf("You must give an output file with the -o or --output parameter.\n");
        is_ok = false;
    }
    if (!is_ok) return;
    
    scripthost.add("height", Spec(INT, INT, FLOAT),
            new Factory1< Float_HeightIntInt, TexGen & >(*this) );
    scripthost.add("height", Spec(FLOAT),
            new Factory1< Float_Height, TexGen & >(*this) );
    scripthost.add("average_height", Spec(INT, FLOAT),
            new Factory1< Float_AverageHeightInt, TexGen & >(*this) );
    scripthost.add("u", Spec(INT), new Factory1< Int_U, TexGen & >(*this) );
    scripthost.add("v", Spec(INT), new Factory1< Int_V, TexGen & >(*this) );
    scripthost.add("slope", Spec(FLOAT),
            new Factory1< Float_Slope, TexGen & >(*this) );
    scripthost.add("slope", Spec(INT, INT, FLOAT),
            new Factory1< Float_SlopeIntInt, TexGen & >(*this) );
    scripthost.add("fract", Spec(FLOAT),
            new Factory1< Float_Fract, TexGen & >(*this) );
    Type if4[5] = {INT, FLOAT, FLOAT, FLOAT, FLOAT};
    scripthost.add("fract", Spec(5, if4, FLOAT),
            new Factory1<Float_FractIntFloatFloatFloatFloat, TexGen &>(*this));
    
    parse(this, &scripthost, scriptfile.c_str());
}

bool TexGen::ok() {
    return is_ok;
}

void TexGen::run() {
    Image img(width, height);
    for(Layers::iterator i = layers.begin(); i!=layers.end(); i++)
        img.paletteAt(i->idx) = (i->r << 16) | (i->g << 8) | i->b;
    for(v=0; v<height; v++) for(u=0; u<width; u++) {
        for(Layers::iterator i = layers.begin(); i!=layers.end(); i++) {
            if ((bool) i->expr->eval()) {
                img.pixelAt(u,v) = i->idx;
                break;
            }
        }
    }
    img.saveTo(outfile.c_str());
}

void TexGen::addLayer(int idx, int r, int g, int b, Expression * expr) {
    printf("added layer %d color rgb %d %d %d expr %p\n", idx, r,g,b, expr);
    Layer l;
    l.idx = idx;
    l.r = r;
    l.g = g;
    l.b = b;
    l.expr = expr;
    layers.push_back(l);
}

void TexGen::usage() {
    printf("texgen usage:\n");
    printf("texgen [parameters]\n");
    printf("parameters:\n");
    printf("  -s or --script file.texgen    Load file.texgen as script\n");
    printf("  -i or --infile file.tga:u:v   Load file.tga at position (u,v)\n");
    printf("  -o or --outfile file.tga      Save output file to file.tga\n");
    printf("  -w or --width                 Set output image width\n");
    printf("  -h or --height                Set output image height\n");
    printf("  -u or --usage or --help       Display this information\n");
    printf("\n");
}

char TexGen::parseParam(int *argc, char ***argv) {
    char *param = (*argv)[0];
    if (!param[0] == '-') return 0;
    param++;
    
    (*argc)--;
    (*argv)++;
    
    if (!strcmp(param, "u") || !strcmp(param, "-usage")
            || !strcmp(param, "-help")) return 'u';
    if (!strcmp(param, "w") || !strcmp(param, "-width")) return 'w';
    if (!strcmp(param, "h") || !strcmp(param, "-height")) return 'h';
    if (!strcmp(param, "i") || !strcmp(param, "-input")) return 'i';
    if (!strcmp(param, "o") || !strcmp(param, "-output")) return 'o';
    if (!strcmp(param, "s") || !strcmp(param, "-script")) return 's';
    
    printf("Couldn't recognize param %s\n", param);
    
    return 0;
}

bool TexGen::parseString(int * argc, char ***argv, string * s) {
    if (!*argc) return false;
    if ((*argv[0])[0] == '-') return false;
    *s = *argv[0];
    (*argc)--;
    (*argv)++;
    return true;
}

bool TexGen::parseInt(int * argc, char ***argv, int * n) {
    if (!*argc) return false;
    if ((*argv[0])[0] == '-') return false;
    *n = atoi(*argv[0]);
    (*argc)--;
    (*argv)++;
    return true;
}

bool TexGen::parseInfile(int * argc, char ***argv) {
    if (!*argc) return false;
    if ((*argv[0])[0] == '-') return false;
    char buf[256];
    int u,v;
    if (3 != sscanf(*argv[0], "%255[^:]:%d:%d", buf, &u, &v)) return false;
    if (!ifstream(buf)) {
        printf("The given input file doesn't exist: %s\n", buf);
        return false;
    }
    heightsource.addImage(new Image(buf), u, v);
    (*argc)--;
    (*argv)++;
    return true;
}

int main(int argc, char **argv) {
    TexGen tg(argc, argv);
    if (tg.ok()) tg.run();
    else return -1;
    return 0;
}

    
