#include "LoDTerrain.h"
#include "Config.h"

void LoDQuad::draw(JRenderer *renderer)
{
    renderer->setCullMode(JR_CULLMODE_CULL_POSITIVE);
    renderer->disableFog();
    float alpha[3];
#if ENABLE_FOG_LAYER
    for(int i=0; i<3; i++) {
        Vector v(
                vx[triangle[0].vertex[i]],
                vy[triangle[0].vertex[i]],
                vz[triangle[0].vertex[i]]);
        alpha[i] = environment->getFogStrengthAt(v);
    }
#endif
    drawRecursive(renderer,&triangle[0],
            vy[triangle[0].vertex[0]],
            vy[triangle[0].vertex[1]],
            vy[triangle[0].vertex[2]], alpha);
#if ENABLE_FOG_LAYER
    alpha[2] = alpha[0];
    alpha[0] = alpha[1];
    alpha[1] = alpha[2];
    alpha[2] = environment->getFogStrengthAt( Vector(
            vx[triangle[1].vertex[2]],
            vy[triangle[1].vertex[2]],
            vz[triangle[1].vertex[2]]));
#endif
    drawRecursive(renderer,&triangle[1],
            vy[triangle[1].vertex[0]],
            vy[triangle[1].vertex[1]],
            vy[triangle[1].vertex[2]], alpha);
    
    renderer->enableFog();
}

void LoDQuad::drawWire(JRenderer *renderer)
{
    renderer->setCullMode(JR_CULLMODE_NO_CULLING);
    BorderSet bdrs_left  = {true, true, true};
    BorderSet bdrs_right = {true, true, false};
    drawWireRecursive(renderer,&triangle[0], bdrs_left);
    drawWireRecursive(renderer,&triangle[1], bdrs_right);
}

