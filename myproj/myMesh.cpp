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

	ifstream fin(filename);
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge *> twin_map;
	map<pair<int, int>, myHalfedge *>::iterator it;

	// Temporary vector to store faces vertices ids.
	vector<vector<int>> obj_faces;

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

			// Registering the vertex.
			auto* v = new myVertex();
			v->point = new myPoint3D(x, y, z);
			vertices.push_back(v);
		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			cout << "f";

			// Reading the current face's vertices.
			vector<int> face_ids;
			while (myline >> u)
			{
				int vertex = atoi((u.substr(0, u.find("/"))).c_str());
				cout << " " << vertex;

				face_ids.push_back(vertex);
			}
			// Storing the new face as a series of vertices ids.
			obj_faces.push_back(face_ids);

			cout << endl;
		}
	}

	// Rebuilding each face from its vertices.
	for (const vector<int> &face_ids : obj_faces)
	{
		auto* new_face = new myFace();
		// Registering the new face.
		faces.push_back(new_face);

		// Building the face's half-edges.
		int n = static_cast<int>(face_ids.size());
		for (int i = 0; i < n; i++)
		{
			// Building an edge from vertices indexes.
			int idx1 = face_ids[i];
			int idx2 = i < n ? face_ids[i+1] : face_ids[0];

			// Creating the new half-edge.
			auto* new_h_edge = new myHalfedge();
			new_h_edge->adjacent_face = new_face;

			// Retrieving the corresponding vertex and linking the half-edge with it.
			vertices.at(idx1 - 1)->originof = new_h_edge;
			new_h_edge->source = vertices.at(idx1 - 1);

			// Linking previous and current half-edges together.
			if (!halfedges.empty())
			{
				halfedges.back()->next = new_h_edge;
				new_h_edge->prev = halfedges.back();
			}

			// Try to retrieve the current half-edge's twin.
			it = twin_map.find(make_pair(idx2, idx1));
			if (it == twin_map.end())
			{
				// This means there was no myHalfedge* present at location(a, b).
				// Then we create an entry in the twin mapping.
				twin_map.insert(pair<pair<int, int>, myHalfedge*>(make_pair(idx1, idx2), new_h_edge));
			}
			else
			{
				// It was found. The variable it->second is of type myHalfedge*,
				// and is the half-edge present at location(a, b).
				new_h_edge->twin = it->second;
				it->second->twin = new_h_edge;
			}

			// Registering the half-edge.
			halfedges.push_back(new_h_edge);
		}

		// Link the new face to a corresponding half-edge
		new_face->adjacent_halfedge = halfedges.back();
	}
	// Link the last half-edge with the first to complete the loop.
	halfedges.back()->next = halfedges.front();
	halfedges.front()->prev = halfedges.back();

	// Clear the temporary vector
	obj_faces.clear();

	int cpt = 0;
	for (const myHalfedge* he : halfedges)
	{
		if (he->twin == nullptr) cpt++;
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

