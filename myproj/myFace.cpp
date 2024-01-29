#include "myFace.h"

#include <iostream>

#include "myvector3d.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <GL/glew.h>

#include "myMesh.h"

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
	const myHalfedge* first = adjacent_halfedge;

	// Computing the face's center
	int cpt = 0;
	auto* center = new myPoint3D(0,0,0);
	do
	{
		*center += *(he->source->point);
		cpt++;
		he = he->next;
	}
	while (he != nullptr && he != first);
	*center /= cpt;

	return center;
}

void myFace::computeNormal()
{
	const auto* x0 = new myVector3D(
		adjacent_halfedge->next->source->point->X - adjacent_halfedge->source->point->X,
		adjacent_halfedge->next->source->point->Y - adjacent_halfedge->source->point->Y,
		adjacent_halfedge->next->source->point->Z - adjacent_halfedge->source->point->Z
	);
	const auto* x1 = new myVector3D(
		adjacent_halfedge->next->next->source->point->X - adjacent_halfedge->next->source->point->X,
		adjacent_halfedge->next->next->source->point->Y - adjacent_halfedge->next->source->point->Y,
		adjacent_halfedge->next->next->source->point->Z - adjacent_halfedge->next->source->point->Z
	);
	const auto* xn = new myVector3D(
		x0->dY * x1->dZ - x0->dZ * x1->dY,
		x0->dZ * x1->dX - x0->dX * x1->dZ,
		x0->dX * x1->dY - x0->dY * x1->dX
	);

	const float norm = sqrtf(pow(xn->dX, 2) + pow(xn->dY, 2) + pow(xn->dZ, 2));

	this->normal->dX = xn->dX / norm;
	this->normal->dY = xn->dY / norm;
	this->normal->dZ = xn->dZ / norm;
}

void myFace::validate(myMesh* mesh) const
{
	if (this->index == -1)
		throw invalid_mesh_exception("Invalid face: index is -1, face must be deleted");

	if (this->adjacent_halfedge == nullptr)
		throw invalid_mesh_exception("Invalid face: adjacent half-edge is null");

	std::vector<myHalfedge*>::iterator it;
	for (it = mesh->halfedges.begin(); it != mesh->halfedges.end(); ++it)
	{
		if ((*it) == this->adjacent_halfedge)
			break;
	}
	if (it == mesh->halfedges.end())
		throw invalid_mesh_exception("Invalid face: adjacent half-edge wasn't found in mesh");

	int iterations = 0;
	const myHalfedge* he = this->adjacent_halfedge->next;
	do
	{
		if (he->adjacent_face != this)
			throw invalid_mesh_exception("Invalid face: half-edges aren't all pointing to face");
		if (he->twin->adjacent_face == this)
			throw invalid_mesh_exception("Invalid face: half-edge twin points to face");

		++iterations;
		he = he->next;
	} while (he != this->adjacent_halfedge && iterations < 50);
	if (he != this->adjacent_halfedge)
		throw invalid_mesh_exception("Invalid face: the half-edges don't loop");

}
