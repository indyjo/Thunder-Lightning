#include "model.h"
#include <fstream>
#include <cctype>
#include <cstdio>
#include <modules/actors/fx/DebugObject.h>

using namespace std;

namespace {
	void read_to_eol(istream & in) {
        char c;
        do c = in.get();
        	while(in && c != '\n' && c !='\r');
        do c = in.get();
        	while(in && c == '\n' || c =='\r');
        if (in) in.putback(c);
	}
}
		

void Model::parseObjFile(TextureManager & texman, istream & in, const char *dir)
{
	//ls_message("Model::parseObjFile in dir %s\n", dir);
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
        } else if ( op == "vt") {       // UV(W)
            Vector vec;
            in >> vec[0] >> vec[1];
            char c;
            do in.get(c); while (isblank(c));
            if (c=='\r' ||  c=='\n') {
            	vec[2]=0;
            } else if (isdigit(c) || c=='+' || c=='-') {
            	in.unget();
            	in >> vec[2];
            }
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
            //ls_message("[  usemtl: %s]\n", mtlname.c_str());
            grps.push_back( Group(mtls[mtlname]) );
        } else if ( op == "f" ) {       // Face
        	char line[256];
        	vector<int> v,t,n;
        	char c;
        	
        	int count=0;
        	in.getline(line, 256);
        	//ls_message("line: %s\n",line);
        	for(char * tok = strtok(line, " ");
        		tok != NULL;
        		tok = strtok(NULL, " "))
        	{
        		//ls_message("tok: %s\n",tok);
            	int vv=-1,tt=-1,nn=-1;
            	if (3==sscanf(tok, "%d/%d/%d", &vv, &tt, &nn)) {
            	} else if(2==sscanf(tok, "%d//%d", &vv, &nn)) {
            		tt = 0;
            	} else if (1==sscanf(tok, "%d", &vv)) {
            		nn = tt = 0;
            	} else {
            		ls_error("Couldn't parse %s\n", tok);
            	}
            	v.push_back(vv-1);
            	t.push_back(tt-1);
            	n.push_back(nn-1);
                ++count;
        	}
        	
        	//ls_message("read %d-face\n", count);
            
            for (int i=0; i<count-2; ++i) {
	        	Face face;
	        	face.v[0] = v[0];
	        	face.n[0] = n[0];
	        	face.t[0] = t[0];
            	for (int j=1; j<3; ++j) {
	            	face.v[j] = v[i+j];
	            	face.n[j] = n[i+j];
	            	face.t[j] = t[i+j];
            	}
            	grps.back().f.push_back( face );
            }
        } else if ( op == "g" ) {
        	string groupname;
        	in >> groupname;
        	//ls_message("Group: %s\n", groupname.c_str());
        } else {
            // Unknown operator, ignore till next line
            read_to_eol(in);
            //ls_warning("OBJ Operator %s unknown.\n", op.c_str());
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
        } else if ( op == "Kd" ) {
        	Vector v;
        	in >> v[0] >> v[1] >> v[2];
        	mtls[mtlname].Kd = v;
        } else if ( op == "Ka" ) {
        	Vector v;
        	in >> v[0] >> v[1] >> v[2];
        	mtls[mtlname].Ka = v;
        } else if ( op == "Ks" ) {
        	Vector v;
        	in >> v[0] >> v[1] >> v[2];
        	mtls[mtlname].Ks = v;
        } else {
            // Unknown operator, ignore till next line
            read_to_eol(in);
            //ls_warning("MTL Operator %s unknown.\n", op.c_str());
        }
    }
}


void Model::draw(JRenderer & r, const Matrix & Mmodel, const Matrix & Mnormal)
{
    Vector vtx;
    Vector col(1,1,1);
    Vector ambient=0.25*Vector(.97,.83,.74);

    //r.setCullMode(JR_CULLMODE_NO_CULLING);
    r.pushMatrix();
    r.multMatrix(Mmodel);
    for (list<Group>::iterator grp = grps.begin(); grp != grps.end(); grp++) {
        vector<Face> &f=grp->f;
        if (grp->mtl.use_tex) {
    		r.enableTexturing();
            r.setTexture(grp->mtl.tex);
            r.setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
        } else {
    		r.disableTexturing();
        	r.setVertexMode(JR_VERTEXMODE_GOURAUD);
        }
        r.begin(JR_DRAWMODE_TRIANGLES);
        r.setAlpha(1.0);
        for (int i=0; i<f.size(); i++) {
            for (int j=0; j<3; j++) {
                vtx = v[f[i].v[j]];

                //vtx = Mmodel * vtx;
                col = grp->mtl.Kd;

                if (n.size()) {
                    Vector norm = Mnormal * n[f[i].n[j]];
                    Vector to_sun(-1,1,0);
                    to_sun.normalize();
                    float light = to_sun * norm;
                    if (light < 0) light = 0;
                    if (light > 1) light = 1;
                    //light = 1.0;
                    col *= light;
                }
                
                for(int i=0;i<3;++i)
                	col[i] += grp->mtl.Ka[i] * ambient[i];

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
    }
    r.popMatrix();
    r.disableTexturing();
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
