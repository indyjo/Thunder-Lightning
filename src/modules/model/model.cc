#include <fstream>
#include <cctype>
#include <cstdio>
//#include <modules/actors/fx/DebugObject.h>
#include "model.h"

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
		

//Model::Model() { }
//Model::Model(const Model &) { };
Model::Model(TextureManager & texman, const string & filename)
{
    parseObjFile(texman, filename);
}
Model::~Model() { }


void Model::parseObjFile(TextureManager & texman, const string & filename)
{
    string dir;
    map<std::string, Material> materials;
    
    Ptr<MeshData> meshdata = new MeshData;
    
    string::size_type n = filename.rfind('/');
    if ( n == string::npos ) dir = "./";
    else dir = filename.substr(0, n+1);
    ifstream in(filename.c_str());

    Ptr<Object> current_object = new Object(meshdata);
    objects.push_back(current_object);
    
    Ptr<Group> current_group = new Group();
    current_object->groups.push_back(current_group);
    
    string op;
    while (in >> op) {
        //ls_message("[Model: Operator %s]\n", op.c_str());

        if (op == "v") {                // Vertice
            Vector vec;
            in >> vec[0] >> vec[1] >> vec[2];
            //ls_message("[  vec: %f %f %f]\n", vec[0], vec[1], vec[2]);
            meshdata->vertices.push_back(vec);
        } else if ( op == "vn") {       // Normal
            Vector vec;
            in >> vec[0] >> vec[1] >> vec[2];
            //ls_message("[  vn: %f %f %f]\n", vec[0], vec[1], vec[2]);
            vec.normalize();
            meshdata->normals.push_back(vec);
        } else if ( op == "vt") {       // UV(W)
            Vector vec;
            in >> vec[0] >> vec[1];
            char c;
            do in.get(c); while (c==' ' || c=='\t');
            if (c=='\r' ||  c=='\n') {
            	vec[2]=0;
            } else if (isdigit(c) || c=='+' || c=='-') {
            	in.unget();
            	in >> vec[2];
            }
            //ls_message("[  vt: %f %f %f]\n", vec[0], vec[1], vec[2]);
            meshdata->texcoords.push_back(vec);
        } else if ( op == "mtllib" ) {  // Load materials
            string mtllib;
            in >> mtllib;
            parseMtlFile(texman, dir+"/"+mtllib, materials);
        } else if ( op == "usemtl" ) {  // Set the current group to use the given material
            string mtlname;
            in >> mtlname;
            //ls_message("[  usemtl: %s]\n", mtlname.c_str());
            map<string, Material>::iterator i = materials.find(mtlname);
            if (i == materials.end()) {
                ls_error("Error: could not resolve material %s\n", mtlname.c_str());
            } else {
                current_group->mtl = i->second;
            }
        } else if ( op == "f" ) {       // Face
            Face f;
        	char line[256];
        	char c;
        	
        	in.getline(line, 256);
        	//ls_message("line: %s\n",line);
        	for(char * tok = strtok(line, " ");
        		tok != NULL;
        		tok = strtok(NULL, " "))
        	{
        		//ls_message("tok: %s\n",tok);
            	int vv=0,tt=0,nn=0;
            	if (3==sscanf(tok, "%d/%d/%d", &vv, &tt, &nn)) {
            	} else if(2==sscanf(tok, "%d//%d", &vv, &nn)) {
            		tt = 0;
            	} else if (1==sscanf(tok, "%d", &vv)) {
            		nn = tt = 0;
            	} else {
            		ls_error("Couldn't parse %s\n", tok);
            	}
            	
            	Corner corner = { vv-1, nn-1, tt-1 };
            	f.push_back(corner);
        	}
        	
        	current_group->faces.push_back(f);
        } else if ( op == "g" ) {
        	string groupname;
        	in >> groupname;
        	//ls_message("Group: %s\n", groupname.c_str());
        	current_group = new Group(groupname);
        	current_object->groups.push_back(current_group);
        } else if ( op == "o" ) {
        	string objectname;
        	in >> objectname;
        	current_object = new Object(meshdata, objectname);
        	objects.push_back(current_object);
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


void Model::parseMtlFile(TextureManager & texman, const string & filename, map<string, Material> & mtls)
{
    string dir;
    string::size_type n = filename.rfind('/');
    if ( n == string::npos ) dir = "./";
    else dir = filename.substr(0, n+1);
    
    ifstream in(filename.c_str());
    if (!in) {
        ls_error("Couldn't open material file %s\n", filename.c_str());
        return;
    }
    
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
            mtls[mtlname].w = mtls[mtlname].tex->getWidth();
            mtls[mtlname].h = mtls[mtlname].tex->getHeight();
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
        } else if ( op == "Ns" ) {
        	float shininess;
        	in >> shininess;
        	mtls[mtlname].Ns = shininess;
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

    r.pushMatrix();
    r.multMatrix(Mmodel);
    
    for (int i=0; i<objects.size(); ++i) {
        objects[i]->draw(r);
    }
    
    r.popMatrix();
    r.disableTexturing();
}

Ptr<Model::Object> Model::getObject(const std::string & name) {
    for (int i=0; i<objects.size(); ++i) {
        if (objects[i]->name == name)
            return objects[i];
    }
    return 0;
}

void Model::setCullmode(jrcullmode_t cullmode) {
    typedef std::vector<Ptr<Object> > Objects;
    typedef Objects::iterator Iter;
    
    for(Iter i=objects.begin(); i!= objects.end(); ++i) {
        (*i)->setCullmode(cullmode);
    }
}

// HACK
// MSVC has issues with nested classes, so we typedef around that
typedef Model::Object MObject;
void MObject::draw(JRenderer & r)
{
    r.setAmbientColor(0.25*Vector(.97,.83,.74));
    Ptr<JMaterial> jmat = r.createMaterial();
    
    // Todo: proper lighting

    for (int i=0; i<groups.size(); ++i) {
        Ptr<Group> grp = groups[i];
        
        r.setCullMode(grp->cullmode);
        
	    r.setColor(grp->mtl.Kd);
	    jmat->setDiffuse(grp->mtl.Kd);
	    jmat->setAmbient(grp->mtl.Ka);
	    jmat->setSpecular(grp->mtl.Ks);
	    jmat->setShininess(grp->mtl.Ns);
	    jmat->activate();

        if (grp->mtl.use_tex) {
    		r.enableTexturing();
            r.setTexture(grp->mtl.tex->getTxtid());
            r.setVertexMode(JR_VERTEXMODE_GOURAUD_TEXTURE);
        } else {
    		r.disableTexturing();
        	r.setVertexMode(JR_VERTEXMODE_GOURAUD);
        }
        
        r.setAlpha(1.0);
        for (int i=0; i<grp->faces.size(); i++) {
            r.begin(JR_DRAWMODE_TRIANGLE_FAN);
            for (int j=0; j<grp->faces[i].size(); j++) {
                const Corner & corner = grp->faces[i][j];
                Vector vtx = meshdata->vertices[corner.v];

                if (corner.t>=0) {
                    r.setUVW(Vector(meshdata->texcoords[corner.t][0],
                             meshdata->texcoords[corner.t][1],
                             0));
                }
                r.setNormal(meshdata->normals[corner.n]);
                r.vertex(vtx);
            }
            r.end();
        }
        /*
        r.setAlpha(1.0);
        r.disableLighting();
        for (int i=0; i<grp->faces.size(); i++) {
            r.begin(JR_DRAWMODE_LINES);
            for (int j=0; j<grp->faces[i].size(); j++) {
                const Corner & corner = grp->faces[i][j];
                Vector vtx = meshdata->vertices[corner.v];
                Vector vtx2 = vtx + 0.5*meshdata->normals[corner.n];
                r.setColor(Vector(1,0,1));
                r.vertex(vtx);
                r.setColor(Vector(1,1,1));
                r.vertex(vtx2);
            }
            r.end();
        }
        r.enableLighting();
        */
    }
}

void MObject::setCullmode(jrcullmode_t cullmode) {
    typedef std::vector<Ptr<Group> > Groups;
    typedef Groups::iterator Iter;
    
    for(Iter i=groups.begin(); i!= groups.end(); ++i) {
        (*i)->cullmode = cullmode;
    }
}

