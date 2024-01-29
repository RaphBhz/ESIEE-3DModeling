#include "myMesh.h"

#include <algorithm>
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
	int count = 0;
	for (vector<myHalfedge*>::iterator it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			++count;
	}
	if (count > 0)
		std::cout << "\033[1;31m\tError! Not all edges have their twins: " << count << " missing\033[0m" << endl;
	else std::cout << "\tEach edge has a twin!\n";
}

bool myMesh::readFile(std::string filename)
{
	string s, t, u;

	ifstream fin(filename);
	if (!fin.is_open()) {
		std::cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge*> twin_map;
	map<pair<int, int>, myHalfedge*>::iterator it;

	int vertices_index = 1;
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
			std::cout << "v " << x << " " << y << " " << z << endl;

			// Registering the vertex.
			auto* v = new myVertex();
			v->point = new myPoint3D(x, y, z);
			v->index = vertices_index++;
			vertices.push_back(v);
		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			std::cout << "f";

			// Reading the current face's vertices.
			vector<int> face_ids;
			while (myline >> u)
			{
				int vertex = atoi((u.substr(0, u.find("/"))).c_str());
				std::cout << " " << vertex;
				face_ids.push_back(vertex);
			}
			// Storing the new face as a series of vertices ids.
			obj_faces.push_back(face_ids);

			std::cout << endl;
		}
	}

	// Rebuilding each face from its vertices.
	for (const vector<int>& face_ids : obj_faces)
	{
		auto* new_face = new myFace();

		// Temporary vector for the face's half-edges
		vector<pair<myHalfedge*, myVertex*>> face_hedges;

		// Registering the new face.
		faces.push_back(new_face);

		// Building the face's half-edges.
		int n = static_cast<int>(face_ids.size());
		new_face->index = n;
		for (int i = 0; i < n; i++)
		{
			// Building an edge from vertices indexes.
			int idx1 = face_ids[i];
			int idx2 = (i + 1) < n ? face_ids[i + 1] : face_ids[0];

			// Creating the new half-edge.
			auto* new_h_edge = new myHalfedge();
			new_h_edge->index = i;
			new_h_edge->adjacent_face = new_face;

			// Retrieving the corresponding vertex and linking the half-edge with it.
			auto vertex = find_if(vertices.begin(), vertices.end(), [&idx1](const myVertex* v)
			{
					return idx1 == v->index;
			});
			if (vertex != vertices.end())
			{
				(*vertex)->originof = new_h_edge;
				new_h_edge->source = *vertex;
			}
			else
			{
				throw invalid_mesh_exception("Error: a face vertex wasn't found");
			}

			// Retrieving the corresponding vertex of the next half-edge to link them later.
			auto next_vertex = find_if(vertices.begin(), vertices.end(), [&idx2](const myVertex* v)
				{
					return idx2 == v->index;
				});
			if (next_vertex == vertices.end())
			{
				throw invalid_mesh_exception("Error reading a file: a face vertex wasn't found");
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
			face_hedges.push_back(make_pair(new_h_edge, *next_vertex));
		}

		// Linking the half-edges together.
		for_each(face_hedges.begin(), face_hedges.end(), [&face_hedges](pair<myHalfedge*, myVertex*> current) {
			auto next = find_if(face_hedges.begin(), face_hedges.end(), [&current](pair<myHalfedge*, myVertex*> next) {
				return next.first->source == current.second;
			});
			auto prev = find_if(face_hedges.begin(), face_hedges.end(), [&current](pair<myHalfedge*, myVertex*> next) {
				return next.second == current.first->source;
			});

			if (prev == face_hedges.end() || next == face_hedges.end())
				throw invalid_mesh_exception("Error reading a file: half-edges cannot be linked");

			current.first->next = next->first;
			current.first->prev = prev->first;
		});

		// Link the new face to a corresponding half-edge
		new_face->adjacent_halfedge = face_hedges.begin()->first;

		// Clearing face edges
		face_hedges.clear();
	}

	// Clear the temporary vector
	obj_faces.clear();

	checkValues();
	checkMesh();
	normalize();

	return true;
}