void LoDQuad::drawRecursive(JRenderer *r, LoDTriangle *tri,
        float y0, float y1, float y2, float * alpha)
{
    if(tri->morph < 0.0 || tri->morph > 1.0) ls_error("\bmorph:%f\n", tri->morph);
    if(tri->flags & TFLAG_DONT_DRAW) return;
    
    int i;
    float y[3];
    y[0] = y0;
    y[1] = y1;
    y[2] = tri->morph * vy[tri->vertex[2]] + (1.0 - tri->morph) * y2;
    
    if (tri->flags & TFLAG_ENABLED) {
        // We make the backface culling test here because this almost impossible
        // during setup (without calculating morphed coords)
        int vtx = tri->vertex[0];
        Vector p0 = Vector(vx[vtx], y[0], vz[vtx]);
        vtx = tri->vertex[1];
        Vector p1 = Vector(vx[vtx], y[1], vz[vtx]);
        vtx = tri->vertex[2];
        Vector p2 = Vector(vx[vtx], y[2], vz[vtx]);
        Vector d1 = p1 - p0;
        Vector d2 = p2 - p0;
        Vector n = d2 % d1;
        Vector pos;
        pos = game->getCamera()->getLocation();
        if (n*(p2-pos) > 0) return;
    }
    
#if ENABLE_FOG_LAYER
    float min_fog = std::min(alpha[0], std::min(alpha[1], alpha[2]));
    if ((tri->flags & TFLAG_ENABLED) && min_fog >= 1) {
        drawFogTriangle(r, tri, y, alpha);
        return;
    }
#endif
            
    if (tri->flags & TFLAG_ENABLED) {
#if ENABLE_BIG_TEXTURE
        
        r->setZBufferFunc(JR_ZBFUNC_LEQUAL);
        r->enableSmoothShading();
        r->enableTexturing();
        r->setTexture(main_tex);
        
        r->begin(JR_DRAWMODE_TRIANGLES);
        for (i=0; i<3; i++) {
            Vector color(1,1,1);
            //if (tri->flags & TFLAG_DEBUG) color[1]=0;
            int vtx = tri->vertex[i];
            Vector v( vx[vtx], y[i], vz[vtx]);
            Vector uvw( tex_u[vtx], tex_v[vtx], 0);

            r->setColor(color);
            r->setUVW(uvw);
            r->vertex(v);
        }
        r->end();
#endif
#if ENABLE_NEIGHBOR_DEBUGGING
        ls_message("NEIGHBOR DEBUGGING.\n");
        for(int n=0; n<3; n++) {
            ls_message("neighbor[%d]=%p\n", n, tri->neighbor[n]);
            if (tri->neighbor[0]==tri->neighbor[1]
                    && tri->neighbor[1]==tri->neighbor[2]) {
                ls_warning("All neighbors are equal! %p\n", tri->neighbor[0]);
            }
            if (!tri->neighbor[n]) {
                ls_warning("No neighbor!\n");
                Vector p0 = Vector(
                        vx[tri->vertex[0]],
                        vy[tri->vertex[0]],
                        vz[tri->vertex[0]]);
                Vector p1 = Vector(
                        vx[tri->vertex[1]],
                        vy[tri->vertex[1]],
                        vz[tri->vertex[1]]);
                Vector p2 = Vector(
                        vx[tri->vertex[2]],
                        vy[tri->vertex[2]],
                        vz[tri->vertex[2]]);
                Vector m0 = (p0 + p1 + p2)/3 + Vector(0,1000,0);
                Vector m1;

                switch(n) {
                case 0:
                    m1 = (p0 + p2)/2;
                    break;
                case 1:
                    m1 = (p1 + p2)/2;
                    break;
                case 2:
                    m1 = (p0 + p1)/2;
                    break;
                }

                r->disableTexturing();
                r->begin(JR_DRAWMODE_CONNECTED_LINES);
                r->setColor(Vector(1,0,1));
                r->setAlpha(1);
                *r << m0 << m1;
                r->end();
            } else {
                Vector p0 = (Vector(vx[tri->vertex[0]], vy[tri->vertex[0]], vz[tri->vertex[0]])
                        + Vector(vx[tri->vertex[1]], vy[tri->vertex[1]], vz[tri->vertex[1]])
                        + Vector(vx[tri->vertex[2]], vy[tri->vertex[2]], vz[tri->vertex[2]]))
                        / 3;
                float *vx=this->vx, *vy=this->vy, *vz=this->vz;
                LoDTriangle * nb = tri->neighbor[n];
                if (nb < triangle || nb >= triangle+triangles) {
                    for(int i=0; i<4; i++) {
                        if (neighbor[i] &&
                            nb >= neighbor[i]->triangle &&
                            nb  < neighbor[i]->triangle+neighbor[i]->triangles) {
                            vx = neighbor[i]->vx;
                            vy = neighbor[i]->vy;
                            vz = neighbor[i]->vz;
                            break;
                        }
                    }
                }
                Vector p4 = (Vector(vx[nb->vertex[0]], vy[nb->vertex[0]], vz[nb->vertex[0]])
                        + Vector(vx[nb->vertex[1]], vy[nb->vertex[1]], vz[nb->vertex[1]])
                        + Vector(vx[nb->vertex[2]], vy[nb->vertex[2]], vz[nb->vertex[2]]))
                        / 3;
                ls_message("Neighbor %d is triangle at %p with midpoint at: ",
                        n, nb);
                {
                    Vector p0 = Vector(
                            vx[nb->vertex[0]],
                            vy[nb->vertex[0]],
                            vz[nb->vertex[0]]);
                    Vector p1 = Vector(
                            vx[nb->vertex[1]],
                            vy[nb->vertex[1]],
                            vz[nb->vertex[1]]);
                    Vector p2 = Vector(
                            vx[nb->vertex[2]],
                            vy[nb->vertex[2]],
                            vz[nb->vertex[2]]);
                    p0.dump();
                    p1.dump();
                    p2.dump();
                }
                
                p4.dump();
                Vector p2 = (p0 + p4)/2 +  Vector(0, (p4-p0).length()/2.0 ,0);
                Vector p1 = (p0 + p2)/2;
                Vector p3 = (p2 + p4)/2;

                switch(n) {
                case 0:
                    p3 += Vector(10,20,30);
                    break;
                case 1:
                    p3 += Vector(40,50,60);
                    break;
                case 2:
                    p3 += Vector(70,80,90);
                    break;
                }

                r->disableTexturing();
                r->begin(JR_DRAWMODE_CONNECTED_LINES);
                r->setColor(Vector(n==0,n==1,n==2));
                r->setAlpha(1);
                *r << p0 << p1 << p3 << p4;
                r->end();
            }
        }
#endif
#if ENABLE_SPHERE_DEBUGGING
#define PI 3.141593
#define SPHERE_SEGMENTS 24
        {
            Vector p = game->getCamera()->getLocation();
            float d = (tri->bs_center - p).length();
            float rad = tri->radius;
            float e = rad*rad/d;
            float r2 = rad - e*e/rad;
            Vector center = tri->bs_center + e/d * (p-tri->bs_center);
            Vector right = Vector(0,1,0) % (tri->bs_center-p);
            right.normalize();
            Vector up = (tri->bs_center-p) % right;
            up.normalize();
            
            r->disableTexturing();
            r->disableAlphaBlending();
            r->begin(JR_DRAWMODE_CONNECTED_LINES);
            r->setColor(Vector(1,0,1)); // simply ugly
            for(int u=0; u<SPHERE_SEGMENTS; u++) {
                float alpha = (float)u/SPHERE_SEGMENTS * 2*PI;
                Vector v = right * cos(alpha) + up*sin(alpha);
                r->vertex(r2*v + center);
            }
            r->end();
        }
#endif // ENABLE_SPHERE_DEBUGGING
#if ENABLE_DETAIL_TEX
        r->setTexture(detail_tex);
        r->setZBufferFunc(JR_ZBFUNC_EQUAL);
        r->enableAlphaBlending();
        r->begin(JR_DRAWMODE_TRIANGLES);
        for (int x = 2<<(DETAIL_LAYERS-1); x >= 1; x/=2) {
            for (i=0; i<3; i++) {
                v.p.x = vx[ tri->vertex[i] ];
                v.p.y = vy[ tri->vertex[i] ];
                v.p.z = vz[ tri->vertex[i] ];

                v.txt.x=v.p.x * DETAIL_SCALE / (float)x;
                v.txt.y=-v.p.z * DETAIL_SCALE / (float)x;

                r->setAlpha(0.5);
                r->addVertex(&v);
            }
        }
        r->end();
        r->disableAlphaBlending();
        r->setZBufferFunc(JR_ZBFUNC_LEQUAL);
#endif
#if ENABLE_TEXTURING
        if ((tri->flags & TFLAG_HAS_CHILDREN)==0) {
            drawTexturedTriangle(r, tri, y);
        }
#endif
#if ENABLE_LIGHTMAP
        r->enableTexturing();
        r->setTexture(lightmap);
        r->setZBufferFunc(JR_ZBFUNC_LEQUAL);
        r->enableAlphaBlending();
        r->setBlendMode(JR_BLENDMODE_MULTIPLICATIVE);
        r->begin(JR_DRAWMODE_TRIANGLES);
        for (i=0; i<3; i++) {
            Vector color(1,1,1);
            int vtx = tri->vertex[i];
            Vector v( vx[vtx], y[i], vz[vtx]);
            Vector uvw( tex_u[vtx], tex_v[vtx], 0);
            r->setColor(color);
            r->setAlpha(1);
            r->setUVW(uvw);
            r->vertex(v);
        }
        r->end();
        r->setBlendMode(JR_BLENDMODE_BLEND);
#endif
#if ENABLE_FOG_LAYER
        drawFogTriangle(r,tri,y,alpha);
#endif
    } else {
        float ym = (y[0] + y[1]) / 2.0;
        float a[3];
#if ENABLE_FOG_LAYER
        Vector v(
                vx[tri->child[0]->vertex[2]],
                vy[tri->child[0]->vertex[2]],
                vz[tri->child[0]->vertex[2]]);
        float fog = environment->getFogStrengthAt(v);
        a[2]=(alpha[0]+alpha[1])/2 * (1-tri->child[0]->morph)
                + fog * tri->child[0]->morph;
        a[0]=alpha[2];
        a[1]=alpha[0];
#endif
        // left child
        drawRecursive(r, tri->child[0], y[2], y[0], ym, a);
#if ENABLE_FOG_LAYER
        a[0]=alpha[1];
        a[1]=alpha[2];
#endif
        // right child
        drawRecursive(r, tri->child[1], y[1], y[2], ym, a);
    }
}

