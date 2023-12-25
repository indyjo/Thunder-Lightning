#ifdef HAVE_IO

// ---------------------------------------------------------------
// |  NMatrixMapping                                               |
// ---------------------------------------------------------------


#include <modules/math/NMatrix.h>
#include <debug.h>
#include "mappings.h"

namespace {
	
	typedef NMatrix<float> Mat;
	
	#define ISMAT(x) IoObject_hasCloneFunc_((x), rawClone)
	
	struct NMatrixMapping {
        static const char *const id;
        
	    static Mat* getObject(IoObject *o)
        { return reinterpret_cast<Mat*>(IoObject_dataPointer(o)); }
        
	    static void* rawClone(void *self_void) 
	    {
	        IoObject *self = (IoObject*) self_void;
		    IoObject *child = IoObject_rawClonePrimitive(self);
		    IoObject_setDataPointer_(child, new Mat(*getObject(self)));
			*getObject(child) = *getObject(self);
		    return child;
	    }
	    
		static IoObject * create(const Mat & mat, IoState *state) 
		{
			IoObject *child = IOCLONE(
				IoState_protoWithId_(state, id));
			(*getObject(child)) = mat;
			return child;
		}
		
	    static void mark(IoObject * self) {
	    }

	    static void free(IoObject * self) {
			ls_message("Freeing...");
			ls_message(" matrix at %p ", getObject(self));
			ls_message(" rows: %d cols: %d, ", getObject(self)->nrows(), getObject(self)->ncols());
		    delete getObject(self);
			ls_message("done\n");
	    }

	    static IoTag *tag(void * state, const char * name) {
	        IoTag *tag = IoTag_newWithName_(name);
	        tag->state = state;
	        tag->cloneFunc = rawClone;
	        tag->markFunc  = (IoTagMarkFunc *)mark;
	        tag->freeFunc  = (IoTagFreeFunc *)free;
	        //tag->writeToStoreFunc  = (TagWriteToStoreFunc *)IoFile_writeToStore_;
	        //tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoFile_readFromStore_;
	        return tag;
	    }

		static void addMapping(Ptr<IGame> game, IoState * state) {
			IoObject * self = proto(state);
			IoState_registerProtoWithId_(state, self, id);
			IoObject_setSlot_to_(state->lobby, IOSYMBOL("NMatrix"), self);
		}
	
		static IoObject *proto(void *state) {
			IoMethodTable methodTable[] = {
			    {"rows", getRows},
			    {"columns", getCols},
			    {"dim", redim},
				{"at", getValueAt},
				{"atSet", setValueAt},
				{"matMult", matMult},
				{"transposeInPlace", transpose},
				{"set", set},
				{"dot", dot},
				{"+", plus},
				{"-", minus},
				{"scaleInPlace", scaleInPlace},
				{"cross", cross},
				{NULL, NULL}
			};
			IoObject *self = IoObject_new(state);
			IoObject_tag_(self, tag(state, "NMatrix"));
			IoObject_setDataPointer_(self, new Mat(1,1));
			IoObject_addMethodTable_(self, methodTable);
			return self;
		}
		
        static IoObject *getRows(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.rows")
			return IONUMBER(getObject(self)->nrows());
		}
        static IoObject *getCols(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.colums")
			return IONUMBER(getObject(self)->ncols());
		}
		
        static IoObject *redim(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.dim")
            IOASSERT(IoMessage_argCount(m) > 0, "Expected at least one argument")
            IOASSERT(IoMessage_argCount(m) <= 2, "Expected at most two arguments")
            int rows=IoMessage_locals_intArgAt_(m,locals,0);
            int cols=1;
            if (IoMessage_argCount(m) == 2) {
                cols = IoMessage_locals_intArgAt_(m,locals,1);
            }
			getObject(self)->redim(rows,cols);
			return self;
		}
		
        static IoObject *getValueAt(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.at")
            int r=0, c=0;
            if (IoMessage_argCount(m) >= 1) {
                r = IoMessage_locals_intArgAt_(m,locals,0);
                if (IoMessage_argCount(m) >= 2) {
                    c = IoMessage_locals_intArgAt_(m,locals,1);
                }
            }
			return IONUMBER((*getObject(self))(r,c));
		}

