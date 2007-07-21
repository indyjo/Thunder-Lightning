#ifndef TEXGEN_H
#define TEXGEN_H

// for class pair
#include <utility>
#include <string>
#include <vector>
#include "image.h"
#include "texscript.h"

class HeightSource {
    struct Rect { 
        Rect() { }
        Rect(int u, int v, int w, int h) : u(u), v(v), w(w), h(h) { }
        inline bool contains(int pu, int pv) {
            return u<= pu && pu <= u+w && v<=pv && pv <=v+h;
        }
        int u,v,w,h;
    };
    typedef std::vector<std::pair<Rect, Image*> > Images;
public:
    HeightSource() : scale(65536) { }
    void setScale(int s) { scale = s; };
    void addImage(Image *img, int u, int v);
    int heightAt(int u, int v);
    
protected:
    int scale;
    Images images;
};


class TexGen {
    friend class Float_Height;
    friend class Float_HeightIntInt;
    friend class Float_AverageHeightInt;
    friend class Int_U;
    friend class Int_V;
    friend class Float_Slope;
    friend class Float_SlopeIntInt;
    friend class Float_Fract;
    friend class Float_FractIntFloatFloatFloatFloat;
public:
    TexGen(int argc, char **argv);
    bool ok();
    void run();
    void addLayer(int idx, int r, int g, int b, Expression * expr);
    void setDims(float u, float v, float w) { dims.u=u; dims.v=v; dims.w=w; }
private:
    void usage();
    char parseParam(int * argc, char ***argv);
    bool parseString(int * argc, char ***argv, std::string * s);
    bool parseInt(int * argc, char ***argv, int * n);
    bool parseInfile(int * argc, char ***argv);
        
    struct Layer {
        int idx, r, g, b;
        Expression * expr;
    };
    typedef std::vector<Layer> Layers;
    Layers layers;
    ScriptHost scripthost;
    HeightSource heightsource;
    std::string outfile;
    std::string scriptfile;
    int width, height;
    int u,v;
    struct {float u, v, w; } dims;
    bool is_ok;
};

#endif
