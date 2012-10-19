/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef MESH_ROI_H
#define MESH_ROI_H

class vtkPolyDataMapper;
class vtkActor;
class DTIFilterROI;
class vtkTextMapper;
class vtkFilledCaptionActor2D;

class vtkPoints;
class vtkCellArray;

#include "typedefs_quench.h"
#include "DTIFilterROI.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"

typedef vtkSmartPointer<vtkPolyData> PvtkPolyData;

//! This class stores the 3d polygon data of a ROI in vtk format
class vtkROI : public DTIFilterROI
{
public:
  vtkROI(ROIType t, vtkRenderer *r, int id, Vector3d pos, PvtkPolyData pData, std::string name, Colord col = DEFAULT_ROI_COLOR);
	~vtkROI();

	//! Creates a cube shaped ROI
	static vtkROI* CreateCube(vtkRenderer *r, int id, Vector3d pos, Vector3d scale=Vector3d(5,5,5), std::string name = "", Colord col = DEFAULT_ROI_COLOR);
	//! Creates a sphere shaped ROI
	static vtkROI* CreateSphere(vtkRenderer *r, int id, Vector3d pos, double radius = 5, int resolution = 20, std::string name = "", Colord col = DEFAULT_ROI_COLOR);
	//! Creates a mesh ROI (arbitrary mesh shape)
	static vtkROI* CreateMesh(vtkRenderer *r, int id, Vector3d pos, vtkPolyData *polyData, std::string name = "", Colord col = DEFAULT_ROI_COLOR);

	//! Sets the color of the ROI
	void SetColor (Colord &col);

	//! Sets the position of the ROI
	void SetPosition (const double T[3]);
	//! Sets the scale of the ROI, this requires adjusting the associated RAPID model 
	void SetScale (const double s[3]);
	
	//! Sets the name of the voi
	void SetName(std::string &name);
	
	void SetShowROI(bool b);
	void SetShowLabel(bool b);

	PROPERTY_DECLARE(bool, bMeshVisible, MeshVisible);
	PROPERTY_DECLARE(bool, bLabelVisible, LabelVisible);

	//! \return the visiblity of the voi
	//	PROPERTY_DECLARE(bool, bVisible, Visible);
	//	PROPERTY_DECLARE(bool, bShowAll, ShowAll);
	//! \return whether the label is shown
	//	PROPERTY_DECLARE(bool, bShowLabel, ShowLabel);
	//! returns the actor associated with the voi
	PROPERTY_READONLY(vtkActor *, _actor, Actor);

protected:

	void UpdateVisibility();

	vtkPolyData *_mesh_pd;					/// The mesh which stores the voi polygon data
	vtkFilledCaptionActor2D*  _textActor;	/// The 3d text actor which shows the name of the voi
	vtkPolyDataMapper* _mapper;				/// vtk poly mapper
	vtkRenderer *_renderer;					/// pointer to the global vtk renderer
};

#endif

