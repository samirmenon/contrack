/***********************************************************************
* AUTHOR: Anthony Sherbondy
*   FILE: qVolumeViz.cpp
*   DATE: Fri Oct 24 07:53:49 2008
*  DESCR: 
***********************************************************************/
#include "qVolumeViz.h"
#include "vtkImageData.h"
#include "vtkImageActor.h"
#include "vtkWindowLevelLookupTable.h"
#include "vtkColorTransferFunction.h"
#include "vtkMatrix4x4.h"
#include "vtkImageMapToColors.h"
#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkTextActor.h"
#include "vtkRenderer.h"
#include <io_utils.h>
#include "vtkCamera.h"
//#include "Quench.h"
#include "vtkRenderWindow.h"
#include "vtkPropPicker.h"
#include "vtkPropCollection.h"
#include <vtkAssemblyPath.h>


qVolumeViz::qVolumeViz(vtkRenderer *renderer)
{
	_img=0;
	_vol=NULL;
	_pdBorder=NULL;
	_mBorder=NULL;
	_nActiveImage=(DTISceneActorID)-1;

	// create a look up table to adjust the brightness/contrast of the background image
	_lutBW = vtkWindowLevelLookupTable::New();
	_lutColor = vtkColorTransferFunction::New();
	_lutColor->SetColorSpaceToRGB();
	_lutColor->AddRGBSegment  	(0,
		0,0,0,
		1,
		1,1,1);
	_lutBW->SetValueRange (0, 1);
	_lutBW->SetHueRange (0, 1);
	_lutBW->SetAlphaRange (0, 1);

	// create the 3 actors for the 3 image planes
	_aSag   = vtkImageActor::New();
	_aAxial = vtkImageActor::New();
	_aCor   = vtkImageActor::New();
	_aBorder = vtkActor::New();
	_aPosition = vtkTextActor::New();


	_pdBorder = vtkPolyData::New();
	vtkPoints *pts = vtkPoints::New();
	pts->SetNumberOfPoints(4);
	_pdBorder->SetPoints(pts);
	pts->Delete();
	// create a border actor used to highlight a particular image plane
	vtkCellArray *border = vtkCellArray::New();
	border->InsertNextCell(5);
	border->InsertCellPoint(0);
	border->InsertCellPoint(1);
	border->InsertCellPoint(2);
	border->InsertCellPoint(3);
	border->InsertCellPoint(0);
	_pdBorder->SetLines(border);  
	border->Delete();
	_mBorder = vtkPolyDataMapper::New();
	_mBorder->SetInput(_pdBorder);
	_aBorder->SetMapper(_mBorder);
	_aBorder->GetProperty()->SetColor(1,0,0);
	_aBorder->GetProperty()->SetLineWidth(5);
	_aBorder->SetVisibility(0); //Don't show it yet

	// add actors to renderer
	_renderer = renderer;
	_renderer->AddActor(_aBorder);
	_renderer->AddActor(_aSag);
	_renderer->AddActor(_aCor);
	_renderer->AddActor(_aAxial);
	_renderer->AddActor(_aPosition);

	// used to see if we clicked on a image plane
	_propPicker = vtkPropPicker::New();
	_propCollection = vtkPropCollection::New();

	// add the image planes and border to the actor picker
	_propCollection->AddItem(_aAxial);
	_propCollection->AddItem(_aSag);
	_propCollection->AddItem(_aCor);
	_propCollection->AddItem(_aBorder);

}
qVolumeViz::~qVolumeViz()
{
	// release memory
	_overlays.clear();
	VTK_SAFE_DELETE(_aPosition);
	VTK_SAFE_DELETE(_aSag);
	VTK_SAFE_DELETE(_aAxial);
	VTK_SAFE_DELETE(_aCor);
	VTK_SAFE_DELETE(_img);
	VTK_SAFE_DELETE(_lutBW);
	VTK_SAFE_DELETE(_lutColor);
	VTK_SAFE_DELETE(_aBorder);
	VTK_SAFE_DELETE(_pdBorder);
	VTK_SAFE_DELETE(_mBorder);
	VTK_SAFE_DELETE(_propPicker);
	VTK_SAFE_DELETE(_propCollection);
}
void qVolumeViz::AddVolume(DTIScalarVolume *vol, ColorMapPanel* cmpanel)
{
	// add a corresponding overlay to the image
	Overlay *o = new Overlay(_renderer, vol, 0, 1, 0, cmpanel);
	POverlay overlay( o );
	_overlays.push_back(overlay);
	
	// compute the range of left and right values
	float left,right,min,max;
	o->Range(left,right,min,max);

	// set the volume
	SetVolume(vol, left, right);
}
//Set the vtk volume for display
void qVolumeViz::SetVolume(DTIScalarVolume *vol, float left, float right)
{
	//Volume is being set for the first time so display the border
	if(!_vol)
		_aBorder->SetVisibility(1);

	const DTIScalarVolume* oldVol = _vol;
	_vol = vol;

	// use the brightness and contrast by using the left and right variables
	_lutBW->SetTableRange (left, right);
	double window = right - left;
	double level = left + window/2.0;
	// adjust the range
	_lutBW->SetWindow(window);
	// adjust the midpoint
	_lutBW->SetLevel(level);
	_lutBW->Modified();

	// set the transformation matrix of the 3 image actors to that of the background volume
	vtkMatrix4x4 *mx = vtkMatrix4x4::New();
	mx->DeepCopy (_vol->getTransformMatrix());
	_aSag->SetUserMatrix (mx);
	_aCor->SetUserMatrix (mx);
	_aAxial->SetUserMatrix (mx);
	mx->Delete();

	uint dim[4];
	double voxSize[3];
	_vol->getDimension(dim[0], dim[1], dim[2], dim[3]);
	_vol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);
	
	VTK_SAFE_DELETE(_img);
	_img = vtkImageData::New();
	_img->SetScalarTypeToFloat();
	_img->SetNumberOfScalarComponents(dim[3]); 
	_img->SetDimensions (dim[0], dim[1], dim[2]);
	_img->SetSpacing (1,1,1);
	if (dim[3] == 1) 
		_img->SetScalarTypeToFloat();
	else 
		_img->SetScalarTypeToUnsignedChar();
	_img->AllocateScalars();

	vtkScalarsToColors *lut = _lutBW;
	if (dim[3] == 1) // more common case
	{
		// create the sagital actor
		vtkImageMapToColors *sagittalColors = vtkImageMapToColors::New();
		sagittalColors->SetInput(_img);
		sagittalColors->SetLookupTable(lut);
		_aSag->SetInput(sagittalColors->GetOutput());
		sagittalColors->Delete();

		// create the axial actor
		vtkImageMapToColors *axialColors = vtkImageMapToColors::New();
		axialColors->SetInput(_img);
		axialColors->SetLookupTable(lut);
		_aAxial->SetInput(axialColors->GetOutput());
		axialColors->Delete();

		// create the coronal actor
		vtkImageMapToColors *coronalColors = vtkImageMapToColors::New();
		coronalColors->SetInput(_img);
		coronalColors->SetLookupTable(lut);
		_aCor->SetInput(coronalColors->GetOutput());
		coronalColors->Delete();
	}
	else {
		_aSag->SetInput(_img);
		_aAxial->SetInput(_img);
		_aCor->SetInput(_img);
	}

	if (oldVol == NULL) 
	{
		// Start in center of volume
		double localPos[4]={dim[0]/2, dim[1]/2, dim[2]/2, 1}; 
		LocalToWorld(_vol->getTransformMatrix(), localPos, _vPos);
	}
	// Find the local pos based on world pos
	DTIVoxel lPos = DTIVoxel(3);
	WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);

	_img->Modified();

	// set the extent to which the image should be mapped
	_aSag->SetDisplayExtent(lPos[0],lPos[0], 0, dim[1]-1, 0, dim[2]-1);
	_aAxial->SetDisplayExtent(0,dim[0]-1, 0,dim[1]-1, lPos[2],lPos[2]);
	_aCor->SetDisplayExtent(0,dim[0]-1, lPos[1],lPos[1], 0,dim[2]-1);

	if (dim[3] == 1) // more common case
	{
		// copy the background image data to a cache(_img)
		float *dataPtr = _vol->getDataPointer();
		float *destinationPtr = static_cast<float *>(_img->GetScalarPointer());  
		memcpy (destinationPtr, dataPtr, dim[0]*dim[1]*dim[2]*dim[3]*sizeof(float));
	}
	else {
		for(uint k=0;k<dim[2];k++) {
			for(uint j=0;j<dim[1];j++) {
				for(uint i=0;i<dim[0];i++){
					for (uint c = 0; c<dim[3];c++) {
						float f = _vol->getScalar(i,j,k,c)*255;
						_img->SetScalarComponentFromFloat (i,j,k,c,f);
					}
				}
			}
		}
	}
	_aAxial->Modified();
	_aCor->Modified();
	_aSag->Modified();

	// set the position in the _aPosition text actor
	_aPosition->GetTextProperty()->SetFontSize(16);
	_aPosition->GetTextProperty()->SetFontFamilyToArial();
	_aPosition->GetTextProperty()->SetJustificationToLeft();
	_aPosition->GetPositionCoordinate()->SetValue(2,0);
	//_aPosition->GetTextProperty()->BoldOn();
	//_aPosition->GetTextProperty()->ItalicOn();
	_aPosition->GetTextProperty()->SetColor(0,0,0);
	_aPosition->Modified();
	SetPosition(_vPos);

	// check to see if this is the very first image that has been loaded. 
	// if so set the camera to a default position and orientation
	if(_overlays.size() == 1 && _nActiveImage == -1)
	{
		vtkCamera *camera = _renderer->GetActiveCamera();
	    vtkMatrix4x4 *mx = _aCor->GetUserMatrix();
		unsigned int dim[3];
		vol->getDimension(dim[0], dim[1], dim[2]);
		const double imageCenter[4] = {dim[0]/2.0, dim[1]/2.0, dim[2] / 2.0, 1};
		const double camP[4] = {dim[0]/2.0, 4*dim[1], dim[2] / 2.0, 1};
		double transformedCenter[4];
		double transformedCamP[4];

		mx->MultiplyPoint(imageCenter, transformedCenter);
		mx->MultiplyPoint(camP, transformedCamP);

	  	camera->SetViewUp (0, 0, 1);
		camera->SetPosition (transformedCamP[0], transformedCamP[1],  transformedCamP[2]);
		camera->SetFocalPoint (transformedCenter[0],transformedCenter[1],transformedCenter[2]);
		camera->ComputeViewPlaneNormal();
		camera->Modified();

		DTISceneActorID n=DTI_ACTOR_CORONAL_TOMO;
		SetActiveImage(n);
	}
}

