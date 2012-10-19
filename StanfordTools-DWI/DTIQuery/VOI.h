/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef VOI_H
#define VOI_H

class vtkPolyDataMapper;
class vtkActor;
class vtkTriangleFilger;
class DTIFilterVOI;
class vtkTextMapper;
class vtkFilledCaptionActor2D;

#include "vtkSystemIncludes.h"

class VOI
{
public:
	VOI(int id, double center[3]);
	virtual ~VOI();
	
	virtual void GetDimensions(double dim[3]) = 0;
	virtual void SetCenter(double x, double y, double z) = 0;
	virtual void GetCenter (double center[3]) = 0;
	virtual void ScaleVOI (double scaling) = 0;
	virtual void SetScale (const double scales[3]) = 0;
	DTIFilterVOI *GetFilter() { return filter; }

	void SetLabel(const char *label);
	const char *GetLabel();
	int getID () {return _id; }
	void setSymmetry (VOI *other) { _symmetric_to = other; }
	VOI *getSymmetry () { return _symmetric_to; }

	vtkActor *getActor() { return actor; }
	vtkActor *GetOverlayActor() { return overlayActor; }
	vtkActor *GetGhostActor() {return ghostActor; }
	vtkFilledCaptionActor2D *getTextActor() { return textActor; }
	virtual void MakeSymmetricTo (VOI *other, int xDim) = 0;

	void SetOverlayActor (vtkActor *actor) { this->overlayActor = actor; }
	void SetGhostActor (vtkActor *actor) { this->ghostActor = actor; }

	void SetSurfaceCellID (vtkIdType id) {cellID = id; }
	vtkIdType GetSurfaceCellID() { return cellID; }
	virtual void PrintSelf(ostream &os) = 0;

protected:

	virtual DTIFilterVOI* createFilter() = 0;
       	
	// Labelling
	vtkFilledCaptionActor2D*  textActor;

	// vtk
	vtkPolyDataMapper* mapper;
	vtkActor* actor;
	vtkActor* overlayActor;
		vtkActor* ghostActor;

	// RAPID
	DTIFilterVOI* filter;

	// misc
	int cellID;
	VOI *_symmetric_to;
	char _label[255];
	int _id;
};

#endif

