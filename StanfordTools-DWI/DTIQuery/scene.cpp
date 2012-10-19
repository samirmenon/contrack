/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "scene.h"
#include "vtkTextActor.h"
#include <iostream>

#include "vtkPolyData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkStripper.h"
#include "vtkMatrix4x4.h"

//#define STRIP_TRIANGLES

double Scene::ACPC_offset[3] = {0,0,0};


int readIntTony(std::ifstream &theStream){
  int i;
  theStream.read ((char *) &i, sizeof(int));
  return i;
}

// Items that are involved in the scene and should be updated on a regular basis

Scene::Scene()
{	
  //_meshMgr = NULL;
  this->curCSlice = 25; this->curSSlice = 25; this->curASlice = 25;
  //  this->delta[0] = 1.0; this->delta[1] = 1.0; this->delta[2] = 1.0;
  //  this->dim[0] = 50; this->dim[1] = 50; this->dim[2] = 50;
  this->bVoxSize[0] = 2; this->bVoxSize[1] = 2; this->bVoxSize[2] = 2;
  this->curUnits = W_UNIT;
  this->cursorInfo = vtkTextActor::New();
  this->matrixCode = 0;

  colorf c;
  // CREAM
  c.r = 0.74;
  c.g = 0.74;
  c.b = 0.66;
  backgrounds.push_back(c);
  // PURPLE (good for projectors)
  c.r = 0.45;
  c.g = 0.35;
  c.b = 0.35;
  backgrounds.push_back(c);
  // white
  c.r = 1;
  c.g = 1;
  c.b = 1;
  backgrounds.push_back(c);
  // black
  c.r = 0;
  c.g = 0;
  c.b = 0;
  backgrounds.push_back(c);
  curBG=0;
  
  
  curPathwayColoring = COLOR_STAIN;	
  totalPathways = 0;
  curShownPathways = 0;
}
Scene::~Scene()
{
}

void Scene::NextBackground(float b[3])
{
	curBG++;
	if(curBG >= this->backgrounds.size())
		curBG=0;
	b[0] = this->backgrounds[curBG].r;
	b[1] = this->backgrounds[curBG].g;
	b[2] = this->backgrounds[curBG].b;


}


void Scene::NextUnits()
{
  	this->curUnits++;
  	if(curUnits > A_UNIT)
  		curUnits = W_UNIT;
}

void Scene::NextPathwayColoring()
{
	this->curPathwayColoring++;
	if(curPathwayColoring > COLOR_FA)
		curPathwayColoring = COLOR_STAIN;
}

void Scene::ReportPositionFromTomos(vtkMatrix4x4 *transformMx)
{
	double in[3];
	double report[3];
	in[0] = this->curSSlice;
	in[1] = this->curCSlice;
	in[2] = this->curASlice;
	char bufferU[255];
	char bufferC[255];
	char bufferT[255];
	char bufferP[255];

	double percent = (double)curShownPathways*100 / (totalPathways+0.001);
	
	sprintf (bufferU, "%d / %d (%0.1f%%)\n", this->curShownPathways,this->totalPathways, percent);
	
	double cursorPos[4] = {this->curSSlice, this->curCSlice, this->curASlice, 1};
	double transformedPos[4] = {0,0,0,1};
	if (transformMx) {
	  transformMx->MultiplyPoint (cursorPos, transformedPos);
	}

	switch(this->matrixCode)
	{
	case 0:
	  strcpy(bufferC,"Coords: Unknown\n");
	  break;
	case 1:
	  strcpy(bufferC,"Coords: Scanner\n");
	  break;
	case 2:
	  strcpy(bufferC,"Coords: Anat\n");
	  break;
	case 3:
	  strcpy(bufferC,"Coords: ACPC/Talairach\n");
	  break;
	default:
	  strcpy(bufferC,"Coords: Unknown\n");
	  break;
	}

	strcat(bufferU,bufferC);

	sprintf (bufferT, "(%.01lf, %.01lf, %.01lf)\n", transformedPos[0], transformedPos[1], transformedPos[2]);

	sprintf (bufferP, "'%s'", backgroundFilename);

	strcat(bufferT,bufferP);
	strcat(bufferU,bufferT);
	
	//reportString << "(" << report[0] << "," << report[1] << "," << report[2] << ")";
	this->cursorInfo->SetInput(bufferU);
	this->cursorInfo->Modified();
}

void error (const char* p, const char* p2="")
{
     std::cerr<<p<<' '<<p2<<'\n';
     std::exit(1);
}

float* loadMatrixBinary(int &length,const char* filename)
{
     int nRow,nCol;
     std::ifstream file(filename,std::ios::binary); // Open vertices file
     if(!file) error("cannot open input file",filename);
     // First get size information of file
     // Each vertex vector v is stored in file as [v1] [v2] [v3]
     // where each vertex vector has length 3 and is a column vector
     nRow = readIntTony(file);
     nCol = readIntTony(file);
     float* v = new float[nCol*nRow];
     file.read((char*) v, sizeof(float) * nRow*nCol);
     file.close();
     length = nCol;
     return v;
}

#include "vtkSphereSource.h"
#include "vtkTransform.h"

CortexMesh::CortexMesh()
{
  smoothMeshPD = NULL;
  bumpyMeshPD = NULL;
}

CortexMesh::~CortexMesh()
{
}

void CortexMesh::LoadCortexRepresentations(std::string meshTopDirectory)
{
  std::string sd = meshTopDirectory + "/smooth";
  smoothMeshPD = LoadMesh(sd);
  std::string bd = meshTopDirectory + "/bumpy";
  bumpyMeshPD = LoadMesh(bd);

}
 vtkPolyData *CortexMesh::LoadMesh(std::string meshDirectory)
 {
   vtkPoints *meshPoints;
   vtkCellArray *meshTris;
   vtkUnsignedCharArray *meshScalars;
   vtkPolyData *meshPD;

      if(!meshDirectory.empty()) {
 	  int nVLength;
 	  int nTLength;
 	  int nNLength;
 	  int nCLength;
 	  int i;
 	  float* v;
 	  float* tris;
 	  float* normals;
 	  float* colors;

 	  // nVLength = 3;
//  	  nTLength = 1;
//  	  nCLength = 3;
//  	  v = new float[3*3];
//  	  v[0]=0; v[1]=0; v[2]=0;
//  	  v[3]=1; v[4]=0; v[5]=0;
//  	  v[6]=1; v[7]=1; v[8]=0;
//  	  tris = new float[3];
//  	  tris[0]=0; tris[1]=1; tris[2]=2;
//  	  colors = new float[3*4];
//  	  colors[0]=1; colors[1]=0; colors[2]=0; colors[3]=0;
//  	  colors[4]=1; colors[5]=0; colors[6]=0; colors[7]=0;
//  	  colors[8]=1; colors[9]=0; colors[10]=0; colors[11]=0;

  	  std::string vf = meshDirectory + "/vertices_bin";
  	  std::string tf = meshDirectory + "/triangles_bin";
  	  std::string nf = meshDirectory + "/normals_bin";
  	  std::string cf = meshDirectory + "/colors_bin";

  	  std::cerr << "Loading mesh..." << std::endl;
  	  // File I/O
  	  v = loadMatrixBinary(nVLength,vf.c_str()); // Vertices
  	  tris = loadMatrixBinary(nTLength,tf.c_str()); // Triangles
  	  normals = loadMatrixBinary(nNLength,nf.c_str()); // Normals
  	  colors = loadMatrixBinary(nCLength,cf.c_str()); // Colors
  	  // End File I/O
  	  std::cerr << "Finished Loading." << std::endl;
  
 	  // We'll create the building blocks of polydata including data attributes.
	  
 	  meshPD = vtkPolyData::New();
 	  meshPoints = vtkPoints::New();
 	  meshTris = vtkCellArray::New();
 	  meshScalars = vtkUnsignedCharArray::New();
 	  meshScalars->SetNumberOfComponents(4);

            // Lets align the vertices to our space
  	  vtkTransform* align = vtkTransform::New();
  	  double xformToMV[16];

  	  xformToMV[0]=-0.0125; xformToMV[1]=0.0034; xformToMV[2]=0.9999;  xformToMV[3]=-71.6795;
  	  xformToMV[4]=-0.9997; xformToMV[5]=0.0103; xformToMV[6]=-0.0138; xformToMV[7]=71.1346;
  	  xformToMV[8]=-0.0233;xformToMV[9]=-0.9999; xformToMV[10]=0.0011; xformToMV[11]=84.4589;
  	  xformToMV[12]=0;     xformToMV[13]=0;      xformToMV[14]=0;      xformToMV[15]=1;
	  

  	  // This matrix gets us from MrVista to Bob's DTI space
	  
     	  align->SetMatrix(xformToMV);
  	  align->PostMultiply();
  	  align->Translate(73,111,46);

  	  for (i=0; i<nVLength; i++) {
  	       float newP[4];
  	       float oldP[4];
  	       oldP[0]=v[3*i];oldP[1]=v[3*i+1];oldP[2]=v[3*i+2];oldP[3]=1;
  	       align->MultiplyPoint(oldP,newP);
  	       v[3*i]=newP[0];v[3*i+1]=newP[1];v[3*i+2]=newP[2];
  	  }



  	  // Load the point, cell, and data attributes.
   	  for (i=0; i<nVLength; i++) meshPoints->InsertPoint(i,v+i*3);
   	  for (i=0; i<nTLength; i++) {
   	       vtkIdType pts[3];
   	       for(int d=0;d<3;d++) pts[d] = (vtkIdType) *(tris+i*3+d);
   	       meshTris->InsertNextCell(3,pts);
   	  }
   	  for (i=0; i<nCLength; i++) {
   	        meshScalars->InsertTuple4(i,colors[i*4+0],
    				     colors[i*4+1],
    				     colors[i*4+2],
  					  255);//colors[i*4+3]);
   	  }

  	  // We now assign the pieces to the vtkPolyData.
  	  meshPD->SetPoints(meshPoints);
  	  //meshPoints->Delete();
  	  meshPD->SetPolys(meshTris);
  	  //meshTris->Delete();
  	  meshPD->GetPointData()->SetScalars(meshScalars);
  	  //meshScalars->Delete();
 	  return meshPD;
 #if 0
 	  vtkStripper *daveStripper = vtkStripper::New();
 	  daveStripper->SetInput(meshPD);
 	  vtkPolyData *output = daveStripper->GetOutput();
 	  output->SetReferenceCount(555);
 	  cerr << "meshing triangles..." << endl;	
 	  daveStripper->Update();
 	  cerr << "done meshing triangles!" << endl;
 	  daveStripper->Delete();
 	  meshPD->Delete();
 	  return output;
 #endif
	  
	  
      }
      else {
        return NULL;
      }
 }
