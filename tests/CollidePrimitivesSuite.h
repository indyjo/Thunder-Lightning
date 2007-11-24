#include <cxxtest/TestSuite.h>
#include <modules/collide/BoundingNode.h>
#include <modules/collide/Primitive.h>

class CollidePrimitivesSuite : public CxxTest::TestSuite 
{
public:
    void testIsPointInPrism( void )
    {
        using namespace Collide;
        
        Vector tri[] = {Vector(-1,0,-1), Vector(1,0,-1), Vector(1,0,1)};
        Vector normal(0,1,0);
        Vector p1(0.5,0,-0.5), p2(0.5,100,-0.5), p3(0.5,-30,-0.5);
        Vector q1(2,0,0), q2(0,0,-2), q3(-0.5,0,0.5);
        TS_ASSERT( isPointInPrism(p1, tri, normal) );
        TS_ASSERT( isPointInPrism(p2, tri, normal) );
        TS_ASSERT( isPointInPrism(p3, tri, normal) );
        TS_ASSERT( ! isPointInPrism(q1, tri, normal) );
        TS_ASSERT( ! isPointInPrism(q2, tri, normal) );
        TS_ASSERT( ! isPointInPrism(q3, tri, normal) );
    }
    
    void testIntersectLineTriangle( void )
    {
        using namespace Collide;
        
        Vector tri[] = {Vector(-1,0,-1), Vector(1,0,-1), Vector(1,0,1)};
        
        TS_ASSERT( intersectLineTriangle(Vector(0.5,1,-.5),Vector(0.5,-1,-.5),
                                         tri, Transform::identity()));
        TS_ASSERT( intersectLineTriangle(Vector(0.0001,1,-0.0001),Vector(0.0001,-1,-0.0001),
                                         tri, Transform::identity()));
        TS_ASSERT(!intersectLineTriangle(Vector(-0.0001,1,0.0001),Vector(-0.0001,-1,0.0001),
                                         tri, Transform::identity()));
        
        TS_ASSERT(!intersectLineTriangle(Vector(-0.5,1,.5),Vector(-0.5,-1,.5),
                                         tri, Transform::identity()));
        TS_ASSERT(!intersectLineTriangle(Vector(1.5,1,0),Vector(1.5,-1,0),
                                         tri, Transform::identity()));
        TS_ASSERT(!intersectLineTriangle(Vector(0,1,-1.5),Vector(0,-1,-1.5),
                                         tri, Transform::identity()));
                                         
        Vector x, normal;
        intersectLineTriangle(Vector(0.5,100,-.5),Vector(0.5,-1,-.5),
                                     tri, Transform::identity(),
                                     &x, &normal);
        
        TS_ASSERT_LESS_THAN( (x-Vector(0.5,0,-.5)).length(), 0.0001 );
        TS_ASSERT_LESS_THAN( (normal-Vector(0,-1,0)).length(), 0.0001 );

        intersectLineTriangle(Vector(0.5,0.1,-.5),Vector(0.5,-0.1,-.5),
                                     tri, Transform::identity(),
                                     &x, &normal);
        
        TS_ASSERT_LESS_THAN( (x-Vector(0.5,0,-.5)).length(), 0.0001 );
        TS_ASSERT_LESS_THAN( (normal-Vector(0,-1,0)).length(), 0.0001 );
    }

