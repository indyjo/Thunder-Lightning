#include <cmath>
#include <vector>
#include <modules/math/SpecialMatrices.h>
#include <modules/jogi/JRenderer.h>
#include <modules/weaponsys/Armament.h>
#include <modules/weaponsys/Targeter.h>
#include <modules/weaponsys/Weapon.h>
#include <interfaces/IFontMan.h>
#include <interfaces/IView.h>
#include <TargetInfo.h>
#include <Faction.h>
#include "gunsight.h"

using namespace std;

struct TargetingModule : public UI::Component {
	Ptr<IActor> self;
	Ptr<Targeter> targeter;
	
	TargetingModule(
		Ptr<IActor> self,
		Ptr<Targeter> targeter)
	:	UI::Component("targeting", 16, 16),
		self(self),
		targeter(targeter)
	{
	}
	
	void draw(UI::Panel & gunsight) {
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
			
		    r->setColor(targets[i]->getFaction()->getColor());
			
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
		r->setColor(target->getFaction()->getColor());
			
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
			r->setAlpha(0.5);
			r->enableAlphaBlending();
			r->begin(JR_DRAWMODE_TRIANGLES);
			*r << M*Vector(0,s,0) << M*Vector(0,-s,0) << M*Vector(s,0,0);
			r->end();
			r->disableAlphaBlending();
			
		}
		
		r->popMatrix();
	}

	void onLayout(UI::Panel & panel) {
	    width = panel.getSurface().getWidth();
	    height = panel.getSurface().getHeight();
	}
};


#define PI 3.141593f
#define BULLET_SPEED 900.0f
#define BULLET_RANGE 1000.0f
#define BULLET_TTL (BULLET_RANGE / BULLET_SPEED)

struct AimingModule : public UI::Component {
	Ptr<IMovementProvider> view;
	Ptr<Targeter> targeter;
	Ptr<Armament> armament;
	
	AimingModule(
		Ptr<IMovementProvider> view,
		Ptr<Targeter> targeter,
		Ptr<Armament> armament)
	:	UI::Component("aiming", 16, 16),
		view(view),
		targeter(targeter),
		armament(armament)
	{
	}
	
	void draw(UI::Panel & gunsight) {
	    if(armament->currentWeapon(0)->isGuided()) {
	        return;
	    }
	
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
		
		Vector p = view->getLocation();
		Vector v = view->getMovementVector();
		
	    Ptr<IActor> other = targeter->getCurrentTarget();
	    if (!other) {
	      r->popMatrix();
	      return;
	    }
	    Vector target_p = other->getLocation();
	    if ((target_p-p).length() > armament->currentWeapon(0)->maxRange()) {
	      r->popMatrix();
	      return;
	    }

	    
	    Vector target_v = other->getMovementVector();
	    
	    Vector target_dir = (target_p-p).normalize();
	    Vector target_vrel = target_v - target_dir*(target_dir*target_v);
	    float target_vrel2 = target_vrel.lengthSquare();
	    float v2 = v.length() + armament->currentWeapon(0)->referenceSpeed();
	    v2 *= v2;
	    if (target_vrel2 > v2) {
	      r->popMatrix();
	      return;
	    }
	    Vector desired_v = target_vrel + target_dir * sqrt(v2 - target_vrel2);
	    Vector desired_dir = Vector(desired_v).normalize();
	    
	    Vector desired_projectile_v = desired_v + desired_dir * armament->currentWeapon(0)->referenceSpeed();
	    float time_to_impact = (target_p-p).length() / ((desired_projectile_v-target_v)*target_dir);
	    float gravity_offset = 0.5f * 9.81 * time_to_impact*time_to_impact;
	    Vector point_of_impact = time_to_impact * desired_v;
	    Vector point_of_aim = point_of_impact + Vector(0, gravity_offset, 0);
	    
		Vector dir = M*(point_of_aim).normalize();
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
	
	void onLayout(UI::Panel & panel) {
	    width = panel.getSurface().getWidth();
	    height = panel.getSurface().getHeight();
	}
};

void FlexibleGunsight::addTargeting(
	Ptr<IMovementProvider> view,
	Ptr<Targeter> targeter,
	Ptr<Armament> armament)
{
    addModule(new TargetingModule(
    		& (targeter->getSubjectActor()),targeter),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
    addModule(new AimingModule(
    		view,targeter,armament),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
}


struct DirectionOfFlightModule : public UI::Component {
	Ptr<IActor> self;
	
	DirectionOfFlightModule(
		Ptr<IActor> self)
	:	UI::Component("direction-of-flight", width, height),
		self(self)
	{
	}
	
	void draw(UI::Panel & gunsight) {
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

	void onLayout(UI::Panel & panel) {
	    width = panel.getSurface().getWidth();
	    height = panel.getSurface().getHeight();
	}
};

void FlexibleGunsight::addDirectionOfFlight(
	Ptr<IActor> self)
{
    addModule(new DirectionOfFlightModule(self),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
}

struct InterceptionModule : public UI::Component {
	Ptr<IActor> self, other;
	
	InterceptionModule(
		float width, float height,
		Ptr<IActor> self, Ptr<IActor> other)
	:	UI::Component("interception", width, height),
		self(self), other(other)
	{
	}
	
	void draw(UI::Panel & gunsight) {
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
    		getSurface().getWidth(), getSurface().getHeight(),
    		self, other),
        "screen", HCENTER | VCENTER, HCENTER | VCENTER);
}

struct CurrentWeaponModule : public UI::Component {
	Ptr<Armament> arms;
	Ptr<IFontMan> fontman;
    size_t weapon_group;
    Ptr<IGame> thegame;
	
	CurrentWeaponModule(const char *name, Ptr<IGame> game, Ptr<Armament> arms, size_t weapon_group)
	:	UI::Component(name, 200, 25),
		arms(arms),
		thegame(game),
		fontman(game->getFontMan()),
        weapon_group(weapon_group)
	{
        height = fontman->selectNamedFont("HUD_font_medium")->getLineHeight();
	}
	
	void draw(UI::Panel & gunsight) {
		UI::Surface surface = gunsight.getSurface();
		surface.translateOrigin(offset[0],offset[1]);
		
        fontman->selectNamedFont("HUD_font_medium");
		
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
        "screen", LEFT | BOTTOM, LEFT | BOTTOM, Vector(5,-5,0));
}

void FlexibleGunsight::addArmamentToScreenRight(
	Ptr<IGame> game, Ptr<Armament> arms, size_t weapon_group)
{
    addModule(new CurrentWeaponModule("secondary", game, arms, weapon_group),
        "screen", RIGHT | BOTTOM, RIGHT | BOTTOM, Vector(-5,-5,0));
}