void myMesh::computeNormals()
{
	// Computing normals of faces.
	for (auto* f : faces)
	{
		f->computeNormal();
	}
	// Computing normals of vertices (face normals needed).
	for (auto* v : vertices)
	{
		v->computeNormal();
	}
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

// TODO: Factorize code.
void myMesh::collapse_half_edge(myHalfedge* he, myVertex* new_vertex)
{
	// Deleting the old vertex and half-edge.
	he->source->index = -1;
	he->index = -1;

	// Making sure no face uses the half-edge to be deleted.
	for (auto* face : faces)
	{
		if (face->index != -1 && face->adjacent_halfedge == he)
		{
			// Trying to find an half-edge in the same face.
			auto* h = he->next;
			while (h != he)
			{
				if (h->index != -1)
				{
					face->adjacent_halfedge = h;
					break;
				}
				h = he->next;
			}

			// If we didn't find any, then the face has become useless.
			if (h == he)
			{
				face->index = -1;
			}
		}
	}

	// Making sure no vertex uses the half-edge to be deleted.
	for (auto* vertex : vertices)
	{
		if (vertex->index != -1 && vertex->originof == he)
		{
			// Finding an half-edge originating from the same vertex.
			auto* h = he->twin->next;
			while (h != he)
			{
				if (h->index != -1)
				{
					vertex->originof = h;
					break;
				}
				h = h->twin->next;
			}

			// If we didn't find any, then the vertex has become useless.
			if (h == he)
			{
				vertex->index = -1;
			}
		}
	}

	// Relinking half-edges to the new vertex.
	for (auto* he_modified : halfedges)
	{
		if (he_modified->index != -1 && he_modified->source == he->source)
			he_modified->source = new_vertex;
	}

	// Collapsing.
	myHalfedge* first = he->next;
	myHalfedge* last = he->prev;

	// Verifying if the face is triangle.
	if (!triangulate(he->adjacent_face))
	{
		// Deleting the triangle face.
		he->adjacent_face->index = -1;
		first->index = -1;
		last->index = -1;

		// Making sure no vertex uses the half-edges to be deleted.
		for (auto* vertex : vertices)
		{
			if (vertex->index != -1 && vertex->originof == first)
			{
				// Finding an half-edge originating from the same vertex.
				auto* h = first->twin->next;
				while (h != first)
				{
					if (h->index != -1)
					{
						vertex->originof = h;
						break;
					}
					h = h->twin->next;
				}

				// If we didn't find any, then the vertex has become useless.
				if (h == first)
				{
					vertex->index = -1;
				}
			}

			if (vertex->index != -1 && vertex->originof == last)
			{
				// Finding an half-edge originating from the same vertex.
				auto* h = last->twin->next;
				while (h != last)
				{
					if (h->index != -1)
					{
						vertex->originof = h;
						break;
					}
					h = h->twin->next;
				}

				// If we didn't find any, then the vertex has become useless.
				if (h == last)
				{
					vertex->index = -1;
				}
			}
		}

		// Deleting half-edges and linking twins of deleted half-edges.
		if (first->twin != nullptr && first->twin->index != -1 && last->twin != nullptr && last->twin->index != -1)
		{
			first->twin->twin = last->twin;
			last->twin->twin = first->twin;
			new_vertex->originof = last->twin;
		}
		else 
		{
			throw exception("Error: cannot link twins during triangle collapsing");
		}
	}
	else
	{
		// Linking impacted half-edges.
		last->next = first;
		first->prev = last;

		new_vertex->originof = first;
	}
}

void myMesh::collapse(const double threshold)
{
	int count = 0;

	for (auto* he : halfedges)
	{
		if (he->index != -1 && he->length() < threshold)
		{
			// Counting collapses.
			count++;

			// Computing the collapse vertex.
			myPoint3D* middle = he->middle();
			auto* middle_vertex = new myVertex();
			middle_vertex->index = 1;
			middle_vertex->point = middle;
			middle_vertex->originof = he->next;

			// Collapsing the half-edge.
			collapse_half_edge(he, middle_vertex);

			// Collapsing the twin if necessary.
			if (he->twin != nullptr)
				collapse_half_edge(he->twin, middle_vertex);

			// Adding new vertex.
			vertices.push_back(middle_vertex);
		}
	}

	cleanValues();
	std::cout << "Collapsed " << count << " half-edges and their twins" << endl;
}

void myMesh::collapse(const int count)
{
	// Iterating through half-edges to collapse.
	for (int i = 0; i < count; i++)
	{
		// Getting the smallest half-edge.
		myHalfedge* he = halfedges.at(0);
		for (auto* h : halfedges)
		{
			if (h->index != -1 && h->length() < he->length())
				he = h;
		}

		// Computing the collapse vertex.
		myPoint3D* middle = he->middle();
		auto* middle_vertex = new myVertex();
		middle_vertex->point = middle;
		middle_vertex->index = 1;

		collapse_half_edge(he, middle_vertex);

		if (he->twin != nullptr)
			collapse_half_edge(he->twin, middle_vertex);
		else
			std::cout << "Warning: twin of collapsed edge is null";

		// Adding new vertex.
		vertices.push_back(middle_vertex);
	}

	cleanValues();
	std::cout << "Collapsed " << count << " half-edges and their twins" << endl;
}

void myMesh::cleanValues()
{
	// Deleting old faces with index -1.
	faces.erase(remove_if(faces.begin(), faces.end(), [&](const myFace* f) {
		const bool res = f->index == -1;
		if (res) delete f;
		f = nullptr;
		return res;
		}), faces.end());

	// Deleting old half-edges with index -1.
	halfedges.erase(remove_if(halfedges.begin(), halfedges.end(), [&](const myHalfedge* he) {
		const bool res = he->index == -1;
		if (res) delete he;
		he = nullptr;
		return res;
		}), halfedges.end());

	// Deleting old vertices with index -1.
	vertices.erase(remove_if(vertices.begin(), vertices.end(), [&](const myVertex* v) {
		const bool res = v->index == -1;
		if (res) delete v;
		v = nullptr;
		return res;
		}), vertices.end());
}

void myMesh::checkValues() const
{
	// Quick checking of half-edges.
	for (const auto* he : halfedges)
	{
		if (!he || he == nullptr || he->index == -1 || !he->source || !he->source->point || !he->next || !he->prev)
		{
			std::cout << "Error: Invalid half-edge:" << endl;
			std::cout << he << endl;
		}
	}
	// Quick checking of vertices.
	for (const auto* v : vertices)
	{
		if (!v || v == nullptr || v->index == -1 || !v->originof || !v->point)
		{
			std::cout << "Error: Invalid vertex." << endl;
		}
	}
	// Quick checking of faces.
	for (const auto* f : faces)
	{
		if (!f || f == nullptr || f->index == -1 || !f->adjacent_halfedge)
		{
			std::cout << "Error: Invalid face." << endl;
		}
	}
}

template <typename T>
int myMesh::validate(std::vector<T> v)
{
	int res = 0;
	for (auto* e : v)
	{
		try 
		{
			if (e == nullptr)
				throw exception("Invalid entry: object is null");
			e->validate(this);
		}
		catch (invalid_mesh_exception e)
		{
			std::cout << "\033[1;31m\t" << e.what() << "\033[0m" << endl;
			++res;
		}
		catch (exception e) {
			std::cout << "\033[1;31m\t" << e.what() << "\033[0m" << endl;
			++res;
		}
	}
	return res;
}

void myMesh::validate_mesh()
{
	// Old functions but let's keep them just in case.
	std::cout << "\033[1;34mCleaning mesh..." << endl;
	cleanValues();
	checkValues();
	std::cout << "Mesh cleaned!" << endl;

	std::cout << "Starting validation..." << endl;
	std::cout << "\tChecking twins..." << endl;
	checkMesh();

	// New in-depth checking functions.
	std::cout << "\tChecking mesh objects..." << endl;
	int res = validate(vertices);
	res += validate(faces);
	res += validate(halfedges);

	if (res == 0)
		std::cout << "Validation succeeded!\033[0m" << endl;
	else
		std::cout << "\033[1;31mValidation failed: " << res << " errors occured.\033[0m" << endl;
}

void register_twins(std::map<std::pair<int, int>, myHalfedge*>* twin_map, myHalfedge* he)
{
	//Finding the boundaries of the current half-edge.
	int p1 = he->source->index;
	int p2 = he->next->source->index;

	// Try to retrieve the current half-edge's twin.
	map<pair<int, int>, myHalfedge*>::iterator it = twin_map->find(make_pair(p2, p1));
	if (it == twin_map->end())
	{
		// This means there was no myHalfedge* present at location(a, b).
		// Then we create an entry in the twin mapping.
		twin_map->insert(pair<pair<int, int>, myHalfedge*>(make_pair(p1, p2), he));
	}
	else
	{
		// It was found. The variable it->second is of type myHalfedge*,
		// and is the half-edge present at location(a, b).
		he->twin = it->second;
		it->second->twin = he;
		twin_map->erase(make_pair(p2, p1));
	}
}

// Implementation of the Catmull-Clark subdivision algorithm.
// Realized with the help of: https://web.cse.ohio-state.edu/~dey.8/course/784/note20.pdf
void myMesh::subdivisionCatmullClark()
{
	// Creating maps of the new mesh's components with their related old mesh components.
	// This will help us compute all the component of the new mesh and to link them.
	std::map<myFace*, std::pair<int, myPoint3D*>> face_points;
	std::map<myHalfedge*, std::pair<int, myPoint3D*>> edge_points;
	std::map < myVertex*, std::pair<int, myPoint3D* >> vertex_points;

	// Incrementing index for our points.
	int idx = 0;

	// First, we create face points.
	for (auto* face : faces)
	{
		// A face point is calculated as the centroid of the current face.
		myPoint3D* p = face->computeCenter();

		// Add the face point to the face points vector.
		face_points.insert(std::pair<myFace*, std::pair<int, myPoint3D*>>(std::make_pair(face, std::make_pair(idx++, p))));
	}

	// Second, we compute the edge points.
	for (auto* he : halfedges)
	{
		// An edge point is calcultated as the average of its boundaries
		// and the face points of its adjacent faces.
		
		// First, we check if we already calculated the point with the current edge's twin.
		if (he->index == -1 || he->twin->index == -1) continue;

		// Taking the first boundary of the edge.
		myPoint3D* p = new myPoint3D(0, 0, 0);
		*p += *(he->source->point);
		
		// Taking the second boundary of the edge.
		*p += *(he->next->source->point);
		
		// Finding the face point of the edge's adjacent face.
		// We recalculate it to be faster.
		*p += *(he->adjacent_face->computeCenter());

		// Finding the face point of the edge's twin's adjacent face.
		// We recalculate it to be faster.
		*p += *(he->twin->adjacent_face->computeCenter());

		// Averaging the value.
		*p /= 4;

		// Inserting the new edge points.
		edge_points.insert(std::pair<myHalfedge*, std::pair<int, myPoint3D*>>(std::make_pair(he, std::make_pair(idx, p))));
		edge_points.insert(std::pair<myHalfedge*, std::pair<int, myPoint3D*>>(std::make_pair(he->twin, std::make_pair(idx++, p))));

		// Marking the twin as already done.
		he->index = -1;
		he->twin->index = -1;
	}

	// Thrid, we compute the vertex points
	for (auto* vertex : vertices)
	{
		// A vertex point is calculated using the neighbouring face's face points,
		// the neighbouring edge's midpoint and itself.
		// With Q the average of the face points, R the average of edge midpoints and n the number of neighbouring edges:
		// v' = (1/n) * Q + (2/n) * R + ((n - 3)/n) * v

		// We need to compute the values of Q and R.
		myPoint3D* Q = new myPoint3D(0, 0, 0);
		myPoint3D* R = new myPoint3D(0, 0, 0);
		myHalfedge* he = vertex->originof;
		int n = 0;

		// We iterate over the edges originating from the current vertex.
		do 
		{
			// Keeping track of the number of values for averaging.
			++n;

			// Finding the corresponding face point.
			myPoint3D* fp = face_points.find(he->adjacent_face)->second.second;

			// Computing the edge midpoint.
			myPoint3D* ep = he->middle();

			// Adding the values.
			*R += *ep;
			*Q += *fp;

			// We iterate over the edges that have the current vertex as a source.
			he = he->twin->next;
		} while (he != vertex->originof);
		
		// Averaging the values.
		*R /= n;
		*Q /= n;

		// Then we apply the weights of the formula.
		myPoint3D* V = new myPoint3D();
		*V += *(vertex->point);

		*R *= 2;
		*V *= (n - static_cast<double>(3));

		// Finally, we apply the formula to get the vertex point.
		myPoint3D* v = new myPoint3D(0, 0, 0);
		*v += *Q + *R + *V;
		*v /= n;

		// Inserting the new vertex point.
		vertex_points.insert(std::pair<myVertex*, std::pair<int, myPoint3D*>>(std::make_pair(vertex, std::make_pair(idx++, v))));
	}

	// After this, we need to link the new objects to create a new mesh.
	// The rules are:
	//     1. link the face points to the edge points
	//     2. link the vertex points to the edge points

	// Temporary vectors for the new mesh.
	std::vector<myFace*> new_faces;
	std::vector<myHalfedge*> new_halfedges;
	std::vector<myVertex*> new_vertices;

	// Mapping of twins by their boundaries.
	std::map<std::pair<int, int>, myHalfedge*> twin_map;

	// We iterate over each face to subdivide them into a new mesh.
	for (auto* face : faces)
	{
		// Creating a vertex for the face point, center of the new faces.
		myVertex* fp = new myVertex();
		fp->index = 1;
		fp->point = face_points.find(face)->second.second;

		// We iterate over the old mesh's halfedges.
		// Each new face is created by linking a face and a vertex point to neighbouring edge points.
		// The resulting mesh is then constitued of quads.
		myHalfedge* he = face->adjacent_halfedge;
		do {
			// New face.
			myFace* f = new myFace();
			f->index = 1;

			// Edges of the new face to be created.
			myHalfedge* h1 = new myHalfedge();
			myHalfedge* h2 = new myHalfedge();
			myHalfedge* h3 = new myHalfedge();
			myHalfedge* h4 = new myHalfedge();

			// Determining the two edge points of the new face.
			std::pair<int, myPoint3D*> e1 = edge_points.find(he)->second;
			std::pair<int, myPoint3D*> e2 = edge_points.find(he->prev)->second;

			myVertex* ep1;
			myVertex* ep2;

			// Determining if vertices were already created for these points.
			auto it = std::find_if(new_vertices.begin(), new_vertices.end(), [e1](const myVertex* v) {
				return v->index == e1.first;
			});

			// It doesn't exist, so we create it.
			if (it == new_vertices.end())
			{
				ep1 = new myVertex();

				ep1->index = e1.first;
				ep1->originof = h2;
				ep1->point = e1.second;

				new_vertices.push_back(ep1);
			}
			// It exists, so we reuse it.
			else {
				ep1 = *it;
			}

			it = std::find_if(new_vertices.begin(), new_vertices.end(), [e2](myVertex* v) {
				return v->index == e2.first;
			});

			// It doesn't exist, so we create it.
			if (it == new_vertices.end())
			{
				ep2 = new myVertex();

				ep2->index = e2.first;
				ep2->originof = h4;
				ep2->point = e2.second;

				new_vertices.push_back(ep2);
			}
			// It exists, so we reuse it.
			else {
				ep2 = *it;
			}

			// Determining the vertex point of the new face.
			std::pair<int, myPoint3D*> v = vertex_points.find(he->source)->second;
			myVertex* vp;

			// Checking if the vertex has already been created.
			it = std::find_if(new_vertices.begin(), new_vertices.end(), [v](myVertex* ve) {
				return ve->index == v.first;
			});

			// It doesn't exist, so we create it.
			if (it == new_vertices.end())
			{
				vp = new myVertex();

				vp->index = v.first;
				vp->originof = h3;
				vp->point = v.second;

				new_vertices.push_back(vp);
			}
			// It exists, so we reuse it.
			else {
				vp = *it;
			}

			// Building the half-edges.
			h1->index = 1;
			h1->source = fp;
			h1->adjacent_face = f;
			f->adjacent_halfedge = h1;
			fp->originof = h1;

			h2->index = 2;
			h2->source = ep1;
			h2->adjacent_face = f;
			
			h3->index = 3;
			h3->source = vp;
			h3->adjacent_face = f;

			h4->index = 4;
			h4->source = ep2;
			h4->adjacent_face = f;

			// Linking the edges together.
			h1->prev = h4;
			h1->next = h2;

			h2->prev = h1;
			h2->next = h3;

			h3->prev = h2;
			h3->next = h4;

			h4->prev = h3;
			h4->next = h1;

			// Registering twins.
			register_twins(&twin_map, h1);
			register_twins(&twin_map, h2);
			register_twins(&twin_map, h3);
			register_twins(&twin_map, h4);

			// Registering the new objects.
			new_halfedges.push_back(h1);
			new_halfedges.push_back(h2);
			new_halfedges.push_back(h3);
			new_halfedges.push_back(h4);

			new_faces.push_back(f);

			// Iterating over the half-edges.
			he = he->prev;
		} while (he != face->adjacent_halfedge);

		new_vertices.push_back(fp);
	}

	// We get rid of the old objects.
	faces.clear();
	halfedges.clear();
	vertices.clear();

	// We put the new objects into the mesh to create the new mesh.
	faces = new_faces;
	halfedges = new_halfedges;
	vertices = new_vertices;

	validate_mesh();
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

			do
			{
				face_hes.push_back(he);
				he = he->next;
			}
			while (he != face->adjacent_halfedge);

			// Variable for new triangle's twin determination.
			myHalfedge* twin = nullptr;
			myHalfedge* first = nullptr;

			// Center point
			myPoint3D* center = face->computeCenter();
			auto* v = new myVertex();
			v->point = center;
			vertices.push_back(v);

			// Looping through half-edges.
			for (int i = 0; i < face_hes.size(); i++)
			{
				// Creating the new face.
				auto* new_face = new myFace();
				new_face->index = 3;
				face_hes.at(i)->adjacent_face = new_face;
				face_hes.at(i)->index = 1;

				// Triangle's first half-edge and vertex.
				auto* h1 = new myHalfedge();

				h1->index = 0;
				h1->source = v;
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

				// Determining newly created half-edges' twins
				if (twin != nullptr)
				{
					h1->twin = twin;
					twin->twin = h1;
				}
				if (first == nullptr) {
					first = h1;
				}

				twin = h3;
			}
			// Determining link between center and face.
			v->originof = first;

			// Determining twin relationship between first and last created triangles.
			if (twin != nullptr)
			{
				first->twin = twin;
				twin->twin = first;
			}
			else
			{
				throw exception("Error: last twin is null after face triangulation");
			}

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

// Return false if already triangle, true otherwise.
bool myMesh::triangulate(myFace* f)
{
	return f->adjacent_halfedge != f->adjacent_halfedge->next->next->next;
}