    void testIntersectLineTriangleUnderXform( void )
    {
        using namespace Collide;
        
        Vector tri[] = {Vector(-1,0,-1), Vector(1,0,-1), Vector(1,0,1)};
        Transform xforms[] = {
            Transform::identity(),
            Transform(Quaternion::Rotation(Vector(0.4, 3.5, -3.5).normalize(), 1.33), Vector(324,235,-34)),
            Transform(Quaternion::Rotation(Vector(23.2, 1.2, 8.2).normalize(), -1.03), Vector(23,-7688,321))
        };
        
        for (int i=0; i<3; ++i) {
            Transform xform = xforms[i];

            TS_ASSERT( intersectLineTriangle(xform(Vector(0.5,1,-.5)),xform(Vector(0.5,-1,-.5)),
                                             tri, xform));
            TS_ASSERT( intersectLineTriangle(xform(Vector(0.0001,1,-0.0001)),xform(Vector(0.0001,-1,-0.0001)),
                                             tri, xform));
            TS_ASSERT(!intersectLineTriangle(xform(Vector(-0.0001,1,0.0001)),xform(Vector(-0.0001,-1,0.0001)),
                                             tri, xform));
            
            TS_ASSERT(!intersectLineTriangle(xform(Vector(-0.5,1,.5)),xform(Vector(-0.5,-1,.5)),
                                             tri, xform));
            TS_ASSERT(!intersectLineTriangle(xform(Vector(1.5,1,0)),xform(Vector(1.5,-1,0)),
                                             tri, xform));
            TS_ASSERT(!intersectLineTriangle(xform(Vector(0,1,-1.5)),xform(Vector(0,-1,-1.5)),
                                             tri, xform));
                                             
            Vector x, normal;
            intersectLineTriangle(xform(Vector(0.5,100,-.5)),xform(Vector(0.5,-1,-.5)),
                                         tri, xform,
                                         &x, &normal);
            
            TS_ASSERT_LESS_THAN( (x-xform(Vector(0.5,0,-.5))).length(), 0.0001 );
            TS_ASSERT_LESS_THAN( (normal-xform.quat().rot(Vector(0,-1,0))).length(), 0.0001 );
        }
    }
        
    
    void testEarliestIntersectionLineTriangle( void )
    {
        using namespace Collide;
        
        Vector tri_0[] = {Vector(-1,0,-1), Vector(1,0,-1), Vector(1,0,1)};
        Vector tri_1[] = {Vector(-1,1,-1), Vector(1,1,-1), Vector(1,1,1)};
        Vector tri_2[] = {Vector(-1,2,-1), Vector(1,2,-1), Vector(1,2,1)};
        Vector a(0.5, 5, -0.5), b(0.5, -5, -0.5);
        
        bool intersect = false;
        Vector x, normal;
        TS_ASSERT( intersect = earliestIntersectionLineTriangle(
            intersect, a, b, tri_0, Transform::identity(), &x, &normal) );
        TS_ASSERT( intersect = earliestIntersectionLineTriangle(
            intersect, a, b, tri_1, Transform::identity(), &x, &normal) );
        TS_ASSERT( intersect = earliestIntersectionLineTriangle(
            intersect, a, b, tri_2, Transform::identity(), &x, &normal) );
        TS_ASSERT_LESS_THAN( (x-Vector(0.5,2,-0.5)).length(), 0.0001);
        TS_ASSERT_LESS_THAN( (normal-Vector(0,-1,0)).length(), 0.0001 );
        
        intersect = false;
        TS_ASSERT( intersect = earliestIntersectionLineTriangle(
            intersect, b, a, tri_0, Transform::identity(), &x, &normal) );
        TS_ASSERT( intersect = earliestIntersectionLineTriangle(
            intersect, b, a, tri_1, Transform::identity(), &x, &normal) );
        TS_ASSERT( intersect = earliestIntersectionLineTriangle(
            intersect, b, a, tri_2, Transform::identity(), &x, &normal) );
        TS_ASSERT_LESS_THAN( (x-Vector(0.5,0,-0.5)).length(), 0.0001 );
        TS_ASSERT_LESS_THAN( (normal-Vector(0,-1,0)).length(), 0.0001 );
    }
    
    void testIntersectLineNodeWithLeaf( void )
    {
        using namespace Collide;
        
        BoundingNode node;
        node.type = BoundingNode::LEAF;
        node.data.leaf.n_triangles = 2;
        node.data.leaf.vertices = new Vector[6];
        
        node.data.leaf.vertices[0] = Vector(-2,-1,-1);
        node.data.leaf.vertices[1] = Vector(0,1,-1);
        node.data.leaf.vertices[2] = Vector(0,1,2);
        node.data.leaf.vertices[3] = Vector(2,-1,-1);
        node.data.leaf.vertices[4] = Vector(0,1,-1);
        node.data.leaf.vertices[5] = Vector(0,1,2);
        
        node.box.pos = Vector(0,0,0.5);
        node.box.dim[0]=2;
        node.box.dim[1]=1;
        node.box.dim[2]=1.5;
        
        Vector x;
        TS_ASSERT( intersectLineNode(
            Vector(-10,0,0), Vector(10,0,0),
            0, Transform::identity(),
            0, // no geom instance
            &node,
            &x) );
        TS_ASSERT_LESS_THAN( (x-Vector(-1,0,0)).length(), 0.0001 );
        
        TS_ASSERT( intersectLineNode(
            Vector(10,0,0), Vector(-10,0,0),
            0, Transform::identity(),
            0, // no geom instance
            &node,
            &x) );
        TS_ASSERT_LESS_THAN( (x-Vector(1,0,0)).length(), 0.0001 );

        TS_ASSERT( intersectLineNode(
            Vector(-1.1,0,0), Vector(-0.9,0,0),
            0, Transform::identity(),
            0, // no geom instance
            &node,
            &x) );
        TS_ASSERT_LESS_THAN( (x-Vector(-1,0,0)).length(), 0.0001 );
    }
};

