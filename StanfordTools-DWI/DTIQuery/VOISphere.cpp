/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "VOISphere.h"
#include "vtkProperty.h"
#include <util/DTIFilterSphere.h>
#include "vtkCellArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkSphereSource.h"
#include "vtkPolyData.h"
#include "vtkFilledCaptionActor2D.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "util/typedefs.h"

VOISphere::VOISphere(int id, double center[3], double length[3]) : VOI (id, center)
{
  actor = vtkActor::New();
  this->filter = createFilter ();
 
  //vtkPolyData *pData = GenerateSphere ();
  mapper = vtkPolyDataMapper::New();
  //	mapper->SetInput(sphereData->GetOutput());
  //  mapper->SetInput(pData);
  //	cerr << "Inserting Sphere VOI with center: " << center[0] << ", " << center[1] << ", " << center[2] << endl;
  //	cerr << "Size: " << length[0] << ", " << length[1] << ", " << length[2] << endl;
 
  actor->SetMapper(mapper);
  actor->GetProperty()->SetOpacity(0.5);
  actor->GetProperty()->SetDiffuseColor(1, 0, 0);
  actor->GetProperty()->SetSpecular(.3);
  actor->GetProperty()->SetSpecularPower(20);
  
  actor->SetPosition (center);

  SetCenter (center[0], center[1], center[2]);
  //  cerr << "length initialized to: " << length[0] << endl;
  SetScale (length);
  
  /*
    actor->SetPosition (center[0], center[1], center[2]);
    actor->SetScale (length);
  */
 

}
#if 0
vtkPolyData *VOISphere::GenerateSphere () {
  vtkSphereSource *sphereData = vtkSphereSource::New();
  //sphereData->SetCenter(center);
  //double centerSphere[3] = {0,0,0};
  //sphereData->SetCenter(centerSphere);
  // xxx
  sphereData->SetRadius(0.5);
  sphereData->SetPhiResolution (8);
  sphereData->SetThetaResolution(8);
  
  sphereData->Update();
  vtkPolyData *data = sphereData->GetOutput();
  
  vtkPoints *points = data->GetPoints();
  
  vtkTransform *scaleAndTranslate = vtkTransform::New();

  //  cerr << "scale: " << _length[0] << ", " << _length[1] << ", " << _length[2] << endl;
  /*
  scaleAndTranslate->Translate (_center[0],
				_center[1],
				_center[2]);*/
  scaleAndTranslate->Scale (_length[0],
			    _length[1],
			    _length[2]);
  
  for (int i = 0; i < points->GetNumberOfPoints(); i++) {
    double pt[4];
    double ptNew[4];
    points->GetPoint(i,pt);
    pt[3] = 1.0;
    scaleAndTranslate->MultiplyPoint (pt, ptNew);
    //cerr << ptNew[0] << ", " << ptNew[1] << ", " << ptNew[2] << endl;
    points->SetPoint(i,ptNew);
  }
  
  
  /*vtkPolyData *pData = vtkPolyData::New();
  pData->SetPoints (points);
  pData->SetPolys (data->GetPolys());*/
  vtkPolyData *pData = data;
  return pData;
}

#endif

VOISphere::~VOISphere() {
  //	sphereData->Delete();
}

void
VOISphere::ScaleVOI (double scaleAmount) 
{
  // double lengthX = sphereData->GetXLength()*scaleAmount;
  //double lengthY = sphereData->GetYLength()*scaleAmount;
  //double lengthZ = sphereData->GetZLength()*scaleAmount;
  // sphereData->SetXLength(lengthX);
  //  sphereData->SetYLength(lengthY);
  //  sphereData->SetZLength(lengthZ);
  
  double scale[3] = {_length[0]*scaleAmount,
		     _length[1]*scaleAmount,
		     _length[2]*scaleAmount};
  SetScale (scale);
}


void
VOISphere::SetScale (const double scale[3]) 
{
  //  actor->SetScale(scale);
  //  cerr << "SetScale called with inputs: " << scale[0] << endl;
  _length[0] = scale[0]; _length[1] = scale[1]; _length[2] = scale[2];

  vtkPolyData *pData = DTIFilterSphere::GenerateEllipsoid (_length);
  ((vtkPolyDataMapper *) actor->GetMapper())->SetInput (pData);
  pData->Modified();
  
  filter->setScale(scale);

 }

void
VOISphere::GetCenter (double center[3]) 
{
  //center[0] = _center[0]; center[1] = _center[1]; center[2] = _center[2];
  this->actor->GetPosition(center);
}

DTIFilterVOI* VOISphere::createFilter()
{
	DTIFilterSphere *theFilter = new DTIFilterSphere(actor);
#if 0
	double center[3];
	this->actor->GetPosition(center);
	theFilter->setTranslation (center);
	//cerr << "center[0]: " << center[0] << endl;
	double scale[3];
	this->actor->GetScale(scale);
	theFilter->setScale (scale);
#endif
	return theFilter;
}

// setPosition - what happens with the filter??

void VOISphere::SetCenter(double x, double y, double z)
{
  _center[0] = x; _center[1] = y; _center[2] = z;
  double data[3] = {x,y,z};
  filter->setTranslation(data);
  this->textActor->SetAttachmentPoint(x,y,z);

  actor->SetPosition (x,y,z);

  /*
  vtkPolyData *pData = GenerateSphere ();
  ((vtkPolyDataMapper *) actor->GetMapper())->SetInput (pData);
  pData->Modified();*/
  
#if 0
	this->actor->SetPosition(x,y,z);
	double data[3] = {x,y,z};
	_filter->setTranslation(data);
	this->textActor->SetAttachmentPoint(x,y,z);
#endif

}

void VOISphere::GetDimensions (double scale[3]) {
  scale[0] = _length[0]; scale[1] = _length[1]; scale[2] = _length[2]; 
}

void VOISphere::MakeSymmetricTo (VOI *other, int xDim)
{
  
	double center[3];
	other->GetCenter(center);
	center[0] = xDim - center[0];
	SetCenter (center[0], center[1], center[2]);
	double scale[3];
	other->GetDimensions(scale);
	actor->SetScale (scale);
}



// I have to put this in the derived classes because of the pure abspathway function 
// declarations in VOI.h
void VOISphere::PrintSelf(ostream& os) {
  double scale[3];
  this->GetDimensions(scale);
  double center[3];
  this->GetCenter(center);
  os << '\t' << "Type: Sphere" << std::endl;
  os << '\t' << "ID: " << this->getID() << std::endl; 
  // Can't do this right now have no idea what label is doing.
//   if(this->GetLabel() != "")
//     os << '\t' << "Label: " << this->GetLabel() << std::endl; 
//   else
//     os << '\t' << "Label: " << emptyStrOut << std::endl;
  os << '\t' << "Center: " << center[0] << "," << center[1] << "," << center[2] << std::endl; 
  os << '\t' << "Scale: " << scale[0] << "," << scale[1] << "," << scale[2] << std::endl; 
}