///Display border around the selected image plane
void qVolumeViz::DisplayBorder()
{
	if(_nActiveImage>=0 && _nActiveImage<3) 
	{
		// Position the slice
		double voxSize[3];
		uint dim[4];
		_vol->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
		_vol->getDimension (dim[0], dim[1], dim[2], dim[3]);
		DTIVoxel lPos = DTIVoxel(3);
		WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);
		double p[3]={lPos[0], lPos[1], lPos[2]}, m[3] = {dim[0]-1, dim[1]-1, dim[2]-1};

		_aBorder->SetUserMatrix (_aCor->GetUserMatrix());
		vtkPoints *pts = _pdBorder->GetPoints();

		// specify the 4 corners of the border depending on which image plane has been selected
		switch(_nActiveImage) 
		{
		case DTI_ACTOR_SAGITTAL_TOMO:
			pts->SetPoint(0, p[0],0,0);
			pts->SetPoint(1, p[0],0,m[2]);
			pts->SetPoint(2, p[0],m[1],m[2]);
			pts->SetPoint(3, p[0],m[1],0);
			break;
		case DTI_ACTOR_AXIAL_TOMO:
			pts->SetPoint(0, 0,0,p[2]);
			pts->SetPoint(1, m[0],0,p[2]);
			pts->SetPoint(2, m[0],m[1],p[2]);
			pts->SetPoint(3, 0,m[1],p[2]);
			break;
		case DTI_ACTOR_CORONAL_TOMO:
			pts->SetPoint(0, 0,p[1],0);
			pts->SetPoint(1, m[0],p[1],0);
			pts->SetPoint(2, m[0],p[1],m[2]);
			pts->SetPoint(3, 0,p[1],m[2]);
			break;
		}
		_pdBorder->Modified();
		_aBorder->SetVisibility(true);
		_aBorder->Modified();
	} 
	else 
		_aBorder->SetVisibility(false);
}

