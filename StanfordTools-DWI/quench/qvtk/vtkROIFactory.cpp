#include "vtkROIFactory.h"
#include "DTIVolume.h"
#include "MeshBuilder.h"
#include "vtkROI.h"
#include "DTIVolumeIO.h"
#include "ROIManager.h"

PDTIFilterROI vtkROIFactory::DeserializeROI(std::istream &s, int version)
 {
   //   cerr << "Deserializing ROI!" << endl;
   s >> "Name: ";
   char line[2048];
   s.getline(line, 2048);
   std::string name = line;
   
   s.getline(line, 2048); // skip Type, for now. Will add spheres,cubes
   
   Colord color;
   color.Deserialize(s,version);
   s.getline(line, 2048);
   std::string fileLocation = line;
   //   cerr << "file location is: " << fileLocation << endl;

   DTIScalarVolume *v = DTIVolumeIO::loadScalarVolumeNifti(fileLocation.c_str());
   vtkPolyData *pData = MeshBuilder::ImageToMesh(v);
   vtkROI *newMeshROI = vtkROI::CreateMesh(_renderer, _voi_manager->NextID(), Vector3d(0,0,0), pData, fileLocation);
   newMeshROI->SetImageRepresentation(v);
   newMeshROI->SetFileLocation(fileLocation);
   newMeshROI->SetColor(color);
   newMeshROI->SetName(name);

   return PDTIFilterROI(newMeshROI);
   
 }
