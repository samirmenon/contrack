/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/


#include "vtkConeSource.h"
#include "vtkCone.h"
#include "vtkClipPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkProperty.h"

class ViewCone
{
public:
	ViewCone(double center[3], double direction[3], double height);
	~ViewCone();

	vtkConeSource* viewconeData;
	vtkPolyDataMapper*  viewconeMapper;
	vtkActor* viewconeActor;

	vtkCone* coneFunc;
	vtkClipPolyData* coneClip;

};