void LoDQuad::drawWireBorder(JRenderer *r, LoDTriangle *tri, int i0, int i1)
{
    jvertex_coltxt v;
    
    v.col.r = 0.0;
    v.col.g = 0.0;
    v.col.b = 0.0;
    
    v.p.x = vx[ tri->vertex[i0] ];
    v.p.y = vy[ tri->vertex[i0] ];
    v.p.z = vz[ tri->vertex[i0] ];
    r->addVertex(&v);

    v.p.x = vx[ tri->vertex[i1] ];
    v.p.y = vy[ tri->vertex[i1] ];
    v.p.z = vz[ tri->vertex[i1] ];
    r->addVertex(&v);
}
    
        
void LoDQuad::drawWireRecursive(JRenderer *r, LoDTriangle *tri, BorderSet bdrs)
{
    int i;
    
    if (tri->flags & TFLAG_ENABLED) {
        r->setVertexMode(JR_VERTEXMODE_GOURAUD);
        
        r->begin(JR_DRAWMODE_LINES);
        
        if (bdrs.left) drawWireBorder(r,tri,0,2);
        if (bdrs.right) drawWireBorder(r,tri,1,2);
        if (bdrs.bottom) drawWireBorder(r,tri,0,1);
        
        r->end();
    } else {
        BorderSet child_bdrs[2];
        
        child_bdrs[0].left = true;
        child_bdrs[0].right = bdrs.bottom;
        child_bdrs[0].bottom = bdrs.left;
        
        child_bdrs[1].right = false;
        child_bdrs[1].left = bdrs.bottom;
        child_bdrs[1].bottom = bdrs.right;
        
        drawWireRecursive(r, tri->child[0], child_bdrs[0]);
        drawWireRecursive(r, tri->child[1], child_bdrs[1]);
    }
}


