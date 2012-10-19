#ifndef MESH_BUILDER_H
#define MESH_BUILDER_H

// Build a mesh from an image (uses marching cubes)
// Used when pushing ROIs from mrDiffusion to Quench, since ROIs are stored
// as image masks in mrDiffusion, but stored as meshes in Quench.
//
// Input is expected from a network connection (hence the use of CParametersMap)

class vtkPolyData;
class CParametersMap;
class vtkStructuredPoints;

class MeshBuilder
{
 private:
  MeshBuilder();

 public:

  static vtkPolyData *ImageToMesh(CParametersMap &paramsIn, CParametersMap &paramsOut);
  static vtkPolyData *ImageToMesh(DTIScalarVolume *inputVolume);

 protected:
  static vtkPolyData *BuildMesh (vtkStructuredPoints *input);
  static void ReverseTriangles(vtkPolyData *pPD);
};

#endif
