/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "VOICube.h"
#include "vtkProperty.h"
#include <util/DTIFilterCube.h>
#include "vtkCellArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkCubeSource.h"
#include "vtkPolyData.h"
#include "vtkFilledCaptionActor2D.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "util/typedefs.h"

VOICube::VOICube(int id, double center[3], double length[3]) : VOI (id, center)
{
	cubeData = vtkCubeSource::New();
	cubeData->SetCenter(center);
	cubeData->SetXLength(length[0]);
	cubeData->SetYLength(length[1]);
	cubeData->SetZLength(length[2]);
	mapper = vtkPolyDataMapper::New();
	mapper->SetInput(cubeData->GetOutput());
	actor = vtkActor::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetOpacity(0.5);
	actor->GetProperty()->SetDiffuseColor(1, 0, 0);
	actor->GetProperty()->SetSpecular(.3);
	actor->GetProperty()->SetSpecularPower(20);
	this->filter = createFilter ();

}

VOICube::~VOICube() {
	cubeData->Delete();
}

void
VOICube::ScaleVOI (double scaleAmount) 
{
  double lengthX = cubeData->GetXLength()*scaleAmount;
  double lengthY = cubeData->GetYLength()*scaleAmount;
  double lengthZ = cubeData->GetZLength()*scaleAmount;
  cubeData->SetXLength(lengthX);
  cubeData->SetYLength(lengthY);
  cubeData->SetZLength(lengthZ);
  double data[3] = {lengthX,lengthY,lengthZ};
  filter->setScale(data);
}


void
VOICube::SetScale (const double scale[3]) 
{
  cubeData->SetXLength(scale[0]);
  cubeData->SetYLength(scale[1]);
  cubeData->SetZLength(scale[2]);
  filter->setScale(scale);
}

void
VOICube::GetCenter (double center[3]) 
{
	cubeData->GetCenter(center);
}

DTIFilterVOI* VOICube::createFilter()
{
	DTIFilterCube *theFilter = new DTIFilterCube();
	double center[3];
	cubeData->GetCenter(center);
	theFilter->setTranslation (center);
	double scale[3];
	scale[0] = cubeData->GetXLength();
	scale[1] = cubeData->GetYLength();
	scale[2] = cubeData->GetZLength();
	theFilter->setScale (scale);
	return theFilter;
}

void VOICube::SetCenter(double x, double y, double z)
{
  double data[3] = {x,y,z};
  filter->setTranslation(data);
  this->cubeData->SetCenter(x,y,z);
  this->textActor->SetAttachmentPoint(x,y,z);
}

void VOICube::GetDimensions (double scale[3]) {
	scale[0] = cubeData->GetXLength();
	scale[1] = cubeData->GetYLength();
	scale[2] = cubeData->GetZLength();
}

void VOICube::MakeSymmetricTo (VOI *other, int xDim)
{
	double center[3];
	other->GetCenter(center);
	center[0] = xDim - center[0];
	SetCenter (center[0], center[1], center[2]);
	double scale[3];
	other->GetDimensions(scale);
	cubeData->SetXLength(scale[0]);
	cubeData->SetYLength(scale[1]);
	cubeData->SetZLength(scale[2]);
}

// I have to put this in the derived classes because of the pure abspathway function 
// declarations in VOI.h
void VOICube::PrintSelf(ostream& os) {
  double scale[3];
  this->GetDimensions(scale);
  double center[3];
  this->GetCenter(center);
  os << '\t' << "Type: Cube" << std::endl;
  os << '\t' << "ID: " << this->getID() << std::endl; 
  // Can't do this right now have no idea what label is doing.
  // if(this->GetLabel() != "")
//     os << '\t' << "Label: " << this->GetLabel() << std::endl; 
//   else
//     os << '\t' << "Label: " << emptyStrOut << std::endl;
  os << '\t' << "Center: " << center[0] << "," << center[1] << "," << center[2] << std::endl; 
  os << '\t' << "Scale: " << scale[0] << "," << scale[1] << "," << scale[2] << std::endl; 
}