void qVolumeViz::SetPosition(Vector3d &v)
{
	uint dim[4];
	double voxSize[3];
	_vol->getDimension(dim[0], dim[1], dim[2], dim[3]);
	_vol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);
	DTIVoxel lPos = DTIVoxel(3);
	WorldToLocal(_vol->getTransformMatrix(), v, dim, lPos);

	// do a lazy update on the image planes that are mapped to the image actors
	if(v[0] != _vPos[0]) 
	{
		_vPos[0] = v[0];
		// Update Sagittal position
		_aSag->SetDisplayExtent(lPos[0],lPos[0], 0,dim[1]-1, 0,dim[2]-1);
		_aSag->Modified();
	}

	if(v[1] != _vPos[1]) 
	{
		_vPos[1] = v[1];
		// Update Coronal position
		_aCor->SetDisplayExtent(0,dim[0]-1, lPos[1],lPos[1], 0,dim[2]-1);
		_aCor->Modified();
	}

	if(v[2] != _vPos[2]) 
	{
		_vPos[2] = v[2];
		// Update Axial position
		_aAxial->SetDisplayExtent(0,dim[0]-1, 0,dim[1]-1, lPos[2],lPos[2]);
		_aAxial->Modified();
	}

	double flPos[3] = {lPos[0], lPos[1], lPos[2]};
	for(int i=0; i<3; i++) { flPos[i]*=voxSize[i]; } 
	for(int i = 0; i < (int)_overlays.size(); i++)
		_overlays[i]->SetPosition(_vPos, flPos);

	DisplayBorder();  
	
	// update the text of the position actor to show the new position in ACPC space
	char spos[100]; sprintf(spos, "Position: %.1f, %.1f, %.1f",_vPos[0],_vPos[1],_vPos[2]);
	_aPosition->SetInput(spos);
	//for(unsigned i = 0; i < _overlays.size(); i++)
	//	_overlays[i]->SetPosition(v, lPos);
}

