#include "model.h"
#include <fstream>
#include <modules/actors/fx/DebugObject.h>

using namespace std;

void Model::parseObjFile(TextureManager & texman, istream & in, const char *dir)
{
    string op;
    //int v_idx=0, n_idx=0, t_idx=0, f_idx=0;

    mtls["Default"];                      // Add a default material
    grps.push_back( Group(mtls["Default"]) ); // Add a default group

    while (in >> op) {
        //ls_message("[Model: Operator %s]\n", op.c_str());

        if (op == "v") {                // Vertice
            Vector vec;
            in >> vec[0] >> vec[1] >> vec[2];
            //ls_message("[  vec: %f %f %f]\n", vec[0], vec[1], vec[2]);
            v.push_back(vec);
        } else if ( op == "vn") {       // Normal
            Vector vec;
            in >> vec[0] >> vec[1] >> vec[2];
            //ls_message("[  vn: %f %f %f]\n", vec[0], vec[1], vec[2]);
            vec.normalize();
            n.push_back(vec);
        } else if ( op == "vt") {       // UVW
            Vector vec;
            in >> vec[0] >> vec[1] >> vec[2];
            //ls_message("[  vt: %f %f %f]\n", vec[0], vec[1], vec[2]);
            t.push_back(vec);
        } else if ( op == "mtllib" ) {  // Load materials
            string filename;
            in >> filename;
            filename = string(dir) + "/" + filename;
            //ls_message("[  mtllib: %s]\n", filename.c_str());
            ifstream file(filename.c_str());
            parseMtlFile(texman, file, dir);
        } else if ( op == "usemtl" ) {  // Start new Material+faces group
            string mtlname;
            in >> mtlname;
            grps.push_back( Group(mtls[mtlname]) );
        } else if ( op == "f" ) {       // Face
            Face face;
            char sep;
            for (int i=0; i<3; i++) {
                if ( n.size() == 0 && t.size() == 0 ) {
                    in >> face.v[i];
                } else if (n.size() != 0 && t.size() == 0) {
                    in >> face.v[i] >> sep >> sep >> face.n[i];
                } else if (n.size() == 0 && t.size() != 0) {
                    in >> face.v[i] >> sep >> face.t[i];
                } else {
                    in >> face.v[i] >> sep >> face.t[i] >> sep >> face.n[i];
                }
                --face.v[i];
                --face.t[i];
                --face.n[i];
                //ls_message("%d/%d/%d%c",
                //        face.v[i], face.n[i], face.t[i], i<2?' ':'\n');
            }
            grps.back().f.push_back( face );
        } else {
            // Unknown operator, ignore till next line
            while(in.get() != '\n' && in) { }
            //ls_warning("Operator %s unknown.\n", op.c_str());
        }

        // Read to end of line
        //ls_message("eol?\n");
        //while(in.get() != '\n') {
        //    if (in.eof()) return;
        //}
        //ls_message("got it!\n");
        //ls_message("Stream state:  good=%s eof=%s fail=%s bad=%s\n",
        //        in.good()?"true":"false",
        //        in.eof()?"true":"false",
        //        in.fail()?"true":"false",
        //        in.bad()?"true":"false");
    }
    //ls_message("Parsing complete!\n");
}


void Model::parseMtlFile(TextureManager & texman, istream & in, const char *dir)
{
    string op;
    string mtlname;

    while (in >> op) {
        if (op == "newmtl") {
            in >> mtlname;

            mtls[mtlname] = Material();
        } else if ( op == "map_Kd" ) {
            string filename;
            in >> filename;

            // If we have a relative file name prepend the given directory
            if (filename[0]!='/') {
                filename = string(dir) + "/" + filename;
            }
            mtls[mtlname].tex = texman.query(filename.c_str());
            mtls[mtlname].use_tex = true;
            mtls[mtlname].w = mtls[mtlname].tex.getWidth();
            mtls[mtlname].h = mtls[mtlname].tex.getHeight();
        } else {
            // Unknown operator, ignore till next line
            while(in.get() != '\n' && in) { }
            ls_warning("Operator %s unknown.\n", op.c_str());
        }
    }
}


void Model::draw(JRenderer & r, const Matrix & Mmodel, const Matrix & Mnormal)
{
    Vector vtx;
    Vector col(1,1,1);

    //r.setCullMode(JR_CULLMODE_NO_CULLING);
    for (list<Group>::iterator grp = grps.begin(); grp != grps.end(); grp++) {
        vector<Face> &f=grp->f;
        if (grp->mtl.use_tex) {
            r.setTexture(grp->mtl.tex);
            r.setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
        } else {
        	r.setVertexMode(JR_VERTEXMODE_GOURAUD);
        }
        r.pushMatrix();
        r.multMatrix(Mmodel);
        r.begin(JR_DRAWMODE_TRIANGLES);
        r.setAlpha(1.0);
        for (int i=0; i<f.size(); i++) {
            for (int j=0; j<3; j++) {
                vtx = v[f[i].v[j]];

                //vtx = Mmodel * vtx;
                col = Vector(1,1,1);

                if (n.size()) {
                    Vector norm = Mnormal * n[f[i].n[j]];
                    Vector to_sun(-1,1,0);
                    to_sun.normalize();
                    float light = to_sun * norm;
                    if (light < 0) light = 0;
                    light += 0.3; // TODO: ambient light
                    if (light > 1) light = 1;
                    //light = 1.0;
                    col *= light;
                }

                if (t.size()) {
                    float u =       t[f[i].t[j]][0];
                    float v = 1.0 - t[f[i].t[j]][1];
                    r.setUVW( Vector(u,v,0) );
                }

                r.setColor(col);
                r.vertex(vtx);
            }
        }
        r.end();
        r.popMatrix();
    }
}


