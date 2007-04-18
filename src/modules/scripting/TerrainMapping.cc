// ---------------------------------------------------------------
// |  TerrainMapping                                             |
// ---------------------------------------------------------------


#include <interfaces/ITerrain.h>
#include "mappings.h"

namespace {
	
	struct TerrainMapping : public TemplatedObjectMapping<ITerrain> {
		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject * self = proto(state);
			IoState_registerProtoWithFunc_(state, self, proto);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("Terrain"), self);
			retarget(self, ptr(game->getTerrain()));
		}
	
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
				{"heightAt", heightAt},
				{"normalAt", normalAt},
				{"lineIntersection", intersect},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "Terrain"));
			IoObject_setDataPointer_(self, 0);
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
		static IoObject *
		heightAt(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Terrain.heightAt")
			IOASSERT(IoMessage_argCount(m) == 2,"Expected two arguments")
			float x = unwrapObject<float>(IoMessage_locals_valueArgAt_(m, locals, 0));
			float y = unwrapObject<float>(IoMessage_locals_valueArgAt_(m, locals, 1));
			float height = getObject(self)->getHeightAt(x,y);
			return wrapObject(height, IOSTATE);
		}

		static IoObject *
		normalAt(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Terrain.normalAt")
			IOASSERT(IoMessage_argCount(m) == 2,"Expected two arguments")
			float x = unwrapObject<float>(IoMessage_locals_valueArgAt_(m, locals, 0));
			float y = unwrapObject<float>(IoMessage_locals_valueArgAt_(m, locals, 1));
			Vector normal;
			getObject(self)->getHeightAt(x,y, &normal);
			return wrapObject(normal, IOSTATE);
		}
		
		static IoObject *
		intersect(IoObject *self, IoObject *locals, IoMessage *m) {
			BEGIN_FUNC("Terrain.lineIntersection")
			IOASSERT(IoMessage_argCount(m) == 2,"Expected two arguments")
			Vector a = unwrapObject<Vector>(IoMessage_locals_valueArgAt_(m, locals, 0));
			Vector b = unwrapObject<Vector>(IoMessage_locals_valueArgAt_(m, locals, 1));
			Vector x;
			if (getObject(self)->lineCollides(a,b,&x)) {
			    return wrapObject(x, IOSTATE);
			} else {			        
			    return IONIL(self);
			}
		}
		
	};
	
} // namespace

template<>
void addMapping<ITerrain>(Ptr<IGame> game, IoState *state) {
	TerrainMapping::addMapping(game,state);
}

template<>
Ptr<ITerrain> unwrapObject<Ptr<ITerrain> >(IoObject * self) {
	return (ITerrain*)IoObject_dataPointer(self);
}