void LoDQuad::drawTexturedTriangle(JRenderer * r, LoDTriangle * tri, float * y)
{
    Vector uvw[4];
    for(int i=0; i<3; i++) {
        uvw[i] = Vector(tex_u[tri->vertex[i]], tex_v[tri->vertex[i]], 0);
    }
    uvw[3][2]=0;
    if (uvw[2][0] == uvw[0][0]) uvw[3][0] = uvw[1][0]; else uvw[3][0] = uvw[0][0];
    if (uvw[2][1] == uvw[0][1]) uvw[3][1] = uvw[1][1]; else uvw[3][1] = uvw[0][1];
    
    Vector v[3];
    for(int i=0; i<3; i++) {
        v[i] = Vector(vx[tri->vertex[i]], y[i], vz[tri->vertex[i]]);
    }
    
    int tex_indices[4];
    int tex_size = texmap->getWidth()-1;
    for (int i=0; i<4; i++) {
        int u = (int) ( uvw[i][0] * tex_size );
        int v = (int) ( uvw[i][1] * tex_size );
        u = std::min(u, tex_size-1);
        v = std::min(v, tex_size-1);
        tex_indices[i] = texmap->pixelAt(u,v);
    }
    int max_index = std::max(
            std::max(tex_indices[0], tex_indices[1]),
            std::max(tex_indices[2], tex_indices[3]));
    int absolute_max = max_index;
    
    int corner_bits[4];
    {
        float u_min = std::min(uvw[0][0], std::min(uvw[1][0], uvw[2][0]));
        float v_min = std::min(uvw[0][1], std::min(uvw[1][1], uvw[2][1]));
        for(int i=0; i<4; i++) {
            if (uvw[i][0] == u_min) {
                if (uvw[i][1] == v_min) corner_bits[i] = 8;   // NORTHWEST
                else corner_bits[i] = 1;                    // SOUTHWEST
            } else {
                if (uvw[i][1] == v_min) corner_bits[i] = 4;   // NORTHEAST
                else corner_bits[i] = 2;                    // SOUTHEAST
            }
        }
        //for(int i=0; i<4; i++) uvw[i] =(uvw[i]-Vector(u_min,v_min,0))*512;
        for(int i=0; i<4; i++) uvw[i] =(uvw[i]-Vector(u_min,v_min,0))*256;
    }
    
    r->enableAlphaBlending();
    r->enableTexturing();
    
    while ( max_index != -1 ) {
        float alpha[3];
        {
            int tile_index = 0;
            for (int i=0; i<4; i++) {
                if (tex_indices[i] == max_index) tile_index |= corner_bits[i];
            }
            if (textures[max_index][tile_index]) {
                alpha[0]=alpha[1]=alpha[2]=tri->morph;
                r->setTexture( textures[max_index][tile_index] );
                r->setWrapMode( JR_TEXDIM_U, JR_WRAPMODE_CLAMP );
                r->setWrapMode( JR_TEXDIM_V, JR_WRAPMODE_CLAMP );
            } else {
                r->setTexture( textures[max_index][15] );
                r->setWrapMode( JR_TEXDIM_U, JR_WRAPMODE_REPEAT );
                r->setWrapMode( JR_TEXDIM_V, JR_WRAPMODE_REPEAT );
                for(int i=0; i<3; i++) {
                    alpha[i] = tri->morph *
                            (float) (tex_indices[i] == max_index);
                }
            }
        }
        r->begin(JR_DRAWMODE_TRIANGLES);
        for (int i=0; i<3; i++) {
            r->setColor(Vector(1,1,1));
            r->setUVW(uvw[i]);
            r->setAlpha(alpha[i]);
            r->vertex(v[i]);
        }
        r->end();
        
        int next_max = -1;
        for(int i=0; i<4; i++) {
            int tex_index = tex_indices[i];
            if (tex_index < max_index && tex_index > next_max)  {
                next_max = tex_index;
            }
        }
        max_index = next_max;
    }
    
    r->disableAlphaBlending();
}


void LoDQuad::drawFogTriangle(JRenderer * r, LoDTriangle * tri,
        float * y, float * alpha)
{
    float max_alpha = std::min(alpha[0], std::min(alpha[1],alpha[2]));
    //if (max_alpha < 0.05) return;
    
    Vector corner[3];
    for(int i=0; i<3; i++) {
        int vtx = tri->vertex[i];
        corner[i] = Vector(vx[vtx], y[i], vz[vtx]);
    }
    
    //r->disableFog();
    r->disableTexturing();
    r->enableSmoothShading();
    r->setZBufferFunc(JR_ZBFUNC_LEQUAL);
    r->enableAlphaBlending();
    r->begin(JR_DRAWMODE_TRIANGLES);
    r->setColor(environment->getFogColor());
    for (int i=0; i<3; i++) {
        
        r->setAlpha(alpha[i]);
        r->vertex(corner[i]);
    }
    r->end();
    r->disableAlphaBlending();
    //r->enableFog();
}
