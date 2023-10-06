#include "myFace.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <GL/glew.h>

myFace::myFace(void)
{
	adjacent_halfedge = NULL;
	normal = new myVector3D(1.0, 1.0, 1.0);
}

myFace::~myFace(void)
{
	if (normal) delete normal;
}

myPoint3D* myFace::computeCenter()
{
	myHalfedge* he = adjacent_halfedge;

	// Computing the face's center
	int cpt = 1;
	myPoint3D* center = he->source->point;
	while (he->next != nullptr && he->next->index != 0)
	{
		he = he->next;
		center->X += he->source->point->X;
		center->Y += he->source->point->Y;
		center->Z += he->source->point->Z;
		cpt++;
	}
	center->X /= cpt;
	center->Y /= cpt;
	center->Z /= cpt;

	return center;
}

void myFace::computeNormal()
{
	/**** TODO ****/
}
