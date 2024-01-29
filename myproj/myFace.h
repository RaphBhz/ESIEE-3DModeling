#pragma once
#include "myMesh.h"

class myMesh;
class myHalfedge;
class myVector3D;
class myPoint3D;

class myFace
{
public:
	myHalfedge *adjacent_halfedge;

	myVector3D *normal;

	int index; //use this variable as you wish.

	void computeNormal();
	void validate(myMesh* mesh) const;
	myFace(void);
	~myFace(void);
	myPoint3D* computeCenter();
};