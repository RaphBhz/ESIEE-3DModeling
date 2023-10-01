#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"

using namespace std;

myMesh::myMesh(void)
{
	/**** TODO ****/
}


myMesh::~myMesh(void)
{
	/**** TODO ****/
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vector<myVertex *> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge *> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace *> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	vector<myHalfedge *>::iterator it;
	for (it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			break;
	}
	if (it != halfedges.end())
		cout << "Error! Not all edges have their twins!\n";
	else cout << "Each edge has a twin!\n";
}


bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	vector<int> faceids;
	myHalfedge **hedges;

	ifstream fin(filename);
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	int i = 0;
	std::vector<myPoint3D> points;
	map<pair<int, int>, myHalfedge *> twin_map;
	map<pair<int, int>, myHalfedge *>::iterator it;

	while (getline(fin, s))
	{
		stringstream myline(s);
		myline >> t;
		if (t == "g") {}
		else if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;
			cout << "v " << x << " " << y << " " << z << endl;
			myPoint3D A;
			A.X = x;
			A.Y = y;
			A.Z = z;
			points.push_back(A);
		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			cout << "f";
			std::vector<int> indexes;

			while (myline >> u)
			{
				int idx = atoi((u.substr(0, u.find("/"))).c_str());
				cout << " " << idx;
				indexes.push_back(idx);
			}
			cout << endl;

			int firstPoint = indexes.at(0);
			int lastPoint = firstPoint;
			int currentPoint;
			myHalfedge lastHEdge;

			for (int i = 1; i < indexes.size(); i++)
			{
				currentPoint = indexes.at(i);
				myHalfedge newHEdge;
				myVertex newVertex;
				myPoint3D newPoint;
				myFace newFace;
				myVector3D newVector;

				newPoint = points.at(lastPoint);
				newVertex.originof = &newHEdge;
				newVertex.point = &newPoint;
				newHEdge.source = &newVertex;
				
				//TODO: Vecteur normal
				newVector.dX = 0;
				newVector.dY = 0;
				newVector.dZ = 0;

				newFace.normal = &newVector;
				newFace.adjacent_halfedge = &newHEdge;
				newHEdge.adjacent_face = &newFace;

				if (i != 1)
				{
					lastHEdge.next = &newHEdge;
					newHEdge.prev = &lastHEdge;
				}

				map<pair<int, int>, myHalfedge*>::iterator it = twin_map.find(make_pair(lastPoint, currentPoint));
				if (it == twin_map.end())
				{
					// This means there was no myHalfedge* present at location(a, b).
					twin_map.insert(pair<pair<int, int>, myHalfedge*>(make_pair(lastPoint, currentPoint), &newHEdge));
				}
				else
				{
					// It was found.The variable it->second is of type myHalfedge*,
					// and is the halfedge present at location(a, b).
					newHEdge.twin = it->second;
					it->second->twin = &newHEdge;
				}
				lastHEdge = newHEdge;
				lastPoint = currentPoint;
			}
		}
	}

	checkMesh();
	normalize();

	return true;
}


void myMesh::computeNormals()
{
	/**** TODO ****/
}

void myMesh::normalize()
{
	if (vertices.size() < 1) return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X) tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X) tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y) tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y) tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z) tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z) tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}


void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{

	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}


void myMesh::subdivisionCatmullClark()
{
	/**** TODO ****/
}


void myMesh::triangulate()
{
	/**** TODO ****/
}

//return false if already triangle, true othewise.
bool myMesh::triangulate(myFace *f)
{
	/**** TODO ****/
	return false;
}