DTIScalarVolume* qVolumeViz::GetActiveVolume()
{
  return _vol;
}

//Move the active image slice by the specified amount
void qVolumeViz::MoveActiveImage(int amount)
{
	uint dim[4];
	_vol->getDimension(dim[0], dim[1], dim[2], dim[3]);
	DTIVoxel lPos = DTIVoxel(3);
	WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);

	DTISceneActorID i = ActiveImage();
	if(i >=0 && i < 3) 
	{
		lPos[i] += amount;
		// see if the new position exceeds the dimensions
		// since both are unsigned, a value of -1 will be mapped to 0xfffffff which is greater than dim[i]
		if(lPos[i] >= dim[i])
			lPos[i] = (amount > 0) ? dim[i]-1 : 0;

		Vector3d vPos;
		LocalToWorld(_vol->getTransformMatrix(), lPos, vPos.v);
		SetPosition(vPos);
		
		//Make image visible if hidden
		if(!Visibility(i))
			SetVisibility(i,true);
		_renderer->GetRenderWindow()->Render();
	}
}
void qVolumeViz::SetActiveImage(DTISceneActorID &nType)
{
	_nActiveImage = nType;
	// Make sure active image's actor is visible
	switch(_nActiveImage) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		_aSag->SetVisibility(true);
		_aSag->Modified();
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		_aAxial->SetVisibility(true);
		_aAxial->Modified();
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		_aCor->SetVisibility(true);
		_aCor->Modified();
		break;
	};
	// show the overlays if they were previously hidden
	for(unsigned i = 0; i < _overlays.size(); i++)
		_overlays[i]->SetVisible(_nActiveImage,true);
	// Show border around active image
	DisplayBorder();
}
void qVolumeViz::GetWindowLevelParams(double &dW, double &dWMin, double &dWMax, double &dL, double &dLMin, double &dLMax)
{
	// unused, will be removed
	dL = _lutBW->GetLevel();
	dLMin = _vol->getCalcMinVal();
	dLMax = _vol->getCalcMaxVal();
	dW = _lutBW->GetWindow();
	dWMin = 0;
	dWMax = dLMax - dLMin;
}


