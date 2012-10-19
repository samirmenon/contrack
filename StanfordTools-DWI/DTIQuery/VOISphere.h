/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef SPHERE_VOI_H
#define SPHERE_VOI_H

class vtkSphereSource;
class vtkPolyDataMapper;
class vtkActor;
class vtkTriangleFilger;
class DTIFilterVOI;
class DTIFilterSphere;
class vtkTextMapper;
class vtkPolyData;
class vtkFilledCaptionActor2D;

#include "VOI.h"

class VOISphere : public VOI
{
public:
	VOISphere(int id, double center[3], double length[3]);
	~VOISphere();

	virtual void SetScale (const double scales[3]);
	virtual void ScaleVOI (double scale);
	virtual void MakeSymmetricTo (VOI *other, int xDim);
	virtual void GetCenter (double center[3]);
	virtual void GetDimensions (double dim[3]);
	//	vtkSphereSource* sphereData;

	void SetCenter(double x, double y, double z);
	void SetLabel(const char *label);
	//	const char *GetLabel();
	int getID () {return _id; }
	void setSymmetry (VOI *other) { _symmetric_to = other; }
	VOI *getSymmetry () { return _symmetric_to; }
	virtual void PrintSelf(ostream& os);

 protected:
	vtkPolyData *GenerateSphere ();
	virtual DTIFilterVOI* createFilter();
	VOI *_symmetric_to;
	char _label[255];
	int _id;
	double _center[3];
	double _length[3];
};



#endif

