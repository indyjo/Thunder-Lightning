#include <modules/model/model.h>

#include "PassiveActor.h"

class JRenderer;

PassiveActor::PassiveActor(Ptr<IGame> game)
: 	SimpleActor(game)
{ }

void PassiveActor::draw() {
	if (!model) return;
	
    Vector p = engine->getLocation();
    Matrix Translation = TranslateMatrix<4,float>(p);

    Vector right, up, front;
    getOrientation(&up, &right, &front);
    Matrix Rotation    = Matrix::Hom(
        MatrixFromColumns(right, up, front));

    Matrix Mmodel  = Translation * Rotation;
    
	JRenderer * renderer = thegame->getRenderer();
    renderer->enableSmoothShading();
    renderer->setCullMode(JR_CULLMODE_CULL_NEGATIVE);
    renderer->setAlpha(1);
    renderer->setColor(Vector(1,1,1));
    
    model->draw(*renderer, Mmodel, Rotation);
}

void PassiveActor::action() {
	SimpleActor::action();
}