void qVolumeViz::GetPlaneEquation (DTISceneActorID id, double pt[4], double normal[4]) 
{
	// unused, will be removed
	double untransformedNormal[4]={0,0,0,0};
	double untransformedPoint[4]={0,0,0,1};

	untransformedPoint[id] = _vPos[0];
	untransformedNormal[id] = 1.0;

	vtkMatrix4x4 *mx = _aCor->GetUserMatrix();
	mx->MultiplyPoint (untransformedPoint, pt);

	vtkMatrix4x4 *invertedMx = vtkMatrix4x4::New();
	mx->Invert (mx, invertedMx);
	vtkMatrix4x4 *transposedMx = vtkMatrix4x4::New();
	mx->Transpose (invertedMx, transposedMx);
	mx->MultiplyPoint (untransformedNormal, normal);
	transposedMx->Delete();
	invertedMx->Delete();
}

void qVolumeViz::ActiveImageExtents(double transPts[4][3], double transNormal[4])
{
	// get the active image extents

	double pts[4][3];
	int displayExtent[6];
	double planeNormal[4]={0,0,0,0};
	uint dim[4];
	// get the image dimensions
	_vol->getDimension (dim[0], dim[1], dim[2], dim[3]);
	DTIVoxel lPos = DTIVoxel(3);
	// map position from ACPC to 3d world space
	WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);
	
	memset(pts, 0, sizeof(double)*4*3);

	int i = ActiveImage();
	GetDisplayExtent ((DTISceneActorID)i,displayExtent);
	for (int j = 0; j < 4; j++)
		pts[j][i] = lPos[i];
	planeNormal[i] = 1;
	
	// get the 4 corners depending on which image plane is selected
	switch (ActiveImage()) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		pts[1][1] = displayExtent[3];
		pts[2][1] = displayExtent[3];
		pts[2][2] = displayExtent[5];
		pts[3][2] = displayExtent[5];
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		pts[1][0] = displayExtent[1];
		pts[2][0] = displayExtent[1];
		pts[2][2] = displayExtent[5];
		pts[3][2] = displayExtent[5];
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		pts[1][0] = displayExtent[1];
		pts[2][0] = displayExtent[1];
		pts[2][1] = displayExtent[3];
		pts[3][1] = displayExtent[3];
		break;
	default:
		break;
	};
	vtkMatrix4x4 *mx = GetUserMatrix();
	// map the points from 3d world space to ACPC space
	for (int i = 0; i < 4; i++) 
	{
		double fooIn[4] = {pts[i][0], pts[i][1], pts[i][2], 1};
		double fooOut[4];
		mx->MultiplyPoint (fooIn, fooOut);
		transPts[i][0] = fooOut[0];
		transPts[i][1] = fooOut[1];
		transPts[i][2] = fooOut[2];
	}
	// inverse of transpose:
	vtkMatrix4x4 *invertedMx = vtkMatrix4x4::New();
	vtkMatrix4x4 *transposedMx = vtkMatrix4x4::New();
	mx->Invert (mx, invertedMx);
	mx->Transpose (invertedMx, transposedMx);
	mx->MultiplyPoint (planeNormal, transNormal);
	transposedMx->Delete();
	invertedMx->Delete();
}
void qVolumeViz::GetDisplayExtent (DTISceneActorID id, int displayExtent[6])
{
	// get the display extent of a specific image plane
	switch (id) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		_aSag->GetDisplayExtent(displayExtent);
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		_aCor->GetDisplayExtent(displayExtent);
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		_aAxial->GetDisplayExtent(displayExtent);
		break;
	default:
		break;
	};
}

