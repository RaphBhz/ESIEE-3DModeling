#include "myHalfedge.h"
#include "myMesh.h"
#include <cmath>
#include <glm/gtc/quaternion.hpp>

#include "mypoint3d.h"
#include "myVertex.h"

myHalfedge::myHalfedge(void)
{
	source = NULL; 
	adjacent_face = NULL; 
	next = NULL;  
	prev = NULL;  
	twin = NULL;  
}

void myHalfedge::copy(myHalfedge *ie)
{
/**** TODO ****/
}

myHalfedge::~myHalfedge(void)
{
}

myPoint3D* myHalfedge::middle() const
{
	myPoint3D* a = new myPoint3D(0, 0, 0);
	*a += *(this->source->point);
	*a += *(this->next->source->point);
	*a /= 2;

	return a;
}

double myHalfedge::length() const
{
	const myPoint3D* a = this->source->point;
	const myPoint3D* b = this->next->source->point;

	return sqrt(pow(b->X - a->X, 2) + pow(b->Y - a->Y, 2) + pow(b->Z - a->Z, 2));
}

void myHalfedge::validate(myMesh* mesh) const
{
	if (this->index == -1)
		throw invalid_mesh_exception("Invalid half-edge: index is -1, half-edge must be deleted");

	if (this->source == nullptr)
		throw invalid_mesh_exception("Invalid half-edge: source is null");

	if (this->next == nullptr)
		throw invalid_mesh_exception("Invalid half-edge: next is null");

	if (this->prev == nullptr)
		throw invalid_mesh_exception("Invalid half-edge: prev is null");

	if (this->twin == nullptr)
		throw invalid_mesh_exception("Invalid half-edge: twin is null");

	if (this->adjacent_face == nullptr)
		throw invalid_mesh_exception("Invalid half-edge: adjacent face is null");

	std::vector<myVertex*>::iterator it;
	for (it = mesh->vertices.begin(); it != mesh->vertices.end(); ++it)
	{
		if ((*it) == this->source)
			break;
	}
	if (it == mesh->vertices.end())
		throw invalid_mesh_exception("Invalid half-edge: source wasn't found in mesh");

	std::vector<myFace*>::iterator it2;
	for (it2 = mesh->faces.begin(); it2 != mesh->faces.end(); ++it2)
	{
		if ((*it2) == this->adjacent_face)
			break;
	}
	if (it2 == mesh->faces.end())
		throw invalid_mesh_exception("Invalid half-edge: adjacent face wasn't found in mesh");

	bool next = false;
	bool prev = false;
	bool twin = false;
	for (const auto& halfedge : mesh->halfedges)
	{
		if (halfedge == this->next)
			next = true;
		if (halfedge == this->prev)
			prev = true;
		if (halfedge == this->twin)
			twin = true;
		if (next && prev && twin) break;
	}
	if (!prev)
		throw invalid_mesh_exception("Invalid half-edge: prev wasn't found in mesh");
	if (!next)
		throw invalid_mesh_exception("Invalid half-edge: next wasn't found in mesh");
	if (!twin)
		throw invalid_mesh_exception("Invalid half-edge: twin wasn't found in mesh");

	if (this->twin->twin != this)
		throw invalid_mesh_exception("Invalid half-edge: twin of twin isn't this");

	if (this->next->prev != this)
		throw invalid_mesh_exception("Invalid half-edge: prev of next isn't this");

	if (this->prev->next != this)
		throw invalid_mesh_exception("Invalid half-edge: next of prev isn't this");
}
