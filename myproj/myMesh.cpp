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

	vector<myVertex*> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge*> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace*> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	vector<myHalfedge*>::iterator it;
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

	map<pair<int, int>, myHalfedge*> twin_map;
	map<pair<int, int>, myHalfedge*>::iterator it;

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
	for (const vector<int>& face_ids : obj_faces)
	{
		auto* new_face = new myFace();

		// Temporary vector for the face's half-edges
		vector<myHalfedge*> face_hedges;

		// Registering the new face.
		faces.push_back(new_face);

		// Building the face's half-edges.
		int n = static_cast<int>(face_ids.size());
		new_face->index = n;
		for (int i = 0; i < n; i++)
		{
			// Building an edge from vertices indexes.
			int idx1 = face_ids[i];
			int idx2 = i < n ? face_ids[i + 1] : face_ids[0];

			// Creating the new half-edge.
			auto* new_h_edge = new myHalfedge();
			new_h_edge->index = i;
			new_h_edge->adjacent_face = new_face;

			// Retrieving the corresponding vertex and linking the half-edge with it.
			vertices.at(idx1 - 1)->originof = new_h_edge;
			new_h_edge->source = vertices.at(idx1 - 1);

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
			face_hedges.push_back(new_h_edge);
		}

		// Linking the half-edges together.
		int he_count = static_cast<int>(face_hedges.size());
		for (int i = 0; i < he_count; ++i) {
			face_hedges[i]->next = face_hedges[(i + 1) % he_count];
			face_hedges[i]->prev = face_hedges[(i + he_count - 1) % he_count];
		}

		// Link the new face to a corresponding half-edge
		new_face->adjacent_halfedge = halfedges.back();
	}

	// Clear the temporary vector
	obj_faces.clear();

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


void myMesh::splitFaceTRIS(myFace* f, myPoint3D* p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge* e1, myPoint3D* p)
{
	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace* f, myPoint3D* p)
{
	/**** TODO ****/
}


void myMesh::subdivisionCatmullClark()
{
	/**** TODO ****/
}


void myMesh::triangulate()
{
	// Newly created triangle faces.
	vector<myFace*> new_faces;

	for (const auto face : faces)
	{
		if (triangulate(face))
		{
			// Splitting every non-triangle face into triangles.
			myHalfedge* he = face->adjacent_halfedge;

			// Registering half-edges to iterate through.
			vector<myHalfedge*> face_hes;
			for (int i = 0; i < face->index; i++)
			{
				face_hes.push_back(he);
				he = he->next;
			}

			// Variable for new triangle's twin determination.
			myHalfedge* twin = nullptr;

			// Center point
			myPoint3D* center = face->computeCenter();

			// Looping through half-edges.
			for (int i = 0; i < face->index; i++)
			{
				// Creating the new face.
				auto* new_face = new myFace();
				new_face->index = 3;
				face_hes.at(i)->adjacent_face = new_face;
				face_hes.at(i)->index = 1;

				// Triangle's first half-edge and vertex.
				auto* h1 = new myHalfedge();
				auto* v1 = new myVertex();

				v1->point = center;
				v1->originof = h1;

				h1->index = 0;
				h1->source = v1;
				h1->adjacent_face = new_face;

				// Triangle's third half-edge and vertex.
				auto* h3 = new myHalfedge();

				h3->index = 2;
				h3->source = face_hes.at(i)->next->source;
				h3->adjacent_face = new_face;

				// Connecting the triangle's half-edges together.
				h1->prev = h3;
				h1->next = face_hes.at(i);
				face_hes.at(i)->prev = h1;
				face_hes.at(i)->next = h3;
				h3->prev = face_hes.at(i);
				h3->next = h1;

				// Connecting the new face with its half-edges.
				new_face->adjacent_halfedge = h1;

				// Registering the new face, half-edges and vertex.
				new_faces.push_back(new_face);
				halfedges.push_back(h1);
				halfedges.push_back(h3);
				vertices.push_back(v1);

				// Determining newly created half-edges' twins
				if (twin != nullptr)
				{
					h1->twin = twin;
					twin->twin = h1;
				}

				twin = h3;
			}

			// Determining twin relationship between first and last created triangles.
			const myFace* first = new_faces.at(0);
			first->adjacent_halfedge->twin = face_hes.at(face_hes.size() - 1);
			face_hes.at(face_hes.size() - 1)->twin = first->adjacent_halfedge;

			// Setting non-triangle face to be deleted.
			face->index = -1;
		}
	}

	// Deleting old faces with index -1.
	faces.erase(remove_if(faces.begin(), faces.end(), [&](const myFace* f) {
		const bool res = f->index == -1;
		if (res) delete f;
		return res;
		}), faces.end());

	// Adding newly created faces.
	faces.insert(faces.end(), new_faces.begin(), new_faces.end());
}

//return false if already triangle, true othewise.
bool myMesh::triangulate(myFace* f)
{
	return f->adjacent_halfedge != f->adjacent_halfedge->next->next->next;
}