bool qVolumeViz::Visibility (DTISceneActorID id)
{
	// get the visibility of the selected image plane
	bool bVis=false;
	switch (id) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		bVis = _aSag->GetVisibility()?true:false;
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		bVis = _aCor->GetVisibility()?true:false;
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		bVis = _aAxial->GetVisibility()?true:false;
		break;
	default:
		break;
	};
	return bVis;
}

void qVolumeViz::SetVisibility (DTISceneActorID id, bool bVis)
{
	// set the visibility of the specified image plane
	switch (id) 
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		_aSag->SetVisibility(bVis);
		_aSag->Modified();
		break;
	case DTI_ACTOR_AXIAL_TOMO:
		_aAxial->SetVisibility(bVis);
		_aAxial->Modified();
		break;
	case DTI_ACTOR_CORONAL_TOMO:
		_aCor->SetVisibility(bVis);
		_aCor->Modified();
		break;
	default:
		break;
	};
	// update the visibility of the overlay too
	for(unsigned i = 0; i < _overlays.size(); i++)
		_overlays[i]->SetVisible(id,bVis);
	if((int)id == _nActiveImage)
		_aBorder->SetVisibility(bVis);
}

vtkMatrix4x4* qVolumeViz::GetUserMatrix()
{
	assert(_aCor!=NULL); 
	return _aCor->GetUserMatrix();
}

bool qVolumeViz::OnRightButtonUp(int x, int y)
{
	// show the overlay panel if we clicked on one of the image planes...

	_propPicker->PickProp(x, y, _renderer, _propCollection);
	vtkProp *prop = _propPicker->GetViewProp();

	 // There could be other props assigned to this picker, so 

    // make sure we picked the image actor

    vtkAssemblyPath* path = _propPicker->GetPath();

    bool validPick = false;

 

	if (path)

	{

		vtkCollectionSimpleIterator sit;

		path->InitTraversal(sit);

		vtkAssemblyNode *node;

		for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)

		{

			node = path->GetNextNode(sit);

			if( prop == _aAxial || prop == _aCor || prop == _aSag || prop == _aBorder)
			{
				NotifyAllListeners( PEvent (new Event(SHOW_OVERLAY_PANEL)) );
				return true;
			}

		}

	}


	// this is for backup if the first one didnt work
	if( prop == _aAxial || prop == _aCor || prop == _aSag || prop == _aBorder)
	{
		NotifyAllListeners( PEvent (new Event(SHOW_OVERLAY_PANEL)) );
		return true;
	}
	return false;
}

void qVolumeViz::Serialize(ostream &s, int version)
{
	// Tomo Position
	s<<"--- Volume State ---"<<endl;
	s << "Volume Section's Position: " << _vPos[0] << "," << _vPos[1] << "," << _vPos[2] << std::endl;
	s << "Volume Section's Visibility: " << _aSag->GetVisibility() << "," << _aCor->GetVisibility() << "," << _aAxial->GetVisibility() << std::endl;
	s << "Active Image :"<<_nActiveImage << endl;
	s<<endl;

	s<<"--- Overlays ---"<<endl;
	s<<"Num overlay items :"<<_overlays.size()<<endl;
	for(int i = 0; i < (int)_overlays.size(); i++)
		_overlays[i]->Serialize(s,version);
}

void qVolumeViz::Deserialize(istream &s, int version)
{
	//_overlays.clear();
	double pos[3];
	bool bVis[3];
	s>>"--- Volume State ---">>endl;
	GetQStateVector3<double>(s,pos);
	GetQStateVector3<bool  >(s,bVis);
	int activeImage;
	s >> "Active Image :">> activeImage >> endl;
	s>>endl;

	DTISceneActorID id = (DTISceneActorID) activeImage;
	SetActiveImage( id );

	int unused;
	s>>"--- Overlays ---">>endl;
	s>>"Num overlay items :">>unused>>endl;
	for(int i = 0; i < (int)_overlays.size(); i++)
		_overlays[i]->Deserialize(s,version);

	Vector3d vpos(pos);
	SetPosition(vpos);
	SetVisibility(DTI_ACTOR_SAGITTAL_TOMO,bVis[0]);
	SetVisibility(DTI_ACTOR_CORONAL_TOMO,bVis[1]);
	SetVisibility(DTI_ACTOR_AXIAL_TOMO,bVis[2]);
	for(unsigned i = 0; i < _overlays.size(); i++)
		for(unsigned j = 0; j < 3; j++)
		_overlays[i]->SetVisible((DTISceneActorID)j,bVis[j]);
}


