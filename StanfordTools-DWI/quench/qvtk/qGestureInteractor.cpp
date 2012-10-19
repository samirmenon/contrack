/***********************************************************************
* AUTHOR: sherbond <sherbond>
*   FILE: qGestureInteractor.cpp
*   DATE: Thu Nov 20 22:40:47 2008
*  DESCR: 
***********************************************************************/
#include "qGestureInteractor.h"

#include "vtkPoints.h"
#include "vtkPolyData.h"
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkMath.h>
#include <vtkRenderWindow.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkMatrix4x4.h>
#include "vtkCellArray.h"
#include <vtkWindowToImageFilter.h>
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkActor2D.h"

#include <wx/wx.h>
#include <GL/glu.h>

#include <DTIMath.h>
#include "RAPIDHelper.h"

using namespace std;
qGestureInteractor::qGestureInteractor(vtkRenderer *renderer)
{
	_renderer = renderer;
	_inProgress = false;
	
	vtkPolyDataMapper2D *mGesture = vtkPolyDataMapper2D::New();
	_pdGesture = vtkPolyData::New();
	mGesture->SetInput(_pdGesture);
	
	_aGesture = vtkActor2D::New();
	_aGesture->SetMapper(mGesture);
	_aGesture->GetProperty()->SetColor( USER_GESTURE_COLOR.r, USER_GESTURE_COLOR.g, USER_GESTURE_COLOR.b );
	_aGesture->GetProperty()->SetLineWidth(4);
	
	vtkPoints *ptsGesture = vtkPoints::New();
	vtkCellArray *cellsGesture = vtkCellArray::New();
	_pdGesture->SetPoints(ptsGesture);
	_pdGesture->SetLines (cellsGesture);
	
	ptsGesture->Delete();
	cellsGesture->Delete();
	mGesture->Delete();
	
	//_renderer->AddActor(_aGesture);
}

qGestureInteractor::~qGestureInteractor()
{
	VTK_SAFE_DELETE(_pdGesture);
	VTK_SAFE_DELETE(_aGesture);
}

void qGestureInteractor::ResetSelect()
{
	vtkPoints *screenPoints = GetGeometry()->GetPoints();
	screenPoints->Reset();
	if(GetGeometry()->GetLines()) GetGeometry()->GetLines()->Reset();
	if(GetGeometry()->GetPolys()) GetGeometry()->GetPolys()->Reset();
}

void qGestureInteractor::BeginSelect(int x, int y)
{
	// return if already in progress
	if (_inProgress) return;
	_inProgress = true;

	// clear the points from last selection
	this->ResetSelect();
	
	// Set actor visibile
	//this->_aGesture->SetVisibility(true);
	
	// add the starting point
	this->AddPoint(x,y);
}

void qGestureInteractor::EndSelect(int x, int y, bool bClosed)
{
	// add last point
	this->AddPoint(x,y);
			
	// end the selection
	_inProgress = false;
	
	// Set actor visibile
	//this->_aGesture->SetVisibility(false);
}

void qGestureInteractor::AddPoint(int x, int y)
{
	// if selection is not going on then return
	if(!_inProgress) return;
	
	//	 Only update screen after a certain distance
	//	double oldPt[3];
	//	screenPoints->GetPoint(screenPoints->GetNumberOfPoints()-1, oldPt);
	//	float oldX = oldPt[0];
	//	float oldY = oldPt[1];
	//	if (fabs(oldX-x) + fabs(oldY-y) <= 5) 
	//		return;
	
	vtkPoints *screenPoints = this->GetGeometry()->GetPoints();
	vtkCellArray *screenCells = this->GetGeometry()->GetLines();
	// insert the points to the gesture selection
	screenPoints->InsertNextPoint (x,y,0.0);
	// insert new line
	if (screenPoints->GetNumberOfPoints() > 2) {
		screenCells->InsertNextCell(2);
		screenCells->InsertCellPoint(screenPoints->GetNumberOfPoints()-2);
		screenCells->InsertCellPoint(screenPoints->GetNumberOfPoints()-1);
		this->GetGeometry()->Modified();
		this->DrawScreenPoints();
	}
	
}

