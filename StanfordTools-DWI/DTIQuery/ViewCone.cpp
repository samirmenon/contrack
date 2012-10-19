/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "ViewCone.h"

ViewCone::ViewCone(double center[3], double direction[3], double height)
{
	viewconeData = vtkConeSource::New();
	viewconeData->SetCenter(center);
	viewconeData->SetHeight(height);
	viewconeData->SetRadius(50);
	viewconeMapper = vtkPolyDataMapper::New();
	viewconeMapper->SetInput(viewconeData->GetOutput());
	viewconeActor = vtkActor::New();
	viewconeActor->SetMapper(viewconeMapper);
	viewconeActor->GetProperty()->SetOpacity(0.5);
	viewconeActor->GetProperty()->SetDiffuseColor(1, 0, 0);
	viewconeActor->GetProperty()->SetSpecular(.3);
	viewconeActor->GetProperty()->SetSpecularPower(20);

	this->coneFunc = vtkCone::New();
	this->coneFunc->SetAngle(20);
	this->coneClip = vtkClipPolyData::New();
	this->coneClip->SetClipFunction(coneFunc);
}
ViewCone::~ViewCone()
{
}

