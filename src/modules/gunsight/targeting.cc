#include <cmath>
#include <vector>
#include <modules/math/SpecialMatrices.h>
#include <modules/jogi/JRenderer.h>
#include <modules/weaponsys/Armament.h>
#include <modules/weaponsys/Targeter.h>
#include <modules/weaponsys/Weapon.h>
#include <interfaces/IFontMan.h>
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
		           * TranslateMatrix<4,float>(-cam->getLocation());
		
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
			size = std::max(5.0f, size);
			
			if (self->getFaction()->getAttitudeTowards(targets[i]->getFaction())==Faction::HOSTILE)
			{
			    r->setColor(Vector(1,0,0));
			} else if (self->getFaction()->getAttitudeTowards(targets[i]->getFaction())==Faction::NEUTRAL)
			{
			    r->setColor(Vector(0,0,1));
			} else
			{
    			r->setColor(Vector(0,1,0));
			}
			
			r->begin(JR_DRAWMODE_CONNECTED_LINES);
			*r << p+Vector(-size,-size,0)
			   << p+Vector( size,-size,0)
			   << p+Vector( size, size,0)
			   << p+Vector(-size, size,0)
			   << p+Vector(-size,-size,0);
			r->end();
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
			size = std::max(5.0f, size);
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


#define PI 3.141593f
#define BULLET_SPEED 900.0f
#define BULLET_RANGE 1000.0f
#define BULLET_TTL (BULLET_RANGE / BULLET_SPEED)

struct AimingModule : public GunsightModule {
	Ptr<IActor> self;
	Ptr<Targeter> targeter;
	
	AimingModule(
		float width, float height,
		Ptr<IActor> self,
		Ptr<Targeter> targeter)
	:	GunsightModule("aiming", width, height),
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
		float focus = cam->getFocus();
		float zfactor = 0.5*surface.getHeight() * focus;
		float aspect = cam->getAspect();
		Matrix3 M = cam->getOrientInv();
		
		r->pushMatrix();
		r->multMatrix(surface.getMatrix());
		
		Vector p = self->getLocation();
		Vector v = self->getMovementVector();
		
	    Ptr<IActor> other = targeter->getCurrentTarget();
	    if (!other) {
	      r->popMatrix();
	      return;
	    }
	    Vector target_p = other->getLocation();
	    Vector target_v = other->getMovementVector();
	    
	    Vector target_dir = (target_p-p).normalize();
	    Vector target_vrel = target_v - target_dir*(target_dir*target_v);
	    float target_vrel2 = target_vrel.lengthSquare();
	    float v2 = v.length() + BULLET_SPEED;
	    v2 *= v2;
	    if (target_vrel2 > v2) {
	      r->popMatrix();
	      return;
	    }
	    Vector desired_v = target_vrel + target_dir * sqrt(v2 - target_vrel2);
		
		Vector dir = M*(desired_v).normalize();
		if (dir[2] <= 0) {
	      r->popMatrix();
	      return;
	    }
		
		p=(zfactor/dir[2])*Vector(dir[0], -dir[1], 0);
		
		float size = 4;
		int n = 8;
		
		r->setColor(Vector(1,0,0));
		r->begin(JR_DRAWMODE_CONNECTED_LINES);
		for (int i=0; i<=n; ++i)
		  *r << p + size*Vector(cos(2*PI*i/n), sin(2*PI*i/n), 0);
		r->end();
		
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
    addModule(new AimingModule(
    		surface.getWidth(), surface.getHeight(),
    		self,targeter),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
}


struct DirectionOfFlightModule : public GunsightModule {
	Ptr<IActor> self;
	
	DirectionOfFlightModule(
		float width, float height,
		Ptr<IActor> self)
	:	GunsightModule("direction-of-flight", width, height),
		self(self)
	{
	}
	
