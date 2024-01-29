#pragma once
#include "myMesh.h"
#include "mypoint3d.h"

class myMesh;
class myHalfedge;
class myVector3D;

class myVertex
{
public:
	myPoint3D *point;
	myHalfedge *originof;

	int index;  //use as you wish.

	myVector3D *normal;

	void computeNormal();
	void validate(myMesh* mesh) const;
	myVertex(void);
	~myVertex(void);
};
