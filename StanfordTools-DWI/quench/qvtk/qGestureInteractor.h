/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef Q_GESTUREINTERACTOR_H
#define Q_GESTUREINTERACTOR_H

#include <typedefs_quench.h>
#include <vtkSmartPointer.h>
#include <RAPID.H>

class vtkPolyData;
class vtkRenderer;
class vtkActor2D;
class vtkPoints;
class vtkWorldPointPicker;
class vtkImageData;
class vtkWindowToImageFilter;
class DTIPathwaySelection;

//! vtk based class to draw user gestures 
/*! The user draw the gesture, which is an arbitary shape, in 2d screen space.
	At the end of the gesture, the shape is extruded in the direction of the camera
	This 3d volume can now be used for ROI based filtering
*/
class qGestureInteractor 
{
public:
	qGestureInteractor(vtkRenderer *renderer);
	~qGestureInteractor();
	
	//! Start drawing of the gesture
	/*! Couldn't use the 3d points because they don't back project in the same place*/
	void BeginSelect(int x, int y);
	
	//! Add a point to the gesture and shows it on screen
 	void AddPoint(int x, int y);
	
	//! Finish the gesture
	void EndSelect(int x, int y, bool bClosed);

	//! The 3d points corresponding to the current gesture
	PROPERTY_READONLY(vtkPoints*, _points2d, Points);
	
	//! Specifies whether a user gesture is currently being drawn
	PROPERTY_READONLY(bool, _inProgress, InProgess);
	
	//! Call this fuction to use the stored gesture as a prune operation and generate a RAPID model from that
	PRAPID_model PruneGesture2RAPIDModel();
	
	//! Call this fuction to use the stored gesture as a surface intersection operation and generate a RAPID model from that
	PRAPID_model SurfaceIntersectionGesture2RAPIDModel();
	
protected:	
	void ResetSelect();
	
	vtkPolyData *GetGeometry(){return _pdGesture;}
	
	vtkPoints* BuildClosedGesture (vtkPoints *inputGesture);
	
	void DrawScreenPoints();

	//! Creates a extruded 'cylinder' using the points. \return RAPID_model from the 3d points. 
	/*! It assumes that the points are in plane */
	//PRAPID_model CreateCylinderFromPoints(vtkPoints *, double extrudeBy=5);

	//! Computes the depth image with fibers hidden for gesture based filtering
	vtkImageData* ComputeDepthImage(vtkWindowToImageFilter*& filter);
	
	bool IntersectSurface(const double displayPoint[3], double pickPoint[3], vtkImageData *depthImage, int imageOffsets[3]);
	
	vtkRenderer* _renderer;
	vtkPolyData *_pdGesture;
	vtkActor2D  *_aGesture;
};

#endif