        static IoObject *setValueAt(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.atSet")
            IOASSERT(IoMessage_argCount(m) > 0, "Expected at least one argument")
            IOASSERT(IoMessage_argCount(m) <= 3, "Expected at most three arguments")
            int r=0, c=0;
            double val;
            if (IoMessage_argCount(m) == 3) {
                r = IoMessage_locals_intArgAt_(m,locals,0);
                c = IoMessage_locals_intArgAt_(m,locals,1);
                val = IoMessage_locals_doubleArgAt_(m,locals,2);
            } else if (IoMessage_argCount(m) == 2) {
                r = IoMessage_locals_intArgAt_(m,locals,0);
                val = IoMessage_locals_doubleArgAt_(m,locals,1);
            } else {
                val = IoMessage_locals_doubleArgAt_(m,locals,0);
            }
			(*getObject(self))(r,c) = val;
			return self;
		}

        static IoObject *matMult(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.matMult")
            IOASSERT(IoMessage_argCount(m) == 1, "Expected exactly one argument")
            
            IoObject *other_obj = IoMessage_locals_valueArgAt_(m,locals,0);
            IOASSERT(ISMAT(other_obj), "Argument must be of NMatrix type")
            
            Mat *other = getObject(other_obj);
            Mat result = (*getObject(self)) * (*other);
            
            return create(result, IOSTATE);
		}
		
        VOID_FUNC(transpose);
		
        static IoObject *set(IoObject *self, IoObject*locals, IoObject*m) {
    	    BEGIN_FUNC("NMatrix.set")
            IOASSERT(IoMessage_argCount(m) == 1, "Expected exactly one argument")
            
            IoObject *other_obj = IoMessage_locals_valueArgAt_(m,locals,0);
            IOASSERT(ISMAT(other_obj), "Argument must be of NMatrix type")
            
            *getObject(self) = *getObject(other_obj);
            return self;
        }

        static IoObject *dot(IoObject *self, IoObject*locals, IoObject*m) {
    	    BEGIN_FUNC("NMatrix.dot")
            IOASSERT(IoMessage_argCount(m) == 1, "Expected exactly one argument")
            
            IoObject *other_obj = IoMessage_locals_valueArgAt_(m,locals,0);
            IOASSERT(ISMAT(other_obj), "Argument must be of NMatrix type")
            
            return IONUMBER(getObject(self)->inner(*getObject(other_obj)));
        }

        static IoObject *plus(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.plus")
            IOASSERT(IoMessage_argCount(m) == 1, "Expected exactly one argument")
            
            IoObject *other_obj = IoMessage_locals_valueArgAt_(m,locals,0);
            IOASSERT(ISMAT(other_obj), "Argument must be of NMatrix type")
            
            Mat *other = getObject(other_obj);
            Mat result = (*getObject(self)) + (*other);
            
            return create(result, IOSTATE);
		}

        static IoObject *minus(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.minus")
            IOASSERT(IoMessage_argCount(m) == 1, "Expected exactly one argument")
            
            IoObject *other_obj = IoMessage_locals_valueArgAt_(m,locals,0);
            IOASSERT(ISMAT(other_obj), "Argument must be of NMatrix type")
            
            Mat *other = getObject(other_obj);
            Mat result = (*getObject(self)) - (*other);
            
            return create(result, IOSTATE);
		}

        static IoObject *scaleInPlace(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.scaleInPlace")
            IOASSERT(IoMessage_argCount(m) == 1, "Expected exactly one argument")
            
            *getObject(self) *= IoMessage_locals_doubleArgAt_(m,locals,0);
            
            return self;
		}

        static IoObject *cross(IoObject *self, IoObject*locals, IoObject*m) {
			BEGIN_FUNC("NMatrix.cross")
            IOASSERT(IoMessage_argCount(m) == 1, "Expected exactly one argument")
            
            IoObject *other_obj = IoMessage_locals_valueArgAt_(m,locals,0);
            IOASSERT(ISMAT(other_obj), "Argument must be of NMatrix type")
            
            Mat *other = getObject(other_obj);
            Mat result = (*getObject(self)) % (*other);
            
            return create(result, IOSTATE);
		}
    };
    
    const char *const NMatrixMapping::id = "NMatrix";
}

template<>
void addMapping<NMatrix<float> >(Ptr<IGame> game, IoState *state) {
	NMatrixMapping::addMapping(game,state);
}

template<>
IoObject * 
wrapObject<NMatrix<float>& >(NMatrix<float>&  mat, IoState * state) {
	return NMatrixMapping::create(mat, state);
}

template<>
NMatrix<float>& unwrapObject(IoObject * self) {
	return *(NMatrix<float>*)IoObject_dataPointer(self);
}


#endif // HAVE_IO