void qVolumeViz::ComputeCurrentVolumeBounds(Vector3d &vmin, Vector3d &vmax)
{
	if(!_vol)return;
	double *transform=_vol->getTransformMatrix();
	unsigned dim[3];
	_vol->getDimension(dim[0], dim[1], dim[2]);
	for(int i = 0; i < 3; i++)
	{
		vmax[i] = transform[i*4+0]*dim[0] + transform[i*4+1]*dim[1] + transform[i*4+2]*dim[2] + transform[i*4+3];
		vmin[i] = transform[i*4+3];
		if(vmin[i] > vmax[i])
		{
			float temp = vmin[i]; vmin[i]=vmax[i], vmax[i]=temp;
		}
	}
}
bool qVolumeViz::WorldToLocal(double *mat, double *wPos, uint *dim, DTIVoxel &lPos, bool round)
{
  bool clamped = false;
  vtkMatrix4x4 *mx = vtkMatrix4x4::New();
  mx->DeepCopy (mat);

  double worldPos[4]={wPos[0], wPos[1], wPos[2], 1}; 
  mx->Invert();
  double *localPos = mx->MultiplyDoublePoint(worldPos);
  for(int ii = 0; ii < 3; ii++)
  {
	  // Must add because of floating point errors that we might get that could be exaggerated by float
	  double very_small_value = 0.0001;
	  if (round) {
	    very_small_value += 0.5;
	  }
	  int pos = floor(localPos[ii]+very_small_value);
	  if (pos < 0) {
	    clamped = true;
	    pos = 0;
	  }
	  lPos[ii]=(uint)pos; 
	  if (lPos[ii] > dim[ii]-1) {
	    clamped = true;
	    lPos[ii] = dim[ii]-1;
	  }
  }
  mx->Delete();
  return clamped;
}
void qVolumeViz::WorldToLocal(double *mat, double *wPos, uint *dim, double *flPos)
{
  vtkMatrix4x4 *mx = vtkMatrix4x4::New();
  mx->DeepCopy (mat);

  double worldPos[4]={wPos[0], wPos[1], wPos[2], 1}; 
  mx->Invert();
  double *localPos = mx->MultiplyDoublePoint(worldPos);
  for(int ii = 0; ii < 3; ii++)
	  flPos[ii]=localPos[ii];
	//  if (flPos[ii] > dim[ii]-1) flPos[ii] = dim[ii]-1;
  mx->Delete();
}

void qVolumeViz::LocalToWorld(double *mat, DTIVoxel &lPos, double *wPos)
{
	double localPos[4]={lPos[0], lPos[1], lPos[2], 1}; 
	LocalToWorld(mat, localPos, wPos);
}
void qVolumeViz::LocalToWorld(double *mat, double *lPos, double *wPos)
{
	vtkMatrix4x4 *mx = vtkMatrix4x4::New();
	mx->DeepCopy (mat);

	double localPos[4]={lPos[0], lPos[1], lPos[2], 1}; 
	double *worldPos = mx->MultiplyDoublePoint(localPos);
	for(int ii = 0; ii < 3; ii++)
		wPos[ii]=worldPos[ii]; 
	mx->Delete();
}

void qVolumeViz::GetCursorPosition(DTIVoxel &lPos)
{
  uint dim[4];
  _vol->getDimension(dim[0], dim[1], dim[2], dim[3]);
  WorldToLocal(_vol->getTransformMatrix(), _vPos, dim, lPos);
}