void qGestureInteractor::DrawScreenPoints()
{
	vtkPoints *screenPoints = this->GetGeometry()->GetPoints();
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	int *size = _renderer->GetRenderWindow()->GetSize();
	
	gluOrtho2D (0, size[0], 0, size[1]);
	glPushAttrib (GL_LINE_BIT | GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	
	glDrawBuffer (GL_FRONT);
	glDisable (GL_LIGHTING);
	glDisable (GL_TEXTURE_2D);
	glColor3f (USER_GESTURE_COLOR.r, USER_GESTURE_COLOR.g, USER_GESTURE_COLOR.b);
	glLineWidth (4.0);
	glBegin (GL_LINES);
	
	double pt1[3];
	double pt2[3];  
	screenPoints->GetPoint (screenPoints->GetNumberOfPoints()-1, pt1);
	screenPoints->GetPoint (screenPoints->GetNumberOfPoints()-2, pt2);
	glVertex3dv (pt1);
	glVertex3dv (pt2);
	glEnd();
	glFinish();
	glPopAttrib();
}

/***********************************************************************
 *  Method: qGestureInteractor::PruneGesture2RAPIDModel
 *  Params: 
 * Returns: PRAPID_model
 * Effects: 
 ***********************************************************************/
PRAPID_model qGestureInteractor::PruneGesture2RAPIDModel()
{
	vtkCamera* camera = _renderer->GetActiveCamera();
	vtkPoints* gesture = GetGeometry()->GetPoints();
	
	vtkPoints *trianglePoints = vtkPoints::New();
	vtkCellArray *triangleCells = vtkCellArray::New();
	vtkPolyData *trianglePolyData = vtkPolyData::New();
	trianglePolyData->SetPoints(trianglePoints);
	trianglePolyData->SetPolys (triangleCells);
	vtkImageData *depthImage = NULL;
	vtkWindowToImageFilter *filter = NULL;
	depthImage = ComputeDepthImage(filter);
	int *size = _renderer->GetRenderWindow()->GetSize();
	
	// compute triangle in world space, intersect it with the pathways:
	double eyePoint[4];
	double zNear, zFar;
	double focalPoint[4];
	camera->GetPosition (eyePoint);
	camera->GetClippingRange (zNear, zFar);
	camera->GetFocalPoint (focalPoint);
	double focalVector[3] = {focalPoint[0] - eyePoint[0],
		focalPoint[1] - eyePoint[1],
		focalPoint[2] - eyePoint[2]};
	vtkMath::Normalize(focalVector);
	trianglePoints->InsertNextPoint (eyePoint);
	for (int i = 0; i < gesture->GetNumberOfPoints(); i++) {
		double displayPoint[3];
		double pickPointFar[3];
		int imageOffsets[3];
		gesture->GetPoint(i, displayPoint);
		IntersectSurface (displayPoint, pickPointFar, depthImage, imageOffsets);
		trianglePoints->InsertNextPoint (pickPointFar);
		trianglePoints->Modified();
		if (trianglePoints->GetNumberOfPoints() > 2) {
			triangleCells->InsertNextCell(3);
			triangleCells->InsertCellPoint(0); // eye point
			triangleCells->InsertCellPoint(trianglePoints->GetNumberOfPoints()-2);
			triangleCells->InsertCellPoint(trianglePoints->GetNumberOfPoints()-1);
		}
	}
	triangleCells->Modified();
	
	PRAPID_model p_rmodel = RAPIDHelper::RAPIDModelFromPolyData(trianglePolyData);
	
	// Cleanup
	triangleCells->Delete();
	trianglePoints->Delete();
	filter->Delete();

	return p_rmodel;
}


/***********************************************************************
 *  Method: qGestureInteractor::SurfaceIntersectionGesture2RAPIDModel
 *  Params: 
 * Returns: PRAPID_model
 * Effects: 
 ***********************************************************************/
PRAPID_model qGestureInteractor::SurfaceIntersectionGesture2RAPIDModel()
{
	// test gesture for self-intersection
	vtkPoints* gesture = GetGeometry()->GetPoints();
	vtkPoints *closedGesture = BuildClosedGesture (gesture);
	// look up what bob is doing for cortical surface mesh.
	// otherwise, just use depth value
	vtkImageData *depthImage = NULL;
	vtkWindowToImageFilter *filter;
	depthImage = ComputeDepthImage(filter);
	
	vtkPoints *trianglePoints = vtkPoints::New();
	vtkCellArray *triangleCells = vtkCellArray::New();
	vtkPolyData *trianglePolyData = vtkPolyData::New();
	trianglePolyData->SetPoints(trianglePoints);
	trianglePolyData->SetPolys (triangleCells);
	trianglePoints->Delete();
	triangleCells->Delete();
	
	for (int i = 0; i < closedGesture->GetNumberOfPoints(); i++) {
		double gesturePoint[3];
		closedGesture->GetPoint (i, gesturePoint);
		double pickPoint[3];
		int imageOffsets[3];
		IntersectSurface (gesturePoint, pickPoint, depthImage, imageOffsets);
		trianglePoints->InsertNextPoint (pickPoint);
		if (i >= 2) {
			triangleCells->InsertNextCell(3);
			triangleCells->InsertCellPoint(0); // eye point
			triangleCells->InsertCellPoint(trianglePoints->GetNumberOfPoints()-2);
			triangleCells->InsertCellPoint(trianglePoints->GetNumberOfPoints()-1);
		}
	}
	triangleCells->Modified();
	
	PRAPID_model p_rmodel = RAPIDHelper::RAPIDModelFromPolyData(trianglePolyData);
	
	// Cleanup
	triangleCells->Delete();
	trianglePoints->Delete();
	filter->Delete();
	
	return p_rmodel;
}

/***********************************************************************
 *  Method: qGestureInteractor::BuildClosedGesture
 *  Params: vtkPoints *inputGesture
 * Returns: vtkPoints*
 * Effects: 
 ***********************************************************************/
vtkPoints *
qGestureInteractor::BuildClosedGesture (vtkPoints *inputGesture)
{
	vtkPoints *closedGesture = vtkPoints::New();
	int jIntersect = 0;
	int iIntersect = inputGesture->GetNumberOfPoints()-1;
	
	for (int i = 0; i < inputGesture->GetNumberOfPoints()-1; i++) {
		double pt1[3];
		double pt2[3];
		inputGesture->GetPoint(i, pt1);
		inputGesture->GetPoint(i+1, pt2);
		// now see if this cell intersected any previous cell (earlier segment of line):
		for (int j = 0; j < i-1; j++) {
			double pt3[3];
			double pt4[3];
			inputGesture->GetPoint(j, pt3);
			inputGesture->GetPoint(j+1, pt4);
			if (DTIMath::linesIntersect (pt1, pt2, pt3, pt4)) {
				iIntersect = i;
				jIntersect = j;
				break;
			}
		}
	}
	
	for (int ptIndex = jIntersect; ptIndex < iIntersect; ptIndex++) {
		double pt[3];
		inputGesture->GetPoint (ptIndex, pt);
		closedGesture->InsertNextPoint (pt[0],
										pt[1],
										0.0);
	}
	return closedGesture;
}

vtkImageData * qGestureInteractor::ComputeDepthImage(vtkWindowToImageFilter*& filter)
{
	// render the scene to a depth image.
	// it is assumed that the fibers are already hidden before calling this function.
	// This is done in vtkInteractorStyleQuench.cpp
  vtkRenderWindow* myWindow = _renderer->GetRenderWindow();
  myWindow->SetSwapBuffers (false);
  glDrawBuffer (GL_BACK);
  myWindow->Render();
  
  filter = vtkWindowToImageFilter::New();
  filter->SetInput( myWindow );
  filter->SetInputBufferType (VTK_ZBUFFER);
  filter->SetReadFrontBuffer (false);
  filter->Modified();
  filter->Update();
  vtkImageData *data = filter->GetOutput();
  data->Update();
  data->Modified();
  
  return data;
}

bool qGestureInteractor::IntersectSurface(const double displayPoint[3], double pickPoint[3], vtkImageData *depthImage, int imageOffsets[3])
{
	double *world;
	double z;
	double display[3];
	
	// simply pick up the z value based on the given 2d point
	z = depthImage->GetScalarComponentAsFloat ((int) floor(displayPoint[0]), (int) floor(displayPoint[1]), 0, 0);
	if (z > 1.0) z = 1.0;
	// now convert the display point to world coordinates
	display[0] = displayPoint[0];
	display[1] = displayPoint[1];
	display[2] = z;
	_renderer->SetDisplayPoint (display);
	_renderer->DisplayToWorld ();

	// get the 3d world point
	world = _renderer->GetWorldPoint ();

	// divide by the homogenous coordinate
	for (int i=0; i < 3; i++) 
		pickPoint[i] = world[i] / world[3];

	// intersection occurred if the z buffer had a valid value ( < 1.0 ).
	return (z < 1.0);
}

//PRAPID_model qGestureInteractor::CreateCylinderFromPoints(vtkPoints *points2d, double extrudeBy)
//{
//	vtkSmartPointer<vtkPoints	> trianglePoints  ( vtkPoints	::New() );
//	vtkSmartPointer<vtkCellArray> triangleCells	  ( vtkCellArray::New() );
//	vtkSmartPointer<vtkPolyData	> trianglePolyData( vtkPolyData	::New() );
//
//	trianglePolyData->SetPoints(trianglePoints);
//	trianglePolyData->SetPolys (triangleCells);
//
//	double z[4];
//	_renderer->GetActiveCamera()->GetViewPlaneNormal(z);
//	for (int i = 0; i < points2d->GetNumberOfPoints(); i++) 
//	{
//		double pickPointFar[3];
//		points2d->GetPoint(i, pickPointFar);
//
//		for(int j = 0; j < 3; j++)
//			pickPointFar[j]+=z[j]*extrudeBy;
//		trianglePoints->InsertNextPoint (pickPointFar);
//
//		for(int j = 0; j < 3; j++)
//			pickPointFar[j]-=2*z[j]*extrudeBy;
//		trianglePoints->InsertNextPoint (pickPointFar);
//
//		int numPoints = trianglePoints->GetNumberOfPoints() ;
//		if (numPoints>= 4) 
//		{
//			triangleCells->InsertNextCell(3);
//			triangleCells->InsertCellPoint(numPoints-4);
//			triangleCells->InsertCellPoint(numPoints-3);
//			triangleCells->InsertCellPoint(numPoints-2);
//
//			triangleCells->InsertNextCell(3);
//			triangleCells->InsertCellPoint(numPoints-2);
//			triangleCells->InsertCellPoint(numPoints-3);
//			triangleCells->InsertCellPoint(numPoints-1);
//		}
//	}
//
//	//Create Caps
//	for(int i = 1; i < trianglePoints->GetNumberOfPoints()/2-1; i++)
//	{
//		triangleCells->InsertNextCell(3);
//		triangleCells->InsertCellPoint(0);
//		triangleCells->InsertCellPoint(i*2);
//		triangleCells->InsertCellPoint((i+1)*2);
//
//		triangleCells->InsertNextCell(3);
//		triangleCells->InsertCellPoint(1);
//		triangleCells->InsertCellPoint(i*2+1);
//		triangleCells->InsertCellPoint((i+1)*2+1);
//
//	}
//	return RAPIDHelper::RAPIDModelFromPolyData(trianglePolyData);
//}