#define SQUARE_SIZE 100.0
#define DELTA -0.5
void Model::debugTextures(JRenderer & r, const Matrix & M) {
    typedef vector<Face>::iterator VFI;
    typedef list<Group>::iterator LGI;

    jvertex_coltxt vtx;
    Vector pivot(-SQUARE_SIZE/2, SQUARE_SIZE/2, 0);
    Vector vec;
    const float xdif[4]={0, 1, 1, 0};
    const float ydif[4]={0, 0, -1, -1};
    const int polygon[4] = {0, 1, 2, 0};

    vtx.col.r = vtx.col.g = vtx.col.b = 255;

    r.setCullMode(JR_CULLMODE_CULL_NEGATIVE);

    for(LGI group=grps.begin(); group!=grps.end(); group++) {
        if (!group->mtl.use_tex) continue;

        // draw a textured square
        r.setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
        r.setTexture(group->mtl.tex);
        r.begin(JR_DRAWMODE_TRIANGLE_FAN);
        for (int corner=0; corner<4; corner++) {
            vec = pivot + SQUARE_SIZE * Vector(xdif[corner], ydif[corner], 0);
            vec = M * vec;

            vtx.p.x = vec[0];
            vtx.p.y = vec[1];
            vtx.p.z = vec[2];

            vtx.txt.x = xdif[corner] * group->mtl.w;
            vtx.txt.y = ydif[corner] * group->mtl.h;

            r.addVertex(&vtx);
        }
        r.end();
        // now splat the texture coordinates on it
        r.setVertexMode(JR_VERTEXMODE_GOURAUD);
        //r.enableAlphaBlending();
        //r.setAlpha(1.0);
        for (VFI face=group->f.begin(); face!=group->f.end(); face++) {
            //ls_message("face: %p, face->n={%d, %d, %d}\n",
            //        (void *) face, face->n[0], face->n[1], face->n[2]);
            r.begin(JR_DRAWMODE_CONNECTED_LINES);
            for (int i=0; i<4; i++) {
                float x = SQUARE_SIZE *  t[face->t[ polygon[i] ]][0];
                float y = SQUARE_SIZE * -t[face->t[ polygon[i] ]][1];
                //ls_message("x=%f y=%f face->t[%d] = %d \n", x,y,polygon[i],
                //        face->t[polygon[i]]);
                //ls_message("t[%d] = %d %d %d\n", face->t[polygon[i]],
                //        t[face->t[polygon[i]]][0],
                //        t[face->t[polygon[i]]][1],
                //        t[face->t[polygon[i]]][2]);

                vec = pivot + Vector(x, y, -DELTA);
                vec = M * vec;

                vtx.p.x = vec[0];
                vtx.p.y = vec[1];
                vtx.p.z = vec[2];

                r.addVertex(&vtx);
            }
            r.end();
        }

        pivot+=Vector(SQUARE_SIZE, 0, 0);
    }
}






void Model::dump() {
    ls_message("Model@%p dump following:\n", this);
    ls_message("  model in directory %s consists of:\n", path.c_str());
    ls_message("    %d materials\n",      mtls.size());
    ls_message("    %d vertices\n",       v.size());
    ls_message("    %d normals\n",        n.size());
    ls_message("    %d texture coords\n", t.size());
    ls_message("    %d groups\n",         grps.size());

    ls_message("  materials:\n");
    for(map<string, Material>::iterator i=mtls.begin(); i!=mtls.end(); i++) {
        ls_message("    %s ", i->first.c_str());
        if(i->second.use_tex) {
            ls_message("uses texture with id %d\n", (jrtxtid_t)i->second.tex);
        } else {
            ls_message("\n");
        }
    }
    ls_message("  groups:\n");
    int j=0;
    for(list<Group>::iterator i=grps.begin(); i!=grps.end(); i++) {
        ls_message("    %3d: %d faces using material ",j++, i->f.size());
        for(map<string,Material>::iterator m=mtls.begin(); m!=mtls.end(); m++ ){
            if (&m->second == &i->mtl) {
                ls_message("%s", m->first.c_str());
            }
        }
        ls_message("\n");
        typedef vector<Face>::iterator VFI;
        for(VFI face=i->f.begin(); face!=i->f.end(); face++) {
            ls_message("          ");
            for( int i=0; i<3; i++) {
                ls_message("%d/%d/%d ",
                        face->v[i],
                        face->n[i],
                        face->t[i]);
            }
            //ls_message("@ %p\n", (void*) face);
            ls_message("\n");
        }
    }
    ls_message("  texture coords:\n");
    for(vector<Vector>::iterator vec=t.begin(); vec!=t.end(); vec++) {
        ls_message("    %f %f %f\n", (*vec)[0], (*vec)[1], (*vec)[2]);
    }
}
