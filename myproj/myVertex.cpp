#include "myVertex.h"
#include "myMesh.h"

#include <exception>

#include "myvector3d.h"
#include "myHalfedge.h"
#include "myFace.h"
#include <iostream>

myVertex::myVertex(void)
{
	point = NULL;
	originof = NULL;
	normal = new myVector3D(1.0,1.0,1.0);
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}

void myVertex::computeNormal()
{
	int cpt = 1;
	auto* he = originof->twin->next;
	auto* xn = new myVector3D(originof->source->point->X, originof->source->point->Y, originof->source->point->Z);

	while (he != originof)
	{
		cpt++;

		xn->dX += he->source->point->X;
		xn->dY += he->source->point->Y;
		xn->dZ += he->source->point->Z;

		he = he->twin->next;
	}

	xn->dX = xn->dX / cpt;
	xn->dY = xn->dY / cpt;
	xn->dZ = xn->dZ / cpt;

	const float norm = sqrtf(pow(xn->dX, 2) + pow(xn->dY, 2) + pow(xn->dZ, 2));

	this->normal->dX = xn->dX / norm;
	this->normal->dY = xn->dY / norm;
	this->normal->dZ = xn->dZ / norm;
}

void myVertex::validate(myMesh* mesh) const
{
	if (this->index == -1)
		throw invalid_mesh_exception("Invalid vertex: index is -1, vertex must be deleted");

	if (this->originof == nullptr)
		throw invalid_mesh_exception("Invalid vertex: originated half-edge is null");
	if (this->point == nullptr)
		throw invalid_mesh_exception("Invalid vertex: point is null");

	for (auto* v : mesh->vertices)
	{
		if (v != this && v->point->X == this->point->X && v->point->Y == this->point->Y && v->point->Z == this->point->Z)
			std::cout << "\033[1;33m\tWarning: two vertices have the same coordinates\033[0m" << std::endl;
	}

	std::vector<myHalfedge*>::iterator it;
	for (it = mesh->halfedges.begin(); it != mesh->halfedges.end(); ++it)
	{
		if ((*it) == this->originof)
			break;
	}
	if (it == mesh->halfedges.end())
		throw invalid_mesh_exception("Invalid vertex: originated half-edge wasn't found");

	int iterations = 0;
	const myHalfedge* first = this->originof->twin->next;
	const myHalfedge* he = first;
	do
	{
		if (he->source != this)
			throw invalid_mesh_exception("Invalid vertex: half-edge is not originating from it as it should");

		he = he->twin->next;
		++iterations;
	} while (he != first && iterations < 50);
	if (iterations >= 50)
		throw invalid_mesh_exception("Invalid vertex: originated half-edges are not looping");
}
