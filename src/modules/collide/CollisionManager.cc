#include <string>
#include <typeinfo>
#include <fstream>
#include <cstdio>
#include <modules/math/Interval.h>
#include <modules/actors/fx/DebugObject.h>
#include <game.h>

#include "BoundingGeometry.h"
#include "Contact.h"
#include "GeometryInstance.h"
#include "PossibleContact.h"
#include "Primitive.h"

#include "CollisionManager.h"

//#define DEBUG_MESSAGES

#ifdef DEBUG_MESSAGES
    #define debug_msg(...) ls_message(__VA_ARGS__)
#else
    #define debug_msg(...)
#endif


using namespace std;

typedef XTransform<Interval> ITransform;
typedef XVector<3,Interval> IVector;
typedef XMatrix<3,Interval> IMatrix3;


namespace Collide {

CollisionManager::CollisionManager() {
	ls_message("Initializing CollisionManager... ");
	ls_message("done.\n");
}

CollisionManager::~CollisionManager() {
	ls_message("Deleting CollisionManager %p with %d refs\n", this, getRefs());
	Object::backtrace();
	ls_message("Got %d geometry instances to delete.\n", geom_instances.size());
	for (GeomIter i=geom_instances.begin(); i!= geom_instances.end(); ++i) {
		ls_message("deleting geometry instance %p.\n", i->second);
		delete i->second;
		ls_message("done\n");
	}
	ls_message("Now cleaning up the rest.\n");
}

Ptr<BoundingGeometry>
CollisionManager::queryGeometry(const std::string & name) {
    typedef std::map<std::string, Ptr<BoundingGeometry> > Map;
    typedef Map::iterator Iter;
    Iter i = bounding_geometries.find(name);
    if (i == bounding_geometries.end()) {
        Ptr<BoundingGeometry> bg = new BoundingGeometry();
        ifstream in(name.c_str());
        if (!in) {
            ls_error("CollisionManager: Error opening\n  %s\n",
                name.c_str());
            return 0;
        }
        in >> *bg;
        if (!in) {
            ls_error("CollisionManager: Error reading\n  %s\n",
                name.c_str());
            return 0;
        }
        bounding_geometries.insert(std::make_pair(name, bg));
        return bg;
    }

    return i->second;
}

void CollisionManager::add(Ptr<Collidable> c) {
	GeometryInstance *instance = new GeometryInstance(c);
    geom_instances.insert(make_pair(c, instance));
    //ls_message("Added geometry instance %p\n", instance);
}

void CollisionManager::remove(Ptr<Collidable> c) {
    //ls_message("Removed geometry instance %p\n", geom_instances[c]);
    delete geom_instances[c];
    geom_instances.erase(c);
    sweep_n_prune.remove(c);
}

namespace {
void visualize_geometry(Ptr<IGame> game, const BoundingNode * node,
                        GeometryInstance *instance)
{
    switch(node->type) {
    case (BoundingNode::NONE): break;
    case (BoundingNode::NEWDOMAIN): break;
    case (BoundingNode::TRANSFORM): break;
    case (BoundingNode::INNER):
        visualize_geometry(game, node->data.inner.children[0], instance);
        visualize_geometry(game, node->data.inner.children[1], instance);
        break;
    case (BoundingNode::LEAF):
        for(int i=0; i<node->data.leaf.n_triangles; i++) {
            Transform & t = instance->transforms_0[0];
            for(int j=0; j<3; j++) {
                Vector p = t(node->data.leaf.vertices[3*i + j]);
                new DebugActor(game, p, "leaf", 0.01);
            }
        }
        break;
    case (BoundingNode::GATE): break;
    }
}
}

#define TIME_EPS 0.01

#define NUM_CONTACTS 1

void CollisionManager::run(Ptr<IGame> game, float delta_t) {
    float stop_time;
    Contact contact[NUM_CONTACTS];
    PossibleContact possible;
    Hints hints;

    int found_contacts;

    // get current transforms
    for(GeomIter i=geom_instances.begin(); i!=geom_instances.end(); i++) {
        Ptr<Collidable> collidable = i->first;
        GeometryInstance & instance = *i->second;

        collidable->integrate(0.0f, instance.transforms_0);
    }
    
    // compute destination transforms at delta_t
    for(GeomIter i=geom_instances.begin(); i!=geom_instances.end(); i++) {
        Ptr<Collidable> collidable = i->first;
        GeometryInstance & instance = *i->second;

        if (collidable->getRigid()) {
            collidable->integrate(delta_t, instance.transforms_1);
        } else {
            // non-rigid collidables are treated as static, which we enforce 
            // by just copying the first transform.
            int n = collidable->getBoundingGeometry()->getNumOfTransforms();
            for(int j=0; j<n; ++j) {
                instance.transforms_1[j] = instance.transforms_0[j];
            }
        }
    }

    while (delta_t > 0) {
        // First update the sweep'n'prune structure for finding test candidates
        for(GeomIter i=geom_instances.begin(); i!=geom_instances.end(); i++) {
            float min_x = i->second->transforms_0[0].vec()[0],
                max_x = i->second->transforms_1[0].vec()[0];
            if (min_x > max_x) swap(min_x, max_x);
            float r = i->first->getBoundingGeometry()->getBoundingRadius();

            sweep_n_prune.set(i->first, min_x - r, max_x + r);
        }

        ContactList possible_contacts;
        sweep_n_prune.findContacts(possible_contacts);

        // Now that we have our test candidates we feed them into the collision
        // queue
        while(!queue.empty()) queue.pop();
        possible.t0 = 0.0f;
        possible.t1 = delta_t;
        for(ContactIter i=possible_contacts.begin(); i!= possible_contacts.end(); i++) {
        	if (i->first->noCollideWith(i->second))
        		continue;
            if (!i->first->getRigid() && !i->second->getRigid())
                continue;
            possible.setPartner(0, geom_instances[i->first]);
            possible.setPartner(1, geom_instances[i->second]);
            queue.push(possible);
        }
        
        debug_msg("%d elements in queue.\n", (int)queue.size());

        // We have fed our queue and the main algorithm can start now.
        found_contacts = 0;
        stop_time = delta_t;
        bool found = false;
        int max_iters = 2048;
        int iter_count = 0;
        while(!queue.empty()) {
            if (iter_count++ == max_iters) {
                ls_warning("Aborting collsion test because of number of iterations.\n");
                break;
            }
            PossibleContact pc(queue.top());
            queue.pop();

            if (found_contacts>0 && pc.t0 > stop_time) break;

            if (pc.mustSubdivide()) {
                pc.subdivide(queue);
                continue;
            }

            bool collision = pc.collide(delta_t, hints);
            debug_msg("%s collision test %s <-> %s\n",
                collision?"positive":"negative",
                pc.partners[0].isTriangle()?"Triangle":(pc.partners[0].isNode()?"Node":"Sphere"),
                pc.partners[1].isTriangle()?"Triangle":(pc.partners[1].isNode()?"Node":"Sphere"));
            
            if (collision) {
                if (pc.shouldDivideTime(hints)) {
                    pc.divideTime(queue);
                    continue;
                }
                if (pc.canSubdivide()) {
                    pc.subdivide(queue);
                    continue;
                }

                if (pc.makeContact(contact[found_contacts], delta_t, hints)) {
                    Contact & c = contact[found_contacts++];
                    /*
                    char buf[256];
                    snprintf(buf, 256, "contact: %s / %s hints: %s",
                        pc.partners[0].isTriangle()?"Triangle":
                        (pc.partners[0].isBox()?"Box":"Sphere"),
                        pc.partners[1].isTriangle()?"Triangle":
                        (pc.partners[1].isBox()?"Box":"Sphere"),
                        hints.triangle_triangle.type==Hints::TriTri::VERTEX_TRIANGLE?"vertex-triangle":
                        (hints.triangle_triangle.type==Hints::TriTri::TRIANGLE_VERTEX?"triangle-vertex":
                        "edge-edge")
                        );
                    new DebugActor(game, c.p, buf);*/
                    if (pc.partners[0].isTriangle() && pc.partners[1].isTriangle()) {
                        for(int j=0; j<2; j++) {
                            ContactPartner & partner = pc.partners[j];
                            /*
                            for(int i=0; i<3; i++) {
                                Vector p = partner.instance->transforms_0[partner.transform]
                                    (partner.data.triangle[i]);
                                //snprintf(buf, 256, "Triangle %d point %d", j, i);
                                //new DebugActor(game, p, buf);
                            }*/
                            GeometryInstance *instance = partner.instance;
                            Ptr<BoundingGeometry> bounding = instance->collidable->getBoundingGeometry();
                            Transform & transform = instance->transforms_0[partner.transform];
                            //debug_msg("transform[%d]: quat(%f, %f, %f, %f)\n",
                            //    j, transform.quat().real(),transform.quat().imag()[0],transform.quat().imag()[1],transform.quat().imag()[2]);
                            //visualize_geometry(game, bounding->getRootNode(), instance);
                        }
                    }


                    stop_time = pc.t0;
                    /*
                    ls_warning("Found first contact #%d!\n", found_contacts);
                    ls_warning("t0:%f t1:%f\n", pc.t0, pc.t1);
                    ls_warning("p: %f %f %f\n",
                        c.p[0],c.p[1],c.p[2]);
                    ls_warning("n: %f %f %f\n",
                        c.n[0],c.n[1],c.n[2]);
                    ls_warning("v[0]: %f v[1]: %f\n", c.v[0], c.v[1]);
                    */
                    //Game::the_game->togglePauseMode();

                    if (found_contacts == NUM_CONTACTS)
                        break;

                } else {
                    ls_warning("Sorry, this ain't no first contact.\n");
                }
            }
        }
        
        debug_msg("Finished collision detection after %d iterations with %d contacts\n",
            iter_count-1, found_contacts);
        
        // If there was no collision we integrate up to delta_t and break
        if (found_contacts == 0) {
            for(GeomIter i=geom_instances.begin(); i!=geom_instances.end(); i++) {
                Ptr<Collidable> collidable = i->first;
                GeometryInstance & instance = *i->second;
                collidable->update(delta_t, instance.transforms_1);
            }
            break;
        }

        // So there was a collision. We have to interpolate up to the point where it
        // happened.
        float u = stop_time / delta_t;
        for(GeomIter i=geom_instances.begin(); i!=geom_instances.end(); i++) {
            Ptr<Collidable> collidable = i->first;
            GeometryInstance & instance = *i->second;
            int n = collidable->getBoundingGeometry()->getNumOfTransforms();
            for(int j=0; j<n; j++)
                instance.transforms_0[j] = interp(
                    u, instance.transforms_0[j], instance.transforms_1[j]);
            /*for(int c=0; c<found_contacts; c++) {
                if(collidable == contact[c].collidables[0]) {
                    instance.transforms_0[0] *= Transform(Quaternion(1,0,0,0), 0.05*contact[c].n);
                } else if (collidable == contact[c].collidables[1]) {
                    instance.transforms_0[0] *= Transform(Quaternion(1,0,0,0),-0.05*contact[c].n);
                }
            }*/
            collidable->update(delta_t, instance.transforms_0);
        }

        // now we calculate and apply the collision impulse (if possible) and
        // notify the objects of the collision
        for(int c = 0; c<found_contacts; c++) {
            contact[c].applyCollisionImpulse();
            for(int i=0; i<2; i++)
                contact[c].collidables[i]->integrate(stop_time,
                    geom_instances[contact[c].collidables[i]]->transforms_1);
            for(int i=0; i<2; i++)
                contact[c].collidables[i]->collide(contact[c]);
        }

        delta_t -= stop_time;
    } // while delta_t > 0
}

Ptr<Collidable> CollisionManager::lineQuery(
    const Vector &a,
    const Vector &b,
    Vector * out_x,
    Vector * out_normal,
    Ptr<Collidable> nocollide)
{
    BoundingBox box;
    box.pos =(a+b)/2;
    for(int i=0; i<3; ++i) box.dim[i] = std::abs(a[i]-b[i])/2;
    
    Ptr<Collidable> best_collidable;
    Vector best_x, best_normal;
    
    for(GeomIter i=geom_instances.begin(); i!=geom_instances.end(); ++i) {
        if (i->first == nocollide) {
            continue;
        }
        float r = i->first->getBoundingGeometry()->getBoundingRadius();
        Vector d = i->second->transforms_0[0].vec() - box.pos;
        
        // We try to sort out non-intersecting objects as fast as we can
        // by performing an AABB-AABB test. The second AABB wraps
        // the tested object's bounding sphere.
        for(int j=0; j<3; ++j) {
            if (std::abs(d[j]) > r + box.dim[j]) {
                continue;
            }
        }
        
        // If we have come so far, we might still sort out an object with a
        // separating axis test, i.e. a real AABB<->sphere test.
        float d_len = d.length();
        if (d_len > 1e-5) {
            d /= d_len;
            float dim_dot_abs_d =
                box.dim[0]*std::abs(d[0]) +
                box.dim[1]*std::abs(d[1]) +
                box.dim[2]*std::abs(d[2]);
            if (dim_dot_abs_d + r < d_len) {
                continue;
            }
        }
        
        // If we have come he we must perform a fine-grained intersection test
        Vector new_x;
        Vector new_normal;
        bool intersect = intersectLineNode(
            a,b,
            0, i->second->transforms_0[0],                  // xform_id, xform
            i->second,                                      // geom_instance
            i->first->getBoundingGeometry()->getRootNode(), // node
            &new_x, &new_normal);
            
        
        if (intersect && (!best_collidable || (new_x-best_x) * (b-a) < 0)) {
            best_collidable = i->first;
            best_x = new_x;
            best_normal = new_normal;
        }
    }
    
    if (best_collidable) {
        if (out_x) *out_x = best_x;
        if (out_normal) *out_normal = best_normal;
    }
    
    return best_collidable;
}

} // namespace Collide

