/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef LOADED_VOI_H
#define LOADED_VOI_H

class vtkPolyDataMapper;
class vtkActor;
class DTIFilterVOI;
class DTIFilterCube;
class vtkTextMapper;
class vtkFilledCaptionActor2D;

class vtkPoints;
class vtkCellArray;
class vtkPolyData;

#include "VOI.h"
#include <iosfwd>

class VOILoaded : public VOI
{
public:
	VOILoaded(int id, double center[3], double length[3], const char* filename);
	~VOILoaded();

	virtual void ScaleVOI (double scalingAmount);
	virtual void SetScale (const double scales[3]);
	virtual void MakeSymmetricTo (VOI *other, int xDim);
	virtual void GetCenter (double center[3]);
	virtual void GetDimensions (double dim[3]);

	void SetCenter(double x, double y, double z);
	void SetLabel(const char *label);
	//	const char *GetLabel();
	int getID () {return _id; }
	void setSymmetry (VOI *other) { _symmetric_to = other; }
	VOI *getSymmetry () { return _symmetric_to; }
	virtual void PrintSelf(ostream &os);

  float* loadMatrixBinary(int &length,const char* filename);
  static int readInt(std::ifstream &theStream);
  static void error (const char* p, const char* p2);

 protected:
  virtual DTIFilterVOI* createFilter();
  VOI *_symmetric_to;
  char _label[255];
  int _id;
  
  double center[3];
  double lengths[3];
  vtkPoints *meshPoints;
  vtkCellArray *meshTris;
  vtkPolyData *meshPD;
};

#endif