	void draw(FlexibleGunsight & gunsight) {
		JRenderer *r = gunsight.getRenderer();
		UI::Surface surface = gunsight.getSurface();
		surface.translateOrigin(
			offset[0]+width/2,
			offset[1]+height/2);
			
		Ptr<ICamera> cam = gunsight.getCamera();
		float focus = cam->getFocus();
		float zfactor = 0.5*surface.getHeight() * focus;
		float aspect = cam->getAspect();
		Matrix3 M = cam->getOrientInv();
		
		r->pushMatrix();
		r->multMatrix(surface.getMatrix());
		
		Vector dir = M*(self->getMovementVector()).normalize();
		if (dir[2] > 0) {
    		Vector p=(zfactor/dir[2])*Vector(dir[0], -dir[1], 0);
    		
    		float outer = 13, inner = 8;
    		
    		r->setColor(Vector(0,1,0));
    		r->begin(JR_DRAWMODE_LINES);
    		*r << p+Vector(-outer,-outer,0)
    		   << p+Vector(-inner,-inner,0)
    		   << p+Vector( outer,-outer,0)
    		   << p+Vector( inner,-inner,0)
    		   << p+Vector( outer, outer,0)
    		   << p+Vector( inner, inner,0)
    		   << p+Vector(-outer, outer,0)
    		   << p+Vector(-inner, inner,0);
            r->end();
        }
		
		r->popMatrix();
	}
};

void FlexibleGunsight::addDirectionOfFlight(
	Ptr<IActor> self)
{
    addModule(new DirectionOfFlightModule(
    		surface.getWidth(), surface.getHeight(),
    		self),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
}

struct InterceptionModule : public GunsightModule {
	Ptr<IActor> self, other;
	
	InterceptionModule(
		float width, float height,
		Ptr<IActor> self, Ptr<IActor> other)
	:	GunsightModule("interception", width, height),
		self(self), other(other)
	{
	}
	
	void draw(FlexibleGunsight & gunsight) {
		JRenderer *r = gunsight.getRenderer();
		UI::Surface surface = gunsight.getSurface();
		surface.translateOrigin(
			offset[0]+width/2,
			offset[1]+height/2);
			
		Ptr<ICamera> cam = gunsight.getCamera();
		float focus = cam->getFocus();
		float zfactor = 0.5*surface.getHeight() * focus;
		float aspect = cam->getAspect();
		Matrix3 M = cam->getOrientInv();
		
		r->pushMatrix();
		r->multMatrix(surface.getMatrix());
		
		Vector p = self->getLocation();
		Vector v = self->getMovementVector();
		
	    Vector target_p = other->getLocation();
	    Vector target_v = other->getMovementVector();
	    
	    Vector target_dir = (target_p-p).normalize();
	    Vector target_vrel = target_v - target_dir*(target_dir*target_v);
	    float target_vrel2 = target_vrel.lengthSquare();
	    float v2 = v.lengthSquare();
	    if (target_vrel2 > v2) target_vrel2 = v2;
	    Vector desired_v = target_vrel + target_dir * sqrt(v2 - target_vrel2);
		
		Vector dir = M*(desired_v).normalize();
		if (dir[2] <= 0) {
	      r->popMatrix();
	      return;
	    }
		
		p=(zfactor/dir[2])*Vector(dir[0], -dir[1], 0);
		
		float outer = 8, inner = 5;
		
		r->setColor(Vector(0,1,0));
		r->begin(JR_DRAWMODE_LINES);
		*r << p+Vector(-outer,-outer,0)
		   << p+Vector(-inner,-inner,0)
		   << p+Vector( outer,-outer,0)
		   << p+Vector( inner,-inner,0)
		   << p+Vector( outer, outer,0)
		   << p+Vector( inner, inner,0)
		   << p+Vector(-outer, outer,0)
		   << p+Vector(-inner, inner,0);
		r->end();
		
		r->popMatrix();
	}
};




void FlexibleGunsight::addInterception(
	Ptr<IActor> self, Ptr<IActor> other)
{
    addModule(new InterceptionModule(
    		surface.getWidth(), surface.getHeight(),
    		self, other),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
}

struct CurrentWeaponModule : public GunsightModule {
	Ptr<Armament> arms;
	Ptr<IFontMan> fontman;
    size_t weapon_group;
	
	CurrentWeaponModule(const char *name, Ptr<IGame> game, Ptr<Armament> arms, size_t weapon_group)
	:	GunsightModule(name, 200, 25),
		arms(arms),
		fontman(game->getFontMan()),
        weapon_group(weapon_group)
	{ }
	
	void draw(FlexibleGunsight & gunsight) {
		UI::Surface surface = gunsight.getSurface();
		surface.translateOrigin(offset[0],offset[1]);
		
		fontman->selectFont(IFontMan::FontSpec(
			"dungeon", 12));
		
		fontman->setCursor(
			surface.getOrigin(),
			surface.getDX(),
			surface.getDY());
		fontman->setAlpha(1);
		fontman->setColor(Vector(0,1,0));
		
		char buf[256];
        Ptr<Weapon> weapon = arms->currentWeapon(weapon_group);
		snprintf(buf,256,"%s:\t%4d / %4d\n",
		                 weapon->getName(),
		                 weapon->getRoundsLeft(),
		                 weapon->getMaxRounds());
		fontman->print(buf);
	}
};


void FlexibleGunsight::addArmamentToScreen(
	Ptr<IGame> game, Ptr<Armament> arms, size_t weapon_group)
{
    addModule(new CurrentWeaponModule("primary", game, arms, weapon_group),
        "screen", LEFT | BOTTOM, LEFT | BOTTOM, Vector(5,5,0));
}

void FlexibleGunsight::addArmamentToScreenRight(
	Ptr<IGame> game, Ptr<Armament> arms, size_t weapon_group)
{
    addModule(new CurrentWeaponModule("secondary", game, arms, weapon_group),
        "screen", RIGHT | BOTTOM, RIGHT | BOTTOM, Vector(-5,5,0));
}
