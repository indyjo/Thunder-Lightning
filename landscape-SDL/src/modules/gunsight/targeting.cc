#include <cmath>
#include <vector>
#include <modules/math/SpecialMatrices.h>
#include <modules/jogi/JRenderer.h>
#include <modules/weaponsys/Targeter.h>
#include <TargetInfo.h>
#include <Faction.h>
#include "gunsight.h"

using namespace std;

struct TargetingModule : public GunsightModule {
	Ptr<IActor> self;
	Ptr<Targeter> targeter;
	
	TargetingModule(
		float width, float height,
		Ptr<IActor> self,
		Ptr<Targeter> targeter)
	:	GunsightModule("targeting", width, height),
		self(self),
		targeter(targeter)
	{
	}
	
	void draw(FlexibleGunsight & gunsight) {
		JRenderer *r = gunsight.getRenderer();
		UI::Surface surface = gunsight.getSurface();
		surface.translateOrigin(
			offset[0]+width/2,
			offset[1]+height/2);
			
		Ptr<ICamera> cam = gunsight.getCamera();
		float zfactor = 0.5*surface.getHeight() * cam->getFocus();
		float focus = cam->getFocus();
		float aspect = cam->getAspect();
		
		Matrix M = Matrix::Hom(cam->getOrientInv())
		           * TranslateMatrix<4>(-cam->getLocation());
		
		r->pushMatrix();
		r->multMatrix(surface.getMatrix());
		
		vector<Ptr<IActor> > targets;
		targeter->listTargets(targets);
		
		for(int i=0; i<targets.size(); ++i) {
			Vector rel = M*targets[i]->getLocation();
			
			if (rel[2] <= 0) continue;
			Vector p=(zfactor/rel[2])*Vector(rel[0], -rel[1], 0);
			float size = targets[i]->getTargetInfo() ?
						 targets[i]->getTargetInfo()->getTargetSize() :
						 5;
			size *= zfactor / rel[2];
			size = max(5.0f, size);
			
			r->setColor(Vector(0,1,0));
			if (self->getFaction()->getAttitudeTowards(
				targets[i]->getFaction())==Faction::HOSTILE)
			{
				r->begin(JR_DRAWMODE_CONNECTED_LINES);
				*r << p+Vector(-size,-size,0)
				   << p+Vector( size,-size,0)
				   << p+Vector( size, size,0)
				   << p+Vector(-size, size,0)
				   << p+Vector(-size,-size,0);
				r->end();
			} else {
				r->begin(JR_DRAWMODE_LINES);
				*r << p+2*Vector(-size,-size,0)
				   << p+2*Vector( size, size,0)
				   << p+2*Vector( size,-size,0)
				   << p+2*Vector(-size, size,0);
				r->end();
				
			}
		}
		
		if (!targeter->getCurrentTarget()) {
			r->popMatrix();
			return;
		}
		Ptr<IActor> target = targeter->getCurrentTarget();
			
		Vector rel = M*target->getLocation();
		
		if (rel[2] > 0
			&& abs(rel[0]/rel[2])*focus<aspect
			&& abs(rel[1]/rel[2])*focus<1)
		{
			Vector p=(zfactor/rel[2])*Vector(rel[0], -rel[1], 0);
			float size = target->getTargetInfo() ?
						 target->getTargetInfo()->getTargetSize() :
						 5;
			size *= zfactor / rel[2];
			size = max(5.0f, size);
			r->begin(JR_DRAWMODE_CONNECTED_LINES);
			*r << p+1.4*Vector(-size,-size,0)
			   << p+1.4*Vector( size,-size,0)
			   << p+1.4*Vector( size, size,0)
			   << p+1.4*Vector(-size, size,0)
			   << p+1.4*Vector(-size,-size,0);
			r->end();
		} else {
			Vector2 dir(rel[0], -rel[1]);
			if (dir.lengthSquare()<1e-5) {
				dir=Vector2(1,0);
			} else {
				dir.normalize();
			}
			float angle = atan2(dir[1],dir[0]);
			float l=0.475*surface.getHeight();
			float s=0.02*surface.getHeight();
			Matrix M( cos(angle),-sin(angle),0,0,
			          sin(angle), cos(angle),0,0,
			                   0,          0,0,0,
			                   0,          0,0,1);
			M=M*TranslateMatrix<4,float>(Vector(l,0,0));
			r->begin(JR_DRAWMODE_TRIANGLES);
			*r << M*Vector(0,s,0) << M*Vector(0,-s,0) << M*Vector(s,0,0);
			r->end();
			
		}
		
		r->popMatrix();
	}
};

void FlexibleGunsight::addTargeting(
	Ptr<IActor> self,
	Ptr<Targeter> targeter)
{
    addModule(new TargetingModule(
    		surface.getWidth(), surface.getHeight(),
    		self,targeter),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
}

