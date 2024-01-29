#pragma once
#include "myFace.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <vector>
#include <string>

class myMesh
{
public:
	std::vector<myVertex *> vertices;
	std::vector<myHalfedge *> halfedges;
	std::vector<myFace *> faces;
	std::string name;

	void checkMesh();
	bool readFile(std::string filename);
	void computeNormals();
	void normalize();

	void subdivisionCatmullClark();

	void splitFaceTRIS(myFace *, myPoint3D *);

	void splitEdge(myHalfedge *, myPoint3D *);
	void splitFaceQUADS(myFace *, myPoint3D *);
	void collapse(const int count);
	void cleanValues();
	void checkValues() const;
	template <typename T>
	int validate(std::vector<T> v);
	void validate_mesh();
	void swap_old_objects(myVertex* new_vertex);
	void collapse_half_edge(myHalfedge* he, myVertex* new_vertex);
	void collapse(double threshold);
	void triangulate();
	static bool triangulate(myFace *);

	void clear();

	myMesh(void);
	~myMesh(void);
};


class invalid_mesh_exception final : public std::exception {
public:
	invalid_mesh_exception(const char* str) : exception(str){}
};