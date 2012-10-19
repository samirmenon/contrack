/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef _scene_h
#define _scene_h 

#include <string>
#include <vector>

class vtkCamera;
class vtkImageActor;
class vtkPlane;
class vtkTextActor;
class vtkActor;
class vtkPolyData;
class vtkUnsignedCharArray;
class vtkPoints;
class vtkCellArray;
class vtkMatrix4x4;

const int V_UNIT = 0;
const int W_UNIT = 1;
const int A_UNIT = 2;

const int COLOR_STAIN  = 0;
const int COLOR_LENGTH = 1;
const int COLOR_FA     = 2;

const float _version = 0;

class colorf
{
public:
	float r;
	float g;
	float b;
};

// NOTE ABOUT FUTURE OF CORTEX MESHES
// CortexMesh may just hold static methods for loading/storing meshes, 
// maybe call it mesh i/o maybe convert the data to matlab and build in 
// util. The data space will then just hold onto mesh's by holding onto a vtkPolyData
// container.  The rest of the program must not hold onto the directory of where the
// meshes came from as that is useless.The visual representation of the meshes will be controlled by either a VOI or 
// by some simple scene object class.  Maybe in the scene class there will be a cortical 
// surface object that will point to one of the stored meshes.
class CortexMesh
{
 public:
  CortexMesh();
  ~CortexMesh();

  vtkPolyData *LoadMesh(std::string meshDirectory);
  void LoadCortexRepresentations(std::string meshTopDirectory);

  inline vtkPolyData *GetSmoothMesh() const{return smoothMeshPD;}
  inline vtkPolyData *GetBumpyMesh() const{return bumpyMeshPD;}
  inline void SetSmoothMesh(vtkPolyData* p) { smoothMeshPD = p;}
  inline void SetBumpyMesh(vtkPolyData* p) { bumpyMeshPD = p;}
  //  inline const char* GetSmoothMeshDirectory(){return smoothMeshDirectory->c_str();}
  //  inline const char* GetBumpyMeshDirectory(){return bumpyMeshDirectory->c_str();}
  //inline void SetSmoothMeshDirectory(const char* p){smoothMeshDirectory=p;}
  //inline void SetBumpyMeshDirectory(const char* p){bumpyMeshDirectory=p;}

 private:
  // Mesh information
  vtkPolyData *smoothMeshPD;
  vtkPolyData *bumpyMeshPD;
  //std::string smoothMeshDirectory;
  //std::string bumpyMeshDirectory;
};

class Scene 
{
  //xxx data should be separated from view
  // -> data items should be placed in a new class known as datastorage
  // -> view items can be left in scene

public:
     Scene();
     ~Scene();
     inline CortexMesh* GetCortexMesh(){ if(!_cortex) _cortex=new CortexMesh(); return _cortex; }

     // Volume Info
     std::vector<colorf> backgrounds;	unsigned short curBG;
     //     unsigned int dim[3];
     //     double delta[3];	// voxel size for tensors
     double bVoxSize[3]; // voxel size for background image
     // Sagittal Tomogram Info
     vtkImageActor *sagittal, *axial, *coronal;
     /*     vtkPlane *sPlane;
     vtkPlane *aPlane;
     vtkPlane *cPlane;*/
     //     vtkCamera *camera;
     unsigned int curSSlice, curASlice, curCSlice;	
     static void ConvertCoord(double in[3], int fromUnits, double out[3], int toUnits, const double deltas[3]);

     void NextBackground(float b[3]);

     void ReportPositionFromTomos(vtkMatrix4x4 *mx);
     void NextUnits();
     int curUnits;
     int matrixCode;
     int curPathwayColoring;
     int totalPathways;
     int curShownPathways;
     void NextPathwayColoring();
     static double ACPC_offset[3];
     vtkTextActor* cursorInfo;
     
     char backgroundFilename[255];

     private:
     CortexMesh* _cortex;
};




#endif
