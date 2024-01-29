# Geometric Modelling Unit Project
This a project realised during the geometric modelling class of [ESIEE Paris](https://www.esiee.fr/)'s computer engineering 4th year program.

## Understanding the half-edge data structure
The entire project is based on a data structure to represent 3D structure and geometric shapes: the half-edge.

The half-edge pictures a 3D object by focusing on its edges. Each edge is attributed a twin edge, a source vertex, a neighbour face and a next edge.
To make things easier, we can also define a previous edge as well as an incendent vertex.

![](https://github.com/RaphBhz/Geometric-modeling/assets/74373766/376a4726-8362-498b-b83e-5399e3bc65a3)

Source: Berkley university of California (www.berkeley.edu)

## Implementations
| Algorithm         | Implemented     | Description |
|--------------|-----------|------------|
| ReadFile | ✔️      |         |
| Normal Computation | ✔️      |         |
| Triangulation | ✔️      |         |
| Simplification | ✔️      |         |
| Catmull-Clark subdivision | ✔️      |         |
