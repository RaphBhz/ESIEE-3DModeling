#pragma once

#include "myMesh.h"

class myMesh;
class myVertex;
class myFace;
class myPoint3D;

class myHalfedge
{
public:
	myVertex *source; 
	myFace *adjacent_face; 
	myHalfedge *next;  
	myHalfedge *prev;  
	myHalfedge *twin;

	int index; //use as you wish.

	myHalfedge(void);
	void copy(myHalfedge *);
	myPoint3D* middle() const;
	double length() const;
	void validate(myMesh* mesh) const;
	~myHalfedge(void);
};