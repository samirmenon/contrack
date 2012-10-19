/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "VOILoaded.h"
#include "vtkProperty.h"
#include <util/DTIFilterComplex.h>
#include "vtkCellArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkFilledCaptionActor2D.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "util/typedefs.h"
#include "vtkTransform.h"

VOILoaded::VOILoaded(int id, double c[3], double l[3],const char* meshDirectory) : VOI (id, c)
{
  int i;
  for(i=0;i<3;i++) center[i]=c[i];
  for(i=0;i<3;i++) lengths[i]=l[i];

  int nArrayLength;
  int nVLength;
  int nTLength;
  int nNLength;
  int nCLength;
  float* v;
  float* tris;
  float* normals;
  float* colors;

  std::string vf = meshDirectory;
  std::string tf = meshDirectory;
  // vf += "/vertices_bin";
//   tf += "/triangles_bin";
  vf += "pnts.bin";
  tf += "polys.bin";
// 	  std::string nf = meshDirectory + "/normals_bin";
// 	  std::string cf = meshDirectory + "/colors_bin";

  std::cerr << "Loading mesh..." << std::endl;
  // File I/O
  // For now just make my own;

//   nVLength = 3;
//   nTLength = 1;
//   v = new float[nVLength*3];	// 3 coordinates per point
//   tris = new float[nTLength*3];	// 3 points per triangle
//   v[0] = 0; v[3] = 0; v[6] = 100;
//   v[1] = 20; v[4] = 20;   v[7] = 20;
//   v[2] = 0; v[5] = 100;   v[8] = 0;
//   tris[0] = 0; tris[1] = 1; tris[2] = 2;
	  
 	  v = loadMatrixBinary(nVLength,vf.c_str()); // Vertices
 	  tris = loadMatrixBinary(nTLength,tf.c_str()); // Triangles
// 	  normals = loadMatrixBinary(nNLength,nf.c_str()); // Normals
// 	  colors = loadMatrixBinary(nCLength,cf.c_str()); // Colors
// 	  // End File I/O
 	  std::cerr << "Finished Loading." << std::endl;
  
// 	  // We'll create the building blocks of polydata including data attributes.
	  
 	  meshPD = vtkPolyData::New();
 	  meshPoints = vtkPoints::New();
 	  meshTris = vtkCellArray::New();
// 	  meshScalars = vtkUnsignedCharArray::New();
// 	  meshScalars->SetNumberOfComponents(4);

//           // Lets align the vertices to our space
 	  vtkTransform* align = vtkTransform::New();
// 	  double xformToMV[16];
// // 	  xformToMV[0]=0.0032; xformToMV[1]=-0.0117; xformToMV[2]=1.1999;  xformToMV[3]=1.3205;
// // 	  xformToMV[4]=0.0096; xformToMV[5]=-0.9373; xformToMV[6]=-0.0165; xformToMV[7]=182.1346;
// // 	  xformToMV[8]=-0.9374;xformToMV[9]=-0.0219; xformToMV[10]=0.0013; xformToMV[11]=130.4589;
// // 	  xformToMV[12]=0;     xformToMV[13]=0;      xformToMV[14]=0;      xformToMV[15]=1;	  
// //  	  align->Translate(0,(this->dim[1]*delta[1]/8.),-(this->dim[2]*delta[2]/4.));
// //  	  align->Translate((this->dim[0]*delta[0]/2.),(this->dim[1]*delta[1]/2.),(this->dim[2]*delta[2]/2.));
// //  	  //align->Translate( ACPC_offset[0]*delta[0]/2.,ACPC_offset[1]*delta[1]/2.,ACPC_offset[2]*delta[2]/2.);
// //  	  align->RotateX(-90);
// //  	  align->RotateY(90);	  
// //  	  align->Translate(-(this->dim[0]*delta[0]/2.),-(this->dim[1]*delta[1]/2.),-(this->dim[2]*delta[2]/2.));

// 	  double mrvScale[3] = {1./0.9375,1./0.9375,1./1.2};
// 	  int fudge[3] = {85,120,46};
// 	  // This matrix gets us from MrVista to Bob's DTI space
	  
//    	  align->SetMatrix(xformToMV);
// 	  align->PostMultiply();
// 	  //align->Scale(mrvScale[0],mrvScale[1],mrvScale[2]);
// 	  align->Translate(73,111,46);
// 	  //align->Translate(97.48*mrvScale[1],67.65*mrvScale[0],71.45*mrvScale[2]);
// 	  //align->Translate(ACPC_offset[0]*2,ACPC_offset[1]*2,ACPC_offset[2]*2);
//    	  //align->Inverse();
//   	  // Now all transformations will happen after this matrix
// //   	  align->PostMultiply();
// //   	  // Now we flip coordinates back to VTK
// //   	  //align->Translate(-(this->dim[0]*delta[0]/2.),-(this->dim[1]*delta[1]/2.),-(this->dim[2]*delta[2]/2.));
//   	  //align->Translate(-67.65*mrvScale[0],-97.48*mrvScale[1],-71.45*mrvScale[2]);
// 	  //align->Translate(67.6470,97.4074, 71.3930);
// 	  //align->Translate((this->dim[0]*delta[0]/2.)*0.9375,(this->dim[1]*delta[1]/2.)*0.9375,(this->dim[2]*delta[2]/2.)*0.9375);
// 	  //align->Translate(97.48*mrvScale[1],67.65*mrvScale[0],71.45*mrvScale[2]);
// 	  //    	  align->Scale(mrvScale[0],mrvScale[1],mrvScale[2]);
// 	  //align->RotateZ(-105);
// 	  //align->RotateX(-75);
// 	  //align->Translate(-97.4074, -67.6470, -71.3930);
// //  	  //align->Translate((this->dim[0]*delta[0]/2.),(this->dim[1]*delta[1]/2.),(this->dim[2]*delta[2]/2.));
// //  	  //align->Translate(67.65*mrvScale[0],97.48*mrvScale[1],71.45*mrvScale[2]);
// //  	  align->Translate(fudge[0],fudge[1],fudge[2]);

	  align->Translate(102,90,0);
	  align->RotateZ(180);
	  //align->Scale(1./1.5625,1./1.5625,1./5);

  	  for (i=0; i<nVLength; i++) {
	    float newP[4];
	    float oldP[4];
	    oldP[0]=v[3*i];oldP[1]=v[3*i+1];oldP[2]=v[3*i+2];oldP[3]=1;
	    align->MultiplyPoint(oldP,newP);
	    v[3*i]=newP[0];v[3*i+1]=newP[1];v[3*i+2]=newP[2];

// 	    float newP[3];
// 	    newP[0] = v[3*i]+100;
// 	    newP[1] = v[3*i+1]+100;
// 	    // newP[2] = 420 - v[3*i+2];
// 	     newP[2] = v[3*i+2];
// 	    v[3*i]=newP[0];v[3*i+1]=newP[1]; v[3*i+2]=newP[2];
  	  }

// 	  // Load the point, cell, and data attributes.
  	  for (i=0; i<nVLength; i++) meshPoints->InsertPoint(i,v+i*3);
  	  for (i=0; i<nTLength; i++) {
  	       vtkIdType pts[3];
  	       for(int d=0;d<3;d++) pts[d] = (vtkIdType) *(tris+i*3+d);
  	       meshTris->InsertNextCell(3,pts);
  	  }
//  	  for (i=0; i<nCLength; i++) {
//  	        meshScalars->InsertTuple4(i,colors[i*4+0],
//   				     colors[i*4+1],
//   				     colors[i*4+2],
// 					  255);//colors[i*4+3]);
//  	  }

// 	  // We now assign the pieces to the vtkPolyData.
 	  meshPD->SetPoints(meshPoints);
 	  meshPoints->Delete();
 	  meshPD->SetPolys(meshTris);
 	  meshTris->Delete();
	  delete []v;
	  delete []tris;
// 	  meshPD->GetPointData()->SetScalars(meshScalars);
// 	  meshScalars->Delete();




  mapper = vtkPolyDataMapper::New();
  mapper->SetInput(meshPD);
  meshPD->Delete();
  //  mapper->SetInput(cubeData->GetOutput());
  actor = vtkActor::New();
  actor->SetMapper(mapper);
  actor->GetProperty()->SetOpacity(0.5);
  actor->GetProperty()->SetDiffuseColor(1, 0, 0);
  actor->GetProperty()->SetSpecular(.3);
  actor->GetProperty()->SetSpecularPower(20);
  this->filter = createFilter ();

}

VOILoaded::~VOILoaded() {
  mapper->Delete();
  actor->Delete();
}

void
VOILoaded::ScaleVOI (double scaleAmount) 
{
}


void
VOILoaded::SetScale (const double scale[3]) 
{
}

void
VOILoaded::GetCenter (double c[3]) 
{
  for(int i=0;i<3;i++) c[i]=center[i];
}

DTIFilterVOI* VOILoaded::createFilter()
{
	DTIFilterComplex *theFilter = new DTIFilterComplex(this->meshPD);
 	return theFilter;
}

void VOILoaded::SetCenter(double x, double y, double z)
{
//   double data[3] = {x/delta[0],y/delta[1],z/delta[2]};
//   filter->setTranslation(data);
//   this->cubeData->SetCenter(x,y,z);
//   this->textActor->SetAttachmentPoint(x,y,z);
}

void VOILoaded::GetDimensions (double scale[3]) 
{
  for(int i=0;i<3;i++) scale[i]=lengths[i];
}

void VOILoaded::MakeSymmetricTo (VOI *other, int xDim)
{
}


void VOILoaded::error (const char* p, const char* p2="")
{
     std::cerr<<p<<' '<<p2<<'\n';
     std::exit(1);
}

int VOILoaded::readInt(std::ifstream &theStream){
  int i;
  theStream.read ((char *) &i, sizeof(int));
  return i;
}

float* VOILoaded::loadMatrixBinary(int &length,const char* filename)
{
     int nRow,nCol,i,j;
     std::ifstream file(filename,std::ios::binary); // Open vertices file
     if(!file) VOILoaded::error("cannot open input file",filename);
     // First get size information of file
     // Each vertex vector v is stored in file as [v1] [v2] [v3]
     // where each vertex vector has length 3 and is a column vector
     nRow = VOILoaded::readInt(file);
     nCol = VOILoaded::readInt(file);
     float* v = new float[nCol*nRow];
     file.read((char*) v, sizeof(float) * nRow*nCol);
     file.close();
     length = nCol;
     return v;
}

// I have to put this in the derived classes because of the pure abspathway function 
// declarations in VOI.h
void VOILoaded::PrintSelf(ostream& os) {
  double scale[3];
  this->GetDimensions(scale);
  double center[3];
  this->GetCenter(center);
  os << '\t' << "Type: Loaded" << std::endl;
  os << '\t' << "ID: " << this->getID() << std::endl; 
  os << '\t' << "Center: " << center[0] << "," << center[1] << "," << center[2] << std::endl; 
  os << '\t' << "Scale: " << scale[0] << "," << scale[1] << "," << scale[2] << std::endl; 
}
