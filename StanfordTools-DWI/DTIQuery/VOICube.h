/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef CUBE_VOI_H
#define CUBE_VOI_H

class vtkCubeSource;
class vtkPolyDataMapper;
class vtkActor;
class DTIFilterVOI;
class DTIFilterCube;
class vtkTextMapper;
class vtkFilledCaptionActor2D;

#include "VOI.h"

class VOICube : public VOI
{
public:
	VOICube(int id, double center[3], double length[3]);
	~VOICube();

	virtual void ScaleVOI (double scalingAmount);
	virtual void SetScale (const double scales[3]);
	virtual void MakeSymmetricTo (VOI *other, int xDim);
	virtual void GetCenter (double center[3]);
	virtual void GetDimensions (double dim[3]);
	vtkCubeSource* cubeData;

	void SetCenter(double x, double y, double z);
	void SetLabel(const char *label);
	//	const char *GetLabel();
	int getID () {return _id; }
	void setSymmetry (VOI *other) { _symmetric_to = other; }
	VOI *getSymmetry () { return _symmetric_to; }
	virtual void PrintSelf(ostream &os);

 protected:
	virtual DTIFilterVOI* createFilter();
	VOI *_symmetric_to;
	char _label[255];
	int _id;
};

#endif

