#include <cstdarg>
#include "TypedObject.h"

const Type::List Type::noparents={0};
const char *noname="<unnamed>";

Type::Type(const char *name, ...)
:	name(name)
{
	int count=1;
	const Type *arg;
	
	va_list args;
	va_start(args, name);
	
	while (va_arg(args,const Type*))
		count++;
		
	if (count==1) {
		parents=&noparents;
		return;
	}
		
	const Type **list = new const Type* [count];
	
	va_start(args,name);
	for(int i=0;i<count;++i)
		list[i]=va_arg(args, const Type*);
	
	parents=list;
}

bool Type::isA(const Type & t) const {
	if (&t==this) return true;
	for(List *p=parents; *p!=0; ++p)
		if ((*p)->isA(t))
			return true;
	return false;
}
