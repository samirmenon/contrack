/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "vtkInteractorStyleDTI.h"

#include <wx/filedlg.h>

#include <GL/gl.h>
#include "vtkTextActor.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkCellPicker.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkImageMapToColors.h"
#include "vtkMatrix4x4.h"

#include "query_typedefs.h"

#include <string.h>
#include <util/DTIPathway.h>
#include <util/DTIFilterTree.h>
#include <util/DTIPathwayIntersectionVolume.h>
#include <util/DTIPathwaySelection.h>

#include <math.h>
#include <hash_map.h>

#include "vtkStripper.h"
#include "vtkCellLocator.h"
#include "vtkCellLocator.h"
#include "scene.h"
#include "VOICube.h"
#include "VOISphere.h"
#include "VOILoaded.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkWorldPointPicker.h"
#include "vtkPointPicker.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "ViewCone.h"
#include <util/DTIPathwayDatabase.h>
#include <util/DTIFilterPathwayScalar.h>
#include <util/DTIPathwayIO.h>
#include <util/DTIFilterAlgorithm.h>
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCubeSource.h"
#include "vtkProp3DCollection.h"
#include "vtkAbstractPropPicker.h"
#include <util/DTIFilterCube.h>
#include "InteractorListener.h"
#include "vtkImageActor.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkFilledCaptionActor2D.h"
#include "vtkColorTransferFunction.h"

#include "CameraControls.h"
#include "DTIFilterApp.h"
#include <util/DTIQueryProcessor.h>
#include <util/DTIVolumeIO.h>
#include "SystemInfoDialog.h"
#include "ButtonChoiceDialog.h"

#include "wx/dir.h"

static const double __initial_VOI_size = 20.0;

vtkCxxRevisionMacro(vtkInteractorStyleDTI, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkInteractorStyleDTI);

using std::cout;
using std::endl;

//----------------------------------------------------------------------------
vtkInteractorStyleDTI::vtkInteractorStyleDTI() 
{
  strcpy (_data_directory_path, "");

  _background_paths = NULL;
  _background_filenames = NULL;

  this->_lut_level = 0.5;
  this->_lut_window = 1.0;

  _query_processor = new DTIQueryProcessor (this);

  this->current_background_id = 0;
  this->voiLabelsToggle = true;
  this->voiVisibilityToggle = true;
  this->camera_controls = NULL;
  this->frame= NULL;
  this->m_ghosting = false;
  this->m_meshVisibility = 0;

  this->bDemoMode = false;
  this->bSPosSlice = true;
  this->bAPosSlice = true;
  this->lineWidthSetting = 0;
  this->_filter_op = DTI_FILTER_AND;
  this->voiCounter = 1;
  this->MotionFactor   = 10.0;
  this->ActorSelected = DTI_ACTOR_AXIAL_TOMO;
  this->activeVOI = NULL;
  this->ViewCut = NULL;
  this->whole_database = NULL;
  pathwaysPolyData = NULL;
  pathwaysMapper = vtkPolyDataMapper::New();
  pathwaysActor = vtkActor::New();
  pathwaysActor->SetMapper(pathwaysMapper);
  this->minPathwayLength = 0;
  lengthFilter = new DTIFilterPathwayScalar(PATHWAY_LENGTH);
  medianFAFilter = new DTIFilterPathwayScalar (PATHWAY_MEDIAN_FA);
  meanCurvatureFilter = new DTIFilterPathwayScalar (PATHWAY_CURVATURE);
  algoFilter = new DTIFilterAlgorithm ();

  this->TomoBorderActor = NULL;

  this->current_database = NULL;
  this->curPathwayRegion=1;
  bInitializedScenePathways=false;
  bInitializedTomograms=false;

  // Initialize a varying color map that we can borrow from the
  // color brewer by Cindy Brewer.  the point of this map is to show different 
  // pathway regions
  color c;
  /*c.r=127; c.g=201; c.b=127;
  pathway_colormap.push_back(c);*/
  // replacing with a slightly darker green:
  c.r=77; c.g=175; c.b=74;
  pathway_colormap_class.push_back(c);
  c.r=77; c.g=175; c.b=74;
  pathway_colormap_seq.push_back(c);
  c.r=55; c.g=126; c.b=184;
  pathway_colormap_class.push_back(c);
  c.r=65; c.g=182; c.b=196;
  pathway_colormap_seq.push_back(c);
  c.r=55; c.g=126; c.b=184;
  pathway_colormap_class.push_back(c);
  c.r=161; c.g=218; c.b=180;
  pathway_colormap_seq.push_back(c);
  c.r=152; c.g=78; c.b=163;
  pathway_colormap_class.push_back(c);
  c.r=255; c.g=255; c.b=204;
  pathway_colormap_seq.push_back(c);
  c.r=255; c.g=127; c.b=0;
  pathway_colormap_class.push_back(c);
  c.r=255; c.g=255; c.b=204;
  pathway_colormap_seq.push_back(c);
  c.r=255; c.g=255; c.b=51;
  pathway_colormap_class.push_back(c);
  c.r=255; c.g=255; c.b=204;
  pathway_colormap_seq.push_back(c);
#if 0
  c.r=107;c.g=187;c.b=107;
  pathway_colormap_class.push_back(c);
  c.r=37; c.g=52; c.b=148;
  pathway_colormap_seq.push_back(c);
  c.r=190; c.g=174; c.b=212;
  pathway_colormap_class.push_back(c);
  c.r=44; c.g=127; c.b=148;
  pathway_colormap_seq.push_back(c);
  c.r=253; c.g=192; c.b=134;
  pathway_colormap_class.push_back(c);
  c.r=65; c.g=182; c.b=196;
  pathway_colormap_seq.push_back(c);
  c.r=255; c.g=255; c.b=153;
  pathway_colormap_class.push_back(c);
  c.r=161; c.g=218; c.b=180;
  pathway_colormap_seq.push_back(c);
  c.r=56; c.g=108; c.b=176;
  pathway_colormap_class.push_back(c);
  c.r=255; c.g=255; c.b=204;
  pathway_colormap_seq.push_back(c);
#endif

  vtkMath::RandomSeed(942385);

  AvgFPS=0;
  NumFPSMeasures=0;
 
}

//----------------------------------------------------------------------------
vtkInteractorStyleDTI::~vtkInteractorStyleDTI() 
{
	this->pathwaysActor->Delete();
	this->pathwaysMapper->Delete();
	this->pathwaysPolyData->Delete();

	TomoBorderPolyData->Delete();
	TomoBorderMapper->Delete();
	TomoBorderActor->Delete();

	if(this->ViewCut!=NULL)
		delete this->ViewCut;
}


//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnMouseMove() 
{
  
#if 0
  vtkRenderWindowInteractor *rwi = this->Interactor;

  if(rwi->GetRenderWindow()->GetAAFrames() > 0) {
    rwi->GetRenderWindow()->SetAAFrames(0);
  }
#endif

  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];

  switch (this->State) 
    {
    case VTKIS_ROTATE:
      this->FindPokedRenderer(x, y);
      this->Rotate();
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      break;

    case VTKIS_PAN:
      this->FindPokedRenderer(x, y);
      this->Pan();
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      break;

    case VTKIS_DOLLY:
      this->FindPokedRenderer(x, y);
      this->Dolly();
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      break;

    case VTKIS_SPIN:
      this->FindPokedRenderer(x, y);
      this->Spin();
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
	  break;
	case VTKIS_MOVEVOI:
		this->FindPokedRenderer(x, y);
		this->MoveVOI();
		this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
		break;
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnLeftButtonDown() 
{ 
  glEnable (GL_DEPTH_TEST);
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  if (this->Interactor->GetShiftKey()) 
    {
    if (this->Interactor->GetControlKey()) 
      {
      this->StartDolly();
      }
    else 
      {
      this->StartPan();
      }
    } 
  else 
    {
    if (this->Interactor->GetControlKey()) 
	{
		//this->StartSpin();
		// This will be the moving VOI mode
		double pick[3];
		if(this->ActorSelected != DTI_ACTOR_SAGITTAL_TOMO && 
			this->ActorSelected != DTI_ACTOR_CORONAL_TOMO &&
			this->ActorSelected != DTI_ACTOR_AXIAL_TOMO &&
		   this->ActorSelected != DTI_ACTOR_SURFACE)
			return;

		if (this->whole_database == NULL) 
		  return;

		this->State = VTKIS_MOVEVOI;
      }
    else 
      {
      this->StartRotate();
      }
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnLeftButtonUp()
{
  #if 0
  vtkRenderWindowInteractor *rwi = this->Interactor;
  // turn on anti-aliasing and re-render

  if(rwi->GetRenderWindow()->GetAAFrames() == 0) {
    rwi->GetRenderWindow()->SetAAFrames(10);
  }
#endif
  
  switch (this->State) 
    {
    case VTKIS_DOLLY:
      this->EndDolly();
      break;

    case VTKIS_PAN:
      this->EndPan();
      break;

    case VTKIS_SPIN:
      this->EndSpin();
      break;

    case VTKIS_ROTATE:
      this->EndRotate();
      break;
	case VTKIS_MOVEVOI:
      this->State = VTKIS_NONE;
      break;
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnMiddleButtonDown() 
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->StartPan();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnMiddleButtonUp()
{
#if 0
  vtkRenderWindowInteractor *rwi = this->Interactor;
  // turn on anti-aliasing and re-render
  
  if(rwi->GetRenderWindow()->GetAAFrames() == 0) {
    rwi->GetRenderWindow()->SetAAFrames(10);
    rwi->Render();
  }
#endif

  switch (this->State) 
    {
    case VTKIS_PAN:
      this->EndPan();
      break;
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnRightButtonDown() 
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->StartDolly();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnRightButtonUp()
{
#if 0
   vtkRenderWindowInteractor *rwi = this->Interactor;
  // turn on anti-aliasing and re-render

  if(rwi->GetRenderWindow()->GetAAFrames() == 0) {
    rwi->GetRenderWindow()->SetAAFrames(10);
    rwi->Render();
  }
#endif
  switch (this->State) 
    {
    case VTKIS_DOLLY:
      this->EndDolly();
      break;
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::Rotate()
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  vtkRenderWindowInteractor *rwi = this->Interactor;

  int dx = rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];
  
  int *size = this->CurrentRenderer->GetRenderWindow()->GetSize();

  double delta_elevation = -20.0 / size[1];
  double delta_azimuth = -20.0 / size[0];
  
  double rxf = (double)dx * delta_azimuth * this->MotionFactor;
  double ryf = (double)dy * delta_elevation * this->MotionFactor;
  
  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->Azimuth(rxf);
  camera->Elevation(ryf);
  camera->OrthogonalizeViewUp();

  if (this->AutoAdjustCameraClippingRange)
    {
    this->CurrentRenderer->ResetCameraClippingRange();
    }

  if (rwi->GetLightFollowCamera()) 
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }

  
  this->frame->RefreshVTK ();
}


//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::Spin()
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double *center = this->CurrentRenderer->GetCenter();

  double newAngle = 
    atan2((double)rwi->GetEventPosition()[1] - (double)center[1],
          (double)rwi->GetEventPosition()[0] - (double)center[0]);

  double oldAngle = 
    atan2((double)rwi->GetLastEventPosition()[1] - (double)center[1],
          (double)rwi->GetLastEventPosition()[0] - (double)center[0]);
  
  newAngle *= vtkMath::RadiansToDegrees();
  oldAngle *= vtkMath::RadiansToDegrees();

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->Roll(newAngle - oldAngle);
  camera->OrthogonalizeViewUp();
      
  
  this->frame->RefreshVTK ();

}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::Pan()
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;

  double viewFocus[4], focalDepth, viewPoint[3];
  double newPickPoint[4], oldPickPoint[4], motionVector[3];
  
  // Calculate the focal depth since we'll be using it a lot

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  camera->GetFocalPoint(viewFocus);
  this->ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2], 
                              viewFocus);
  focalDepth = viewFocus[2];

  this->ComputeDisplayToWorld((double)rwi->GetEventPosition()[0], 
                              (double)rwi->GetEventPosition()[1],
                              focalDepth, 
                              newPickPoint);
    
  // Has to recalc old mouse point since the viewport has moved,
  // so can't move it outside the loop

  this->ComputeDisplayToWorld((double)rwi->GetLastEventPosition()[0],
                              (double)rwi->GetLastEventPosition()[1],
                              focalDepth, 
                              oldPickPoint);
  
  // Camera motion is reversed

  motionVector[0] = oldPickPoint[0] - newPickPoint[0];
  motionVector[1] = oldPickPoint[1] - newPickPoint[1];
  motionVector[2] = oldPickPoint[2] - newPickPoint[2];
  
  camera->GetFocalPoint(viewFocus);
  camera->GetPosition(viewPoint);
  camera->SetFocalPoint(motionVector[0] + viewFocus[0],
                        motionVector[1] + viewFocus[1],
                        motionVector[2] + viewFocus[2]);

  camera->SetPosition(motionVector[0] + viewPoint[0],
                      motionVector[1] + viewPoint[1],
                      motionVector[2] + viewPoint[2]);
      
  if (rwi->GetLightFollowCamera()) 
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
    
  
  this->frame->RefreshVTK ();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::Dolly()
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  vtkRenderWindowInteractor *rwi = this->Interactor;
  double *center = this->CurrentRenderer->GetCenter();
  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];
  double dyf = this->MotionFactor * (double)(dy) / (double)(center[1]);
  this->Dolly(pow((double)1.1, dyf));
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::Dolly(double factor)
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  if (camera->GetParallelProjection())
    {
    camera->SetParallelScale(camera->GetParallelScale() / factor);
    }
  else
    {
    camera->Dolly(factor);
    if (this->AutoAdjustCameraClippingRange)
      {
      this->CurrentRenderer->ResetCameraClippingRange();
      }
    }
  
  if (this->Interactor->GetLightFollowCamera()) 
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
  
  
  this->frame->RefreshVTK ();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}

void
vtkInteractorStyleDTI::MoveCurrentTomogram (int amount) 
{
  DTIScalarVolume *scalarVol = GetCurrentBackground();
  if (!scalarVol) return; 
  unsigned int dim[4];
  scalarVol->getDimension (dim[0], dim[1], dim[2], dim[3]);
  double voxSize[3];
  scalarVol->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
  switch(this->ActorSelected) {
  case DTI_ACTOR_SAGITTAL_TOMO:
    scene->curSSlice += amount;
    if(scene->curSSlice >= dim[0])
      scene->curSSlice = (amount > 0) ? dim[0]-1 : 0;
    scene->sagittal->SetDisplayExtent(scene->curSSlice,scene->curSSlice, 0,dim[1]-1, 0,dim[2]-1);
    scene->sagittal->Modified();
    this->scene->ReportPositionFromTomos(scene->sagittal->GetUserMatrix());
    DisplayTomoBorder(true);
    break;
  case DTI_ACTOR_AXIAL_TOMO:
    scene->curASlice += amount;
    if(scene->curASlice >= dim[2])
      scene->curASlice = (amount > 0) ? dim[2]-1 : 0;
    scene->axial->SetDisplayExtent(0,dim[0]-1, 0,dim[1]-1, scene->curASlice,scene->curASlice);
    scene->axial->Modified();
    this->scene->ReportPositionFromTomos(scene->sagittal->GetUserMatrix());
    DisplayTomoBorder(true);
    break;
  case DTI_ACTOR_CORONAL_TOMO:
    scene->curCSlice += amount;
    if(scene->curCSlice >= dim[1])
      scene->curCSlice = (amount > 0) ? dim[1]-1 : 0;
    scene->coronal->SetDisplayExtent(0,dim[0]-1, scene->curCSlice,scene->curCSlice, 0,dim[2]-1);
    scene->coronal->Modified();
    this->scene->ReportPositionFromTomos(scene->sagittal->GetUserMatrix());
    DisplayTomoBorder(true);
    break;
  default:
    break;
  }
  if (this->AutoAdjustCameraClippingRange)
    {
      this->CurrentRenderer->ResetCameraClippingRange();
    }

}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnChar() 
{

  vtkRenderWindowInteractor *rwi = this->Interactor;
  voiIter iter;
  float bg[3];

  this->FindPokedRenderer(0,0);
  
  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

  switch (this->Interactor->GetKeyCode())
    {
    case ',':
      MoveCurrentTomogram (-1);
      this->frame->RefreshVTK();
      break;
    case '.':
      MoveCurrentTomogram (+1);
      this->frame->RefreshVTK();
      break;
  
    case 'a':
    case 'A':
      if (_background_paths) {
	if (Interactor->GetShiftKey()) {
	  ToggleVisibility (DTI_ACTOR_AXIAL_TOMO);
	  this->frame->SetVisibilityCheck(DTI_ACTOR_AXIAL_TOMO, getSceneProp (DTI_ACTOR_AXIAL_TOMO)->GetVisibility());
	}
	else {
	  SelectPlane (DTI_ACTOR_AXIAL_TOMO);
	}
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
    case 's':
    case 'S':
      if (_background_paths) {
	if (Interactor->GetShiftKey()) {
	  ToggleVisibility (DTI_ACTOR_SAGITTAL_TOMO);
	  this->frame->SetVisibilityCheck(DTI_ACTOR_SAGITTAL_TOMO, getSceneProp (DTI_ACTOR_SAGITTAL_TOMO)->GetVisibility());
	}
	else {
	  SelectPlane (DTI_ACTOR_SAGITTAL_TOMO);
	}
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
    case 'c':
    case 'C':
      if (_background_paths) {
	if (Interactor->GetShiftKey()) {
	  ToggleVisibility (DTI_ACTOR_CORONAL_TOMO);
	  this->frame->SetVisibilityCheck(DTI_ACTOR_CORONAL_TOMO, getSceneProp (DTI_ACTOR_CORONAL_TOMO)->GetVisibility());
	}
	else {
	  SelectPlane (DTI_ACTOR_CORONAL_TOMO);
	}
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
    case 'v':
    case 'V':
      if (this->whole_database) {
	if (Interactor->GetShiftKey()) {
	  ToggleVOIVisibility();
	}
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
    case 'p':
    case 'P':
      if (this->whole_database) {
	if (Interactor->GetShiftKey()) {
	  ToggleVisibility(DTI_ACTOR_TRACTS);
	}
      }
      this->EventCallbackCommand->SetAbortFlag(1);
      break;
#if 0
    case 'm':
      this->ActorSelected = DTI_ACTOR_SURFACE;
      DisplayTomoBorder(false);
      this->EventCallbackCommand->SetAbortFlag(1);
      this->frame->RefreshVTK();
      break;
    case 'g':
    case 'G':
      // ghosting mode
      if (!GetCurrentSurfaceActor()) {
	m_ghosting = !m_ghosting;
	break;
      }
      if (m_ghosting) {
	GetCurrentSurfaceActor()->SetVisibility (true);
	SetGhostVisibility(false);
      }
      else {
	// turning ON ghosting
	GetCurrentSurfaceActor()->SetVisibility (false);
	SetGhostVisibility(true);
      }
      
      m_ghosting = !(m_ghosting);
      this->EventCallbackCommand->SetAbortFlag(1);
      this->StopState();
      break;
#endif
    case 't':
    case 'T':
      // Toggle the VOI labels on/off
      voiLabelsToggle = !(voiLabelsToggle);
      for(iter=this->voiList.begin();iter!=this->voiList.end();iter++)
	(*iter)->getTextActor()->SetVisibility(voiLabelsToggle);
      this->frame->RefreshVTK();
      break;
    case 'b':
    case 'B':
      this->scene->NextBackground(bg);
      this->CurrentRenderer->SetBackground(bg[0],bg[1],bg[2]);
      this->frame->RefreshVTK();
      break;
      //    case 'm':
      //    case 'M':
      //      this->scene->cursorInfo->SetVisibility(!(this->scene->cursorInfo->GetVisibility()));
      //      break;
    case 'k':
    case 'K':
      if(rwi->GetRenderWindow()->GetAAFrames() > 0)
	rwi->GetRenderWindow()->SetAAFrames(0);
      else
	rwi->GetRenderWindow()->SetAAFrames(10);
      this->frame->RefreshVTK();
      break;
    case '[':
      if (GetCurrentBackground()) {
	unsigned int dim[3];
	unsigned int dummy;
      	double voxSize[3];
      	GetCurrentBackground()->getDimension (dim[0], dim[1], dim[2], dummy);
      	GetCurrentBackground()->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
	vtkMatrix4x4 *mx = this->scene->coronal->GetUserMatrix();
	this->camera_controls->NextPosition();
	this->camera_controls->Update(camera, dim, voxSize, mx);
	this->CurrentRenderer->ResetCameraClippingRange();
      }
      this->frame->RefreshVTK();
      break;
#if 0
    case ']':
      if (GetCurrentBackground()) {
	unsigned int dim[3];
	unsigned int dummy;
      	double voxSize[3];
      	GetCurrentBackground()->getDimension (dim[0], dim[1], dim[2], dummy);
      	GetCurrentBackground()->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
	vtkMatrix4x4 *mx = this->scene->coronal->GetUserMatrix();
	this->camera_controls->NextPosition();
	this->camera_controls->Update(camera, dim, voxSize, mx);
	this->CurrentRenderer->ResetCameraClippingRange();
      }
      this->frame->RefreshVTK();
      break;
#endif
    case 'w':
    case 'W':
      this->NextLineWidth();
      this->frame->RefreshVTK();
      break;
    default:
      break;
    }
}

void vtkInteractorStyleDTI::DisplayTomoBorder(bool on)
{
  if (!GetCurrentBackground()) {
    return;
  }
  if (this->ActorSelected == DTI_ACTOR_NONE || (getSceneProp(this->ActorSelected))->GetVisibility() == false) {
    on = false;
  }
	//on = false;
	this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
				this->Interactor->GetEventPosition()[1]);
	if(this->TomoBorderActor == NULL && this->CurrentRenderer!=NULL )
	{
		this->TomoBorderPolyData = vtkPolyData::New();
		vtkPoints *pts = vtkPoints::New();
		pts->SetNumberOfPoints(4);
		this->TomoBorderPolyData->SetPoints(pts);
		pts->Delete();
		vtkCellArray *border = vtkCellArray::New();
		border->InsertNextCell(5);
		border->InsertCellPoint(0);
		border->InsertCellPoint(1);
		border->InsertCellPoint(2);
		border->InsertCellPoint(3);
		border->InsertCellPoint(0);
		this->TomoBorderPolyData->SetLines(border);  
		border->Delete();
		this->TomoBorderMapper = vtkPolyDataMapper::New();
		this->TomoBorderMapper->SetInput(this->TomoBorderPolyData);
		this->TomoBorderActor = vtkActor::New();
		this->TomoBorderActor->SetMapper(this->TomoBorderMapper);
		this->CurrentRenderer->AddActor(this->TomoBorderActor);
		this->TomoBorderActor->GetProperty()->SetColor(1,0,0);
		this->TomoBorderActor->GetProperty()->SetLineWidth(5);
	}

	// Position the slice
	double p[3], m[3];
	double voxSize[3];
	GetCurrentBackground()->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
	unsigned int dim[4];
	GetCurrentBackground()->getDimension (dim[0], dim[1], dim[2], dim[3]);
	p[0] = this->scene->curSSlice;
	p[1] = this->scene->curCSlice;
	p[2] = this->scene->curASlice;
	m[0] = (dim[0]-1);
	m[1] = (dim[1]-1);
	m[2] = (dim[2]-1);

	this->TomoBorderActor->SetUserMatrix (scene->coronal->GetUserMatrix());
	vtkPoints *pts = this->TomoBorderPolyData->GetPoints();

	// Axial Plane
	switch(this->ActorSelected) {
	case DTI_ACTOR_SAGITTAL_TOMO:
	  pts->SetPoint(0, p[0],0,0);
	  pts->SetPoint(1, p[0],0,m[2]);
	  pts->SetPoint(2, p[0],m[1],m[2]);
	  pts->SetPoint(3, p[0],m[1],0);
	  break;
	case DTI_ACTOR_CORONAL_TOMO:
	  pts->SetPoint(0, 0,p[1],0);
	  pts->SetPoint(1, m[0],p[1],0);
	  pts->SetPoint(2, m[0],p[1],m[2]);
	  pts->SetPoint(3, 0,p[1],m[2]);
	  break;
	case DTI_ACTOR_AXIAL_TOMO:
	  pts->SetPoint(0, 0,0,p[2]);
	  pts->SetPoint(1, m[0],0,p[2]);
	  pts->SetPoint(2, m[0],m[1],p[2]);
	  pts->SetPoint(3, 0,m[1],p[2]);
	  break;
	default:
	  break;
	}
	this->TomoBorderPolyData->Modified();
	this->TomoBorderActor->SetVisibility(on);

}

void 
vtkInteractorStyleDTI::LoadPathwaysInternal (ifstream &stream) {

  DTIPathwayDatabase *db = DTIPathwayIO::loadDatabasePDB (stream);

  if (db == NULL) {
    wxString msg;
    wxString caption(_T("Error loading pathways"));
    wxString message(_T("DTI-Query cannot load this pathway format.\nPlease try recomputing the pathways."));
    
    SystemInfoDialog* info_dialog = new SystemInfoDialog(this->frame, caption, message);
    info_dialog->Show();
    return;
  }

  delete this->whole_database;
  this->whole_database = db;

  //  this->whole_database->getSceneDimension(scene->dim);
  //  this->whole_database->getVoxelSize(scene->delta);
  //  scene->curSSlice = scene->dim[0]/2;
  //  scene->curCSlice = scene->dim[1]/2;
  //  scene->curASlice = scene->dim[2]/2;

  this->InitializePathwayJitter();
  lengthFilter->setMinValue (this->whole_database->getMinValue(PATHWAY_LENGTH));
  lengthFilter->setMaxValue (this->whole_database->getMaxValue(PATHWAY_LENGTH));
  medianFAFilter->setMinValue (this->whole_database->getMinValue(PATHWAY_MEDIAN_FA));
  medianFAFilter->setMaxValue (this->whole_database->getMaxValue(PATHWAY_MEDIAN_FA));
  meanCurvatureFilter->setMinValue (this->whole_database->getMinValue(PATHWAY_CURVATURE));
  meanCurvatureFilter->setMaxValue (this->whole_database->getMaxValue(PATHWAY_CURVATURE));

  //  whole_database->addPathwayPropertyFilter(this->lengthFilter);
  //  whole_database->addPathwayPropertyFilter(this->medianFAFilter);
  //  whole_database->addPathwayPropertyFilter(this->meanCurvatureFilter);
  _query_processor->appendScalarFilter (this->lengthFilter);
  _query_processor->appendScalarFilter (this->medianFAFilter);
  _query_processor->appendScalarFilter (this->meanCurvatureFilter);

  _query_processor->setAlgoFilter (this->algoFilter);

  this->algoFilter->setFilter (PATHWAY_ALGORITHM_STT);
  this->scene->totalPathways = this->whole_database->getNumFibers();

  // Make sure that everything gets re-initialized, since we have new
  // paths:

  if (_filter_op == DTI_FILTER_AND || _filter_op == DTI_FILTER_OR) {
    this->current_database = whole_database;
    rebuildFilters();
  }
  this->bInitializedScenePathways = false;
  this->SetScenePathways( );
  for (std::list<InteractorListener *>::iterator iiter = this->listeners.begin(); iiter != this->listeners.end(); iiter++) {
    InteractorListener *listener = *iiter;
    listener->interactorWindowDatabaseLoaded (whole_database);
  }
}

void 
vtkInteractorStyleDTI::SetVOI (int index) 
{
  int count = 0;
  for (voiIter iter = this->voiList.begin(); iter != this->voiList.end(); iter++) {
    VOI *voi = *iter;
    if (count == index) {
      activeVOI = voi;
      this->HighlightProp(activeVOI->getActor());
      broadcastActiveVOI();
      return;
    }
    count++;
    
  }
//  assert (false); // shouldn't ever get here!
}

void vtkInteractorStyleDTI::CycleVOI()
{
  int count = 0;
  for (voiIter iter = this->voiList.begin(); iter != this->voiList.end(); iter++) {
    VOI *voi = *iter;
    if (voi == activeVOI) {
      SetVOI ((count + 1) % this->voiList.size());
      return;
    }
    count++; 
  }
}

void vtkInteractorStyleDTI::InsertVOIFromFile()
{

  // XXX This is a hack way to load VOIs from files, which was necessary for silvia's experiments

  //VOI* voi = new VOILoaded(id, scene->delta, center, length, "/radlab_share/home/tony/images/silvia_data/dti_sil_nov/surfaces/fibula");

  double center[3] = {0,0,0};
  double length[3] = {0,0,0};
  std::vector<std::string> sils_filenames;
  std::vector<std::string>::iterator it;
  std::string sil_d = "/radlab_share/home/tony/images/silvia_data/dti_sil_nov/surfaces/";
  //std::string sil_d = "/radlab_share/home/tony/src/dtivis/meshes/";
  sils_filenames.push_back(sil_d + "fibula");
  sils_filenames.push_back(sil_d + "lgas_insert");
  sils_filenames.push_back(sil_d + "lgas_origin");
  sils_filenames.push_back(sil_d + "lgas");
  sils_filenames.push_back(sil_d + "mgas_insert");
  sils_filenames.push_back(sil_d + "mgas_origin");
  sils_filenames.push_back(sil_d + "mgas");
  sils_filenames.push_back(sil_d + "soleus_insert1");
  sils_filenames.push_back(sil_d + "soleus_insert2");
  sils_filenames.push_back(sil_d + "soleus_origin1");
  sils_filenames.push_back(sil_d + "soleus_origin2");
  sils_filenames.push_back(sil_d + "soleus_origin3");
  sils_filenames.push_back(sil_d + "soleus");
  sils_filenames.push_back(sil_d + "tibia");
  //sils_filenames.push_back(sil_d + "bob2_smooth");
  //sils_filenames.push_back(sil_d + "bob2_nosmooth");
  
  for(it = sils_filenames.begin(); it != sils_filenames.end(); it++) {
    VOI* voi = new VOILoaded(this->voiCounter, center, length, (*it).c_str());
    voi->getTextActor()->SetVisibility (voiLabelsToggle);

    this->voiCounter++;
    (*it).erase(0,(*it).find_last_of('/')+1);
    voi->SetLabel ((*it).c_str());
    this->voiList.push_back(voi);
    
    if(this->current_database != NULL) 
      {
	if (_filter_op == DTI_FILTER_AND || _filter_op == DTI_FILTER_OR) {
	  rebuildFilters();
	}
	this->SetScenePathways( );
      }
    // Reset active VOI
    activeVOI = voi;
    this->broadcastActiveVOI();
    
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
			    this->Interactor->GetEventPosition()[1]);
    if(this->CurrentRenderer != NULL && activeVOI != NULL)
      {
	this->CurrentRenderer->AddActor(activeVOI->getActor());
	this->CurrentRenderer->AddActor2D((vtkProp*)activeVOI->getTextActor());
      }
  }
}

void vtkInteractorStyleDTI::DeleteVOI()
{
  // Delete the active VOI
  if(activeVOI != NULL)
    {
      this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
			      this->Interactor->GetEventPosition()[1]);
      if(this->CurrentRenderer != NULL) 
	{
	  this->CurrentRenderer->RemoveActor(activeVOI->getActor());
	  this->CurrentRenderer->RemoveActor((vtkProp*)activeVOI->getTextActor());
	}
      
      for(voiIter iter=this->voiList.begin();iter!=this->voiList.end() && activeVOI != NULL;iter++)
	{
	  if (*iter == activeVOI)
	    {
	      // Remove the query item
	      
	      this->voiList.erase(iter);
	      delete activeVOI;
	      if (!this->voiList.empty()) {
		activeVOI=*(this->voiList.end()-1);
		this->HighlightProp(activeVOI->getActor());
	      }
	      else {
		activeVOI= NULL;
		this->HighlightProp(NULL);
	      }
	      broadcastActiveVOI();
	      break;
	    }
	}
      if(this->current_database != NULL) 
	{
	  //assert (false);
	  if (_filter_op == DTI_FILTER_AND || _filter_op == DTI_FILTER_OR) {
	    rebuildFilters();
	  }
	  //this->current_database->removeFilter((DTIFilter*)activeVOI->filter);
	  this->SetScenePathways( );
	}
      // Remove previous highlights
      //this->HighlightProp(NULL);
     }
  this->frame->RefreshVTK();
}

void vtkInteractorStyleDTI::InsertVOI()
{
	double center[3];
	double length[3];
	//	center[0] = scene->curSSlice*scene->delta[0];
	//	center[1] = scene->curCSlice*scene->delta[1];
	//	center[2] = scene->curASlice*scene->delta[2];
	center[0] = 0;
	center[1] = 0;
	center[2] = 0;
	length[0]=length[1]=length[2]=__initial_VOI_size;
	this->InsertVOI(center,length, this->voiCounter);

	this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
		this->Interactor->GetEventPosition()[1]);
	if(this->CurrentRenderer != NULL && activeVOI != NULL)
	{
		this->CurrentRenderer->AddActor(activeVOI->getActor());
		this->CurrentRenderer->AddActor2D((vtkProp*)activeVOI->getTextActor());
	}
	// Automatically go int VOI size adjusting mode
	//	this->ActorSelected = DTI_ACTOR_VOI;
	this->frame->RefreshVTK();
}

void vtkInteractorStyleDTI::InsertVOI(double center[3], double length[3], int id)
{

  VOI* voi = new VOICube(id, center, length);
  voi->getActor()->SetVisibility(this->voiVisibilityToggle);
  voi->getTextActor()->SetVisibility(voiLabelsToggle);


  this->voiCounter++;
  voi->SetLabel ("");
  this->voiList.push_back(voi);
  
  if(this->current_database != NULL) 
    {
      if (_filter_op == DTI_FILTER_AND || _filter_op == DTI_FILTER_OR) {
	rebuildFilters();
      }
      this->SetScenePathways( );
    }
  // Reset active VOI
  activeVOI = voi;
  this->HighlightProp(activeVOI->getActor());
  this->broadcastActiveVOI();
}


double vtkInteractorStyleDTI::GetPathwayJitter(DTIPathway* t)
{
	// See if the pathway is within any of the groups
	// 0 is the default pathway group
	if(this->pathwayJittermap.size()>0)
	{
		std::map<DTIPathway*, double>::iterator fiber  = this->pathwayJittermap.find(t);
		if(fiber != this->pathwayJittermap.end())
			return (*fiber).second;
	}
	return 0;
}


short vtkInteractorStyleDTI::GetPathwayGroup(DTIPathway* t)
{
	// See if the pathway is within any of the groups
	// 0 is the default pathway group
	if(this->pathwayVOImap.size()>0)
	{
		std::map<DTIPathway*, int>::iterator fiber  = this->pathwayVOImap.find(t);
		if(fiber != this->pathwayVOImap.end())
			return (*fiber).second;
	}
	return 0;
}

void vtkInteractorStyleDTI::SetPathwayGroup(short g)
{
	DTIPathway* pathway;
	vtkUnsignedCharArray* colors = vtkUnsignedCharArray::New();
	colors->SetNumberOfComponents(3);

	for(int j=0;j<this->whole_database->getNumFibers();j++)
	{
		pathway = this->whole_database->getPathway(j);

		if( this->_query_processor->doesPathwayMatch(pathway) )
		{
			this->pathwayVOImap[pathway] = g;
		}

		// figure out what color we are going to give the next pathway
		// random jitter
		double vRand = this->GetPathwayJitter(pathway);
		double drgb[3],hsv[3];
		unsigned char rgb[3];
		std::vector<color>::iterator c;
		if(this->scene->curPathwayColoring==COLOR_STAIN)
		{
			c=this->pathway_colormap_class.begin();
			int group = this->GetPathwayGroup(pathway);
			c= c + group;
			drgb[0]=(*c).r/255.; 
			drgb[1]=(*c).g/255.; 
			drgb[2]=(*c).b/255.;
			vtkMath::RGBToHSV(drgb,hsv);
			//if(g==0)
			hsv[2]+=vRand;
			if( hsv[2] > 1)
				hsv[2]=1;
			if( hsv[2] < 0)
				hsv[2]=0;
			vtkMath::HSVToRGB(hsv,drgb);	
			rgb[0]=(unsigned char) floor(255 * drgb[0]);
			rgb[1]=(unsigned char) floor(255 * drgb[1]);
			rgb[2]=(unsigned char) floor(255 * drgb[2]);
		}
		for(int i=0;i<pathway->getNumPoints();i++)
			colors->InsertNextTuple3(rgb[0],rgb[1],rgb[2]);
	}
	colors->SetName("colors");

	pathwaysPolyData->GetPointData()->SetScalars(colors);
	colors->Delete();
}

void vtkInteractorStyleDTI::SetPathwayGroup2(short g)
{
	DTIPathway* pathway;
	vtkPolyDataMapper* mapper;
	vtkUnsignedCharArray* colors;

	for(int j=0;j<this->whole_database->getNumFibers();j++)
	{
		pathway = this->whole_database->getPathway(j);
		if( this->_query_processor->doesPathwayMatch(pathway) )
		{
			this->pathwayVOImap[pathway] = g;
			mapper = vtkPolyDataMapper::SafeDownCast(this->pathway_list[j]->GetMapper());
			colors = vtkUnsignedCharArray::SafeDownCast(mapper->GetInput()->GetPointData()->GetScalars());
			colors->SetName("colors");
			// figure out what color we are going to give the next pathway
			// random jitter
			double vRand = this->GetPathwayJitter(pathway);
			double drgb[3],hsv[3];
			unsigned char rgb[3];
			std::vector<color>::iterator c;
			if(this->scene->curPathwayColoring==COLOR_STAIN)
			{
				c=this->pathway_colormap_class.begin();
				int group = this->GetPathwayGroup(pathway);
				c= c + group;
				drgb[0]=(*c).r/255.; 
				drgb[1]=(*c).g/255.; 
				drgb[2]=(*c).b/255.;
				vtkMath::RGBToHSV(drgb,hsv);
				//if(g==0)
				hsv[2]+=vRand;
				if( hsv[2] > 1)
					hsv[2]=1;
				if( hsv[2] < 0)
					hsv[2]=0;
				vtkMath::HSVToRGB(hsv,drgb);	
				rgb[0]=(unsigned char) floor(255 * drgb[0]);
				rgb[1]=(unsigned char) floor(255 * drgb[1]);
				rgb[2]=(unsigned char) floor(255 * drgb[2]);
			}
			for(int i=0;i<pathway->getNumPoints();i++)
				colors->InsertNextTuple3(rgb[0],rgb[1],rgb[2]);

			mapper->GetInput()->Modified();
			colors->Delete();
		}
	}
}

void vtkInteractorStyleDTI::SelectSlicePoint(int slice, int selectionX, int selectionY, double pick[4]) 
{
  unsigned int dim[4];
  GetCurrentBackground()->getDimension(dim[0], dim[1], dim[2], dim[3]);

  vtkMatrix4x4 *mx = scene->sagittal->GetUserMatrix();
  vtkMatrix4x4 *invertedMx = vtkMatrix4x4::New();
  vtkMatrix4x4::Invert (mx, invertedMx);
  vtkMatrix4x4 *transposedMx = vtkMatrix4x4::New();
  vtkMatrix4x4::Transpose(invertedMx, transposedMx);

  double cameraP[4] = {0, 0, 0, 1};
  double cameraD[4] = {0,0,0,0};
  double display[4] = {0, 0, 0, 0};
  double* dPickPoint;

	this->FindPokedRenderer(selectionX, selectionY);
	vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
	camera->GetPosition(cameraP);

	double cameraPVoxSpace [4] = {0, 0, 0, 1};
	invertedMx->MultiplyPoint (cameraP, cameraPVoxSpace);
	
	display[0]=selectionX;
	display[1]=selectionY;
	display[2]=0;//camera->GetDistance();
	this->CurrentRenderer->SetDisplayPoint(display);
	//this->CurrentRenderer->SetViewPoint(display);
	this->CurrentRenderer->DisplayToWorld();
	//this->CurrentRenderer->ViewToWorld();
	dPickPoint = this->CurrentRenderer->GetWorldPoint();

	for(int i=0;i<3;i++) 
		cameraD[i] = dPickPoint[i]-cameraP[i];
	vtkMath::Normalize(cameraD);
	
	double cameraDVoxSpace[4];

	transposedMx->MultiplyPoint (cameraD, cameraDVoxSpace);
	//for(int i=0;i<3;i++)
	//	dPickPoint[i]+=100*(cameraD[i]);
	//camera->GetDirectionOfProjection(cameraD);

	double d, s, P, D;
	switch(slice)
	{
	case DTI_ACTOR_SAGITTAL_TOMO:
		// Check for intersection with sagittal plane
		s = this->scene->curSSlice;
		P = cameraPVoxSpace[0];
		D = cameraDVoxSpace[0];
		break;
		case DTI_ACTOR_CORONAL_TOMO:
		// Check for intersection with sagittal plane
		s = this->scene->curCSlice;
		P = cameraPVoxSpace[1];
		D = cameraDVoxSpace[1];
		break;
		case DTI_ACTOR_AXIAL_TOMO:
		// Check for intersection with sagittal plane
		s = this->scene->curASlice;
		P = cameraPVoxSpace[2];
		D = cameraDVoxSpace[2];
		break;
	}
	d = (s - P) / D;
	double pickObj[4] = {0,0,0,1};
	for (int i=0; i < 3; i++) 
	{
		pickObj[i] = d*cameraDVoxSpace[i] + cameraPVoxSpace[i];
		// bounds checking
		if(pickObj[i] > dim[i])
			pickObj[i] = dim[i];
		if(pickObj[i] < 0)
				pickObj[i] = 0;
	}
	// now convert pick back to world coords:
	mx->MultiplyPoint (pickObj, pick);
}

vtkProp *vtkInteractorStyleDTI::getSceneProp (DTISceneActorID id) {
  switch (id) {
  case DTI_ACTOR_SAGITTAL_TOMO:
    return scene->sagittal;
  case DTI_ACTOR_AXIAL_TOMO:
    return scene->axial;
  case DTI_ACTOR_CORONAL_TOMO:
    return scene->coronal;
  case DTI_ACTOR_TRACTS:
    return pathwaysActor;
  case DTI_ACTOR_VOI:
    if (activeVOI != NULL) {
      return activeVOI->getActor();
    }
    else {
      return NULL;
    }
    break;
  default:
    cerr << "FATAL: Unsupported scene actor type!" << endl;
    return NULL;
  };
}


void vtkInteractorStyleDTI::ToggleVisibility(DTISceneActorID actorID) {
  vtkRenderWindowInteractor *rwi = this->Interactor;
  vtkProp *actor = getSceneProp (actorID);
  if (!actor) return;

  ToggleVisibility(actor);

  // adjust actorID, update tomo border
  if (actor->GetVisibility() && (actorID == DTI_ACTOR_SAGITTAL_TOMO ||
      actorID == DTI_ACTOR_AXIAL_TOMO ||
      actorID == DTI_ACTOR_CORONAL_TOMO)) {
    this->ActorSelected = actorID;
    DisplayTomoBorder(true);
  } else if ((int) actorID == this->ActorSelected) {
    DisplayTomoBorder(false);
    this->ActorSelected = DTI_ACTOR_NONE;
  }
  // if we made the tomo invisible, select the next visible one
  if (!actor->GetVisibility() && (actorID == DTI_ACTOR_SAGITTAL_TOMO ||
				 actorID == DTI_ACTOR_AXIAL_TOMO ||
				 actorID == DTI_ACTOR_CORONAL_TOMO)) {
    SelectNextVisiblePlane();
  }
  this->frame->RefreshVTK();
}

void 
vtkInteractorStyleDTI::ToggleVisibility(vtkProp *actor) {
  actor->SetVisibility(!actor->GetVisibility());
}

void
vtkInteractorStyleDTI::ToggleCorticalSurfaceVisibility()
{
  // XXX this should contact the meshes class
	  m_meshVisibility = (m_meshVisibility + 1) % 3;
	  if (m_meshVisibility == 0) {
	    if (!m_ghosting) m_smoothMeshActor->SetVisibility (true);
	    m_bumpyMeshActor->SetVisibility (false);
	    SetGhostVisibility (m_ghosting);
	    SetOverlayVisibility (true);
	    // turn on all surface patch actors
	  }
	  else if (m_meshVisibility == 1) {
	    m_smoothMeshActor->SetVisibility (false);
	    if (!m_ghosting) m_bumpyMeshActor->SetVisibility (true);
	    SetGhostVisibility (m_ghosting);
	    SetOverlayVisibility (false);
	    // turn off all surface patch actors
	  }
	  else {
	    m_smoothMeshActor->SetVisibility(false);
	    m_bumpyMeshActor->SetVisibility (false);
	    SetGhostVisibility (false);
	    SetOverlayVisibility (false);
	  }
}

void vtkInteractorStyleDTI::InitializeScenePathways()
{
	// Set up the scene pathways for the first time by recording
	// all polylines for the entire pathway database
	this->bInitializedScenePathways=true;

	pathwaysPolyData = vtkPolyData::New();
	vtkPoints* points = vtkPoints::New();
	vtkFloatArray* pcoords = vtkFloatArray::New();	
	vtkUnsignedCharArray* colors = vtkUnsignedCharArray::New();
	colors->SetNumberOfComponents(3);
	
	vtkCellArray* cells = vtkCellArray::New();
	double *pts;
	int i, j;
	DTIPathway* pathway;
	int currPoint;

	// This takes the given database and sets the pathway actor to use these new
	// pathways.
	// Note that by default, an array has 1 component.
	// We have to change it to 3 for points
	pcoords->SetNumberOfComponents(3);
	// We ask pcoords to allocate room for at least 4 tuples
	// and set the number of tuples to 4.
	//pcoords->SetNumberOfTuples(database->getPathway(0)->getNumPoints());
	currPoint=0;
	//pathway_startpt_list.push_back(0); 
	for(j=0;j<this->whole_database->getNumFibers();j++)
	{
		pathway = this->whole_database->getPathway(j);
		if(pathway->getNumPoints() > 0)
		{
			pts = new double[3*pathway->getNumPoints()];
			// Add a cell to handle this pathway
			cells->InsertNextCell(pathway->getNumPoints());

			// figure out what color we are going to give the next pathway
			// random jitter
			double vRand = this->GetPathwayJitter(pathway);
			double drgb[3],hsv[3];
			unsigned char rgb[3];
			std::vector<color>::iterator c;
			if(this->scene->curPathwayColoring==COLOR_STAIN)
			{
				c=this->pathway_colormap_class.begin();
				int g = this->GetPathwayGroup(pathway);
				c= c + g;
				drgb[0]=(*c).r/255.; 
				drgb[1]=(*c).g/255.; 
				drgb[2]=(*c).b/255.;
				vtkMath::RGBToHSV(drgb,hsv);
				//if(g==0)
				hsv[2]+=vRand;
				if( hsv[2] > 1)
					hsv[2]=1;
				if( hsv[2] < 0)
					hsv[2]=0;
				vtkMath::HSVToRGB(hsv,drgb);	
				rgb[0]=(unsigned char) floor(255 * drgb[0]);
				rgb[1]=(unsigned char) floor(255 * drgb[1]);
				rgb[2]=(unsigned char) floor(255 * drgb[2]);
			}
			colors->SetName("colors");
			for(i=0;i<pathway->getNumPoints();i++)
			{
				// Get a point from the pathway database
				pathway->getPoint(i,pts+3*i);
				//pts[3*i]*= this->scene->delta[0];
				//pts[3*i+1]*= this->scene->delta[1];
				//pts[3*i+2]*= this->scene->delta[2];
				// Insert the point into my point list
				pcoords->InsertTuple(currPoint, pts+3*i);
				cells->InsertCellPoint(currPoint);
				if(this->scene->curPathwayColoring==COLOR_LENGTH)
				{
					c=this->pathway_colormap_seq.begin();
					c = c + vtkMath::Floor(i/20.);
					rgb[0]=(*c).r; 
					rgb[1]=(*c).g; 
					rgb[2]=(*c).b;				
				}
				else if(this->scene->curPathwayColoring==COLOR_FA)
				{
					c=this->pathway_colormap_seq.begin();
					c = c + vtkMath::Floor(j/30.);
					rgb[0]=(*c).r; 
					rgb[1]=(*c).g; 
					rgb[2]=(*c).b;				
				}
				colors->InsertNextTuple3(rgb[0],rgb[1],rgb[2]);
				currPoint++;
			}
			delete []pts;
		}
		//pathway_startpt_list.push_back(pathway_startpt_list[j] + pathway->getNumPoints());
	}
	// Create vtkPoints and assign pcoords as the internal data array.
	points->SetData(pcoords);
	// Create the dataset. In this case, we create a vtkPolyData
	// Assign points and cells
	pathwaysPolyData->SetPoints(points);
	pathwaysPolyData->SetLines(cells);
	pathwaysPolyData->GetPointData()->SetScalars(colors);
	// Create the mapper 
	this->pathwaysMapper->SetInput(pathwaysPolyData);
	this->pathwaysMapper->ScalarVisibilityOn();
	// Create an actor.
	//pathwaysActor->GetProperty()->SetColor(0,0.8,0);
	pathwaysActor->GetProperty()->SetLineWidth(1.0);

	this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
		this->Interactor->GetEventPosition()[1]);
	if(this->CurrentRenderer != NULL)
		this->CurrentRenderer->AddActor(pathwaysActor);

	//polydata->Delete();
	points->Delete();
	pcoords->Delete();
	cells->Delete();
	colors->Delete();
}

void vtkInteractorStyleDTI::SetScenePathways( )
{
	this->current_database = this->whole_database;
	if (!this->bInitializedScenePathways) {
	  this->InitializeScenePathways();
	}

	DTIPathway* pathway;
	if(NumFPSMeasures == 10)
	  {
	    NumFPSMeasures = 0;
	    AvgFPS = 0;
	  }

// 	std::cout << "Timing ..." << std::endl;
// 	wxStopWatch sw;
// 	unsigned long passed = 0;
// 	for(int j=0;j<this->whole_database->getNumFibers();j++)
// 	  {
// 	    pathway = this->whole_database->getPathway(j);
// 	    if(this->activeVOI != NULL) {
// 	      if(this->activeVOI->filter->matches(pathway))
// 		passed++;
// 	    }
// 	  }
// 	double time = sw.Time();
// 	this->AvgFPS = this->AvgFPS * this->NumFPSMeasures + time;
// 	this->NumFPSMeasures++;
// 	this->AvgFPS/=this->NumFPSMeasures;
// 	wxLogMessage(" %d paths intersected the VOI and took %.2fms to execute",//%ldms to execute",
// 		     passed,this->AvgFPS);//sw.Time());
	

	vtkCellArray* cells = vtkCellArray::New();
	int pt_offset=0;
	int npts;
	int fiber_shown=0;
	for(int j=0;j<this->whole_database->getNumFibers();j++)
	{
		pathway = this->whole_database->getPathway(j);
		npts = pathway->getNumPoints();
		if( this->_query_processor->doesPathwayMatch(pathway) )
		{
			// Add a cell to handle this pathway
			cells->InsertNextCell( npts );
			for(int i=0;i<npts;i++)
				cells->InsertCellPoint(i+pt_offset);
			fiber_shown++;
		}
		pt_offset+=npts;
	}

	pathwaysPolyData->SetLines(cells);
	pathwaysPolyData->Modified();
	cells->Delete();
	this->scene->curShownPathways = fiber_shown;
	this->scene->ReportPositionFromTomos(scene->sagittal->GetUserMatrix());
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnMouseWheelForward() 
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  MoveCurrentTomogram (+1);
  vtkRenderWindowInteractor *rwi = this->Interactor;
  this->frame->RefreshVTK();
}

extern double sqr (double x);

struct eqint {
  bool operator() (int i1, int i2) {
    return i1 == i2; 
  }
};

// XXX This whole function should be in the meshes class
vector<int> *vtkInteractorStyleDTI::constructMeshPatch (vtkPolyData *inputData, int initialID, const double patchDim[3]) {
  hash_map<int, bool, hash<int>, eqint> visitedCells;

  visitedCells[initialID] = true;
  vector<int> *cellIDs = new vector<int>;
  list<int> priorityQueue;
  cerr << "pushing cellID " << initialID << " to start." << endl;
  priorityQueue.push_front (initialID);

  double seedPoint[3] = {0,0,0};
  bool firstTime = true;

  while (!priorityQueue.empty()) {
    bool cellMatches = false;
    // take the first thing off the queue:
    int cellID = *(priorityQueue.begin());
    priorityQueue.erase (priorityQueue.begin());
    vtkCell *cell = inputData->GetCell (cellID);
    vtkPoints *pts = inputData->GetPoints();
    double pt[3];
    bool distanceFail = false;
    for (int vertex = 0; vertex < 3; vertex++) {
     
      pts->GetPoint(cell->GetPointId(vertex), pt);
      if (firstTime) {
	cellMatches = true;
	// xxx hack Why?
	if (vertex == 2) firstTime = false;
	for (int i = 0; i < 3; i++) {
	  seedPoint[i] += 1.0/3.0*pt[i];
	}
      }
      else {
	double sqrDistances[3] = {sqr(pt[0]-seedPoint[0]),
			      sqr(pt[1]-seedPoint[1]),
			      sqr(pt[2]-seedPoint[2])};
	if (sqrDistances[0]/sqr(patchDim[0]/2.0) +
	    sqrDistances[1]/sqr(patchDim[1]/2.0) +
	    sqrDistances[2]/sqr(patchDim[2]/2.0) > 1) {
	  distanceFail = true;
	}
      }
    }
    if (!distanceFail) {
      cellMatches = true;
      cellIDs->push_back (cellID);
    }
    // add all neighbors to queue.
    if (cellMatches) {
      vtkIdList *neighboringVerts = vtkIdList::New();
      vtkIdList *neighboringCells = vtkIdList::New();
      for (int vert = 0; vert < 3; vert++) {
	neighboringVerts->InsertId(0,inputData->GetCell(cellID)->GetPointId(vert));
	inputData->GetCellNeighbors (cellID, neighboringVerts, neighboringCells);
	for (int c = 0; c < neighboringCells->GetNumberOfIds(); c++) {
	  int cellIDFound = neighboringCells->GetId(c);
	  if (visitedCells.find(cellIDFound) == visitedCells.end()) {
	    visitedCells[cellIDFound] = true;
	    priorityQueue.push_front (neighboringCells->GetId(c));
	  }
	}
      }
    }
  }
  return cellIDs;
   
}


//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::OnMouseWheelBackward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  MoveCurrentTomogram (-1);
  vtkRenderWindowInteractor *rwi = this->Interactor;
  this->frame->RefreshVTK();
}

int vtkInteractorStyleDTI::PickCell (vtkActor *actor, 
				     vtkCellLocator *locator,
				     int mouseX,
				     int mouseY,
				     double pickedPoint[3]) {

  double cameraP[3];
  double cameraD[3];
  double display[3];
  double lineStart[3];
  double lineEnd[3];
  double* dPickPoint;
  
  this->FindPokedRenderer(mouseX, mouseY);
  vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
  camera->GetPosition(cameraP);

  display[0]=mouseX;
  display[1]=mouseY;
  display[2]=0;
  this->CurrentRenderer->SetDisplayPoint(display);
  //this->CurrentRenderer->SetViewPoint(display);
  this->CurrentRenderer->DisplayToWorld();
  //this->CurrentRenderer->ViewToWorld();
  dPickPoint = this->CurrentRenderer->GetWorldPoint();
  
  for(int i=0;i<3;i++) {
    cameraD[i] = 12.0*(dPickPoint[i]-cameraP[i]);
    lineStart[i] = cameraP[i] - 12*cameraD[i];
    lineEnd[i] = 12*cameraD[i]+cameraP[i];
  }

  double t;
  int cellID;
  double b[3];
  int subID;
  cerr << "picking fast" << endl;
  locator->IntersectWithLine (cameraP, lineEnd, 0.0001, t, pickedPoint, b, subID, cellID);
  
  cerr << "done picking" << endl;
  return cellID;
  //  vtkMath::Normalize(cameraD);
  
#if 0
  		 
  vtkCellPicker *picker = vtkCellPicker::New();
  picker->SetTolerance (0.0001);
  picker->PickFromListOn();
  picker->AddPickList (actor);
	 
  cerr << "picking..." << endl;
  int id = picker->Pick (mouseX,
			 mouseY,
			 0.0,
			 this->CurrentRenderer);
  
  cerr << "done picking..." << endl;
  int cellID = picker->GetCellId();
  picker->Delete();
  return cellID;

#endif
}


// XXX communicate with the meshes class for this
void vtkInteractorStyleDTI::SetGhostVisibility (bool flag) {
  
  for(std::vector<VOI *>::iterator iter=this->voiList.begin();iter!=this->voiList.end();iter++) {
    VOI *current = *iter;
    if (current->GetSurfaceCellID() != -1) {
      if (flag == true) {
	vtkPolyData *currentPD;
	if (m_meshVisibility == 0) 
	  currentPD = scene->GetCortexMesh()->GetSmoothMesh();
	else
	  currentPD = scene->GetCortexMesh()->GetBumpyMesh();
	current->SetGhostActor (RebuildPatch (current,
					      currentPD,
					      true));
      }
      // Only set this if the actor has been created
      if(current->GetGhostActor()!=NULL)
	   current->GetGhostActor()->SetVisibility(flag);
    }
  }
}

// XXX communicate with the meshes class for this
void vtkInteractorStyleDTI::SetOverlayVisibility (bool flag) {
  for(std::vector<VOI *>::iterator iter=this->voiList.begin();iter!=this->voiList.end();iter++) {
      VOI *current = *iter;
      if (current->GetOverlayActor() != NULL) {
	if (flag == true) {
	  current->SetOverlayActor(RebuildPatch (current,
						 scene->GetCortexMesh()->GetSmoothMesh(),
						 false));
	}
	current->GetOverlayActor()->SetVisibility(flag);
	current->getActor()->SetVisibility(!flag); // Hide the sphere if we are drawing the overlay
      }
    }
}

// XXX communicate with the meshes class for this
vtkActor *vtkInteractorStyleDTI::GetCurrentSurfaceActor() {
  if (m_meshVisibility == 0) {
    return m_smoothMeshActor; }
  else {
    return m_bumpyMeshActor; }

}

// patchData controls how the patch will look... (bumpy or smooth)
// XXX communicate with the meshes class for this
vtkActor *vtkInteractorStyleDTI::RebuildPatch(VOI *voi, vtkPolyData *patchData, bool ghostPatch) 
{
  vtkPolyData *bumpyData = scene->GetCortexMesh()->GetBumpyMesh();

  // Dimensions of the ellipsoid whose intersection with the bumpy surface
  // forms the patch. These numbers measure all the way across, from one side 
  // to the other -- not the radius!)

  double patchDim[3];
  if (ghostPatch) {
    patchDim[0] = patchDim[1] = patchDim[2] = 40;
  }
  else {
    voi->GetDimensions(patchDim);
  }

  vector<int> *patchCells = constructMeshPatch (bumpyData, voi->GetSurfaceCellID(), patchDim);
  
  vtkActor *actor;
 
  if (ghostPatch) {
    actor = voi->GetGhostActor();
  }
  else {
    actor = voi->GetOverlayActor();
  }
 
  if (!actor) {
    vtkPolyData *newPolyData = vtkPolyData::New();
    newPolyData->SetPoints (patchData->GetPoints());
    actor = BuildActorFromMesh (newPolyData);
    //    voi->SetOverlayActor (actor);
    this->CurrentRenderer->AddActor (actor);
    this->CurrentRenderer->RemoveActor (voi->getActor());
    this->CurrentRenderer->AddActor (voi->getActor());
  }
  vtkCellArray *cellArray = vtkCellArray::New();
  cerr << "number of patch cells: " << patchCells->size() << endl;
  for (vector<int>::iterator iter = patchCells->begin(); iter != patchCells->end(); iter++) {
    int patchCellID = *iter;
    vtkCell *theCell = patchData->GetCell(patchCellID);
    vtkIdType cellVerts[3];
    for (int j = 0; j < 3; j++) {
      cellVerts[j] = theCell->GetPointId(j);
    }
    cellArray->InsertNextCell (3, cellVerts);
  }
  vtkPolyData *actorData = ((vtkPolyDataMapper *) actor->GetMapper())->GetInput();
  actorData->SetPolys (cellArray);
  if (ghostPatch) {
    actorData->SetPoints (patchData->GetPoints());
    actorData->GetPointData()->SetScalars (patchData->GetPointData()->GetScalars());
    actor->GetProperty()->SetOpacity(1.0);
    actor->GetProperty()->SetDiffuseColor(1, 1, 1);
  }
  else {
    actorData->GetPointData()->SetScalars (NULL);
    actor->GetProperty()->SetOpacity(0.5);
    actor->GetProperty()->SetDiffuseColor(0, 1, 0);
  }
  actorData->Modified();
  return actor;
}

void vtkInteractorStyleDTI::MoveVOI()
{
	double pick[3];
	
	//	cerr << "pos: " << this->Interactor->GetEventPosition()[0] << ", " << this->Interactor->GetEventPosition()[1] << endl;

	if(this->ActorSelected != DTI_ACTOR_SAGITTAL_TOMO && 
		this->ActorSelected != DTI_ACTOR_CORONAL_TOMO &&
		this->ActorSelected != DTI_ACTOR_AXIAL_TOMO &&
	        this->ActorSelected != DTI_ACTOR_SURFACE)
		return;

	if (this->ActorSelected == DTI_ACTOR_SURFACE && m_meshVisibility < 3) {
	  vtkActor *pickActor;
	  vtkActor *positionActor;
	  vtkCellLocator *pickLocator;
	  if (m_meshVisibility == 1) {
	    pickActor = this->GetBumpyMeshActor();
	    pickLocator = this->m_bumpyMeshLocator;
	    positionActor = pickActor;
	  }
	  else {
	    pickActor = this->GetSmoothMeshActor();
	    pickLocator = this->m_smoothMeshLocator;
	    positionActor = this->GetBumpyMeshActor(); 
	  }
	  

	  int cellID = this->PickCell (pickActor,
				       pickLocator,
				       this->Interactor->GetEventPosition()[0],
				       this->Interactor->GetEventPosition()[1],
				       pick);

	  activeVOI->SetSurfaceCellID(cellID);
	  vtkPolyData *data;
	  if (positionActor == this->GetSmoothMeshActor()) {
	    data = scene->GetCortexMesh()->GetSmoothMesh();;
	  }
	  else {
	    data = scene->GetCortexMesh()->GetBumpyMesh();
	  }
	  if (pickActor != this->GetBumpyMeshActor()) {
	    vtkCell *cell = data->GetCell (cellID);
	    if (cell != NULL) {
	      vtkPoints *pts = data->GetPoints();
	      pts->GetPoint(cell->GetPointId(0), pick);
	    }
	  }
	  
	  if (cellID == -1) {
	    return; 
	  }
	  else {
	    if (m_meshVisibility == 0) {
	      activeVOI->SetOverlayActor (RebuildPatch(activeVOI,
						       scene->GetCortexMesh()->GetSmoothMesh(),
						       false));
	    }
	    if (m_ghosting) {
	      vtkPolyData *patchData;
	      if (m_meshVisibility == 0) 
		patchData = scene->GetCortexMesh()->GetSmoothMesh();
	      else
		patchData = scene->GetCortexMesh()->GetBumpyMesh();
	      activeVOI->SetGhostActor (RebuildPatch (activeVOI,
						      patchData,
						      true));
	    }				         
	  }
	}
	else if (ActorSelected != DTI_ACTOR_SURFACE) {
	  this->SelectSlicePoint(this->ActorSelected,
				 this->Interactor->GetEventPosition()[0],
				 this->Interactor->GetEventPosition()[1],
				 pick);
	}

	//  if(activeVOI == NULL && this->whole_database != NULL)
//  	{
//  		// Insert a VOI here
//  		double length[3];
//  		length[0]=length[1]=length[2]=__initial_VOI_size;
//  		this->InsertVOI(pick,length, this->voiCounter);
//  		if(this->CurrentRenderer != NULL && activeVOI != NULL)
//  		{
//  			this->CurrentRenderer->AddActor(activeVOI->getActor());
//  			this->CurrentRenderer->AddActor2D((vtkProp*)activeVOI->getTextActor());
//  		}
//  	}
//  	else
		this->SetVOITranslation(pick[0],pick[1],pick[2]);

	this->broadcastActiveVOI();
	this->frame->RefreshVTK();

}


//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::SetVOIScale (double x, double y, double z) 
{
	if (!activeVOI) {
		return;
	}
	double scale[3] = {x,y,z};
	activeVOI->SetScale(scale);
	this->HighlightProp(activeVOI->getActor());
	MoveSymmetric (activeVOI);
	if(this->current_database != NULL) 
		{
		  //		  double data[3] = {x/scene->delta[0],y/scene->delta[1],z/scene->delta[2]};
		  //		  activeVOI->GetFilter()->setScale(data);
		  this->SetScenePathways();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::SetVOITranslation (double x, double y, double z) 
{
	if (!activeVOI) {
		return;
	}
	
	//x *= scene->delta[0];
	//y *= scene->delta[1];
	//z *= scene->delta[2];
	activeVOI->SetCenter(x,y,z);
	this->HighlightProp(activeVOI->getActor());
	MoveSymmetric (activeVOI);
	if(this->current_database != NULL) 
		{
		//this->current_database->removeFilter(activeVOI->filter);
		//this->current_database->addFilter(activeVOI->createFilter(scene->delta));
			
		this->SetScenePathways();
	}
	//this->EventCallbackCommand->SetAbortFlag(1);
}

void
vtkInteractorStyleDTI::MoveSymmetric (VOI *voi)
{
  if (!voi->getSymmetry()) return;
  
  VOI *other = voi->getSymmetry();

  double center[3];
  voi->GetCenter(center);
  
  center[0] = -center[0]; // reflect about the mid-sagittal plane

  other->SetCenter (center[0], center[1], center[2]);
  double scale[3];
  voi->GetDimensions(scale);
  other->SetScale (scale);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::SetMinLength (double minLength) 
{

	this->lengthFilter->setMinValue (minLength);
	this->SetScenePathways ( );
}


//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::SetMaxLength (double maxLength) 
{

	this->lengthFilter->setMaxValue (maxLength);
	this->SetScenePathways ( );
}



//----------------------------------------------------------------------------

void vtkInteractorStyleDTI::AddListener (InteractorListener *listener) {
	this->listeners.push_back (listener);
}


//----------------------------------------------------------------------------

void vtkInteractorStyleDTI::broadcastActiveVOI () 
{
  if (!activeVOI) {
    for (std::list<InteractorListener *>::iterator iter = this->listeners.begin(); iter != this->listeners.end(); iter++) {
      InteractorListener *listener = *iter;
      listener->interactorWindowVOIUpdate(0,0,0,0,0,0,0,"",0,NULL,VOI_MOTION_FREE, 0);
    }
    return;
  }
  double dim[3];
  activeVOI->GetDimensions(dim);

	int numVOIs = voiList.size();

	for (std::list<InteractorListener *>::iterator iter = this->listeners.begin(); iter != this->listeners.end(); iter++) {
		InteractorListener *listener = *iter;
		double center[3];
		activeVOI->GetCenter(center);
		const char *label = activeVOI->GetLabel();
		//sprintf (id, "v%d", activeVOI->getID());
		int *ids = new int[numVOIs];
		voiIter iter2 = voiList.begin();
		for (int i = 0; i < numVOIs; i++) {
			ids[i] = (*iter2)->getID();
			iter2++;
		}
		VOIMotionType motionType = VOI_MOTION_FREE;
		int motionID = 0;
		if (activeVOI->getSymmetry() != NULL) {
		  motionType = VOI_MOTION_SYMMETRY;
		  motionID = activeVOI->getSymmetry()->getID();
		}
		listener->interactorWindowVOIUpdate(center[0], 
						    center[1],
						    center[2],
						    dim[0],
						    dim[1],
						    dim[2],
						    activeVOI->getID(),
						    label,
						    numVOIs,
						    ids,
						    motionType,
						    motionID);
		delete[] ids;
	}
}

//----------------------------------------------------------------------------

void vtkInteractorStyleDTI::broadcastSliderValues () 
{
	double minLength = this->lengthFilter->getMinValue();
	double maxLength = this->lengthFilter->getMaxValue();
	double minFA = this->medianFAFilter->getMinValue();
	double maxFA = this->medianFAFilter->getMaxValue();
	double minCurv = this->meanCurvatureFilter->getMinValue();
	double maxCurv = this->meanCurvatureFilter->getMaxValue();
	
	for (std::list<InteractorListener *>::iterator iter = this->listeners.begin(); iter != this->listeners.end(); iter++) {
		InteractorListener *listener = *iter;
		listener->interactorWindowSliderUpdate(minLength, maxLength,
						       minFA, maxFA,
						       minCurv, maxCurv);
	}
}
void vtkInteractorStyleDTI::broadcastAlgoChoice () 
{
  DTIPathwayAlgorithm algo = this->algoFilter->getFilter();
	for (std::list<InteractorListener *>::iterator iter = this->listeners.begin(); iter != this->listeners.end(); iter++) {
		InteractorListener *listener = *iter;
		listener->interactorWindowAlgoUpdate(algo);
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::SetMinMedianFA(double value) 
{
	this->medianFAFilter->setMinValue (value);
	this->SetScenePathways ( );
}
//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::SetMaxMedianFA(double value) 
{
	this->medianFAFilter->setMaxValue (value);
	this->SetScenePathways ( );
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::SetVOILabel(const char *label)
{
	if (!activeVOI) {
		return;
	}
	/*	
	vtkRenderWindowInteractor *rwi = this->Interactor;
	if (strcmp (label, "")) {
	  activeVOI->getTextActor()->SetVisibility(true);
	}
	else {
	  activeVOI->getTextActor()->SetVisibility(false);
	}
	rwi->Render();*/
	activeVOI->SetLabel(label);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDTI::InitializePathwayJitter() {
	for(int j=0;j<this->whole_database->getNumFibers();j++)
	{
		DTIPathway *pathway = this->whole_database->getPathway(j);
		this->pathwayJittermap[pathway] = vtkMath::Random(-0.3, 0.2);
	}
}


DTIFilterVOI *vtkInteractorStyleDTI::getVOIFilter (const char *name) {
   static char toCompare[255];
   for(std::vector<VOI *>::iterator iter=this->voiList.begin();iter!=this->voiList.end();iter++) {
	  VOI *current = *iter;
	  //sprintf (toCompare, "v%d", current->getID());
	  sprintf (toCompare, "%d", current->getID());
	  if (!strcmp (toCompare, name)) {
		  return current->GetFilter();
	  }
   }
   return NULL;
}

void
vtkInteractorStyleDTI::rebuildFilters() {

  static char str[255];
  char *filterString = new char[255];
  strcpy (filterString, "");
  for(std::vector<VOI *>::iterator iter=this->voiList.begin();iter!=this->voiList.end();iter++) {
    VOI *current = *iter;
    if (iter != this->voiList.begin()) {
      filterString = strcat (filterString, _filter_op == DTI_FILTER_AND ? " and " : " or ");
    }
    sprintf (str, "%d", current->getID());
    filterString = strcat (filterString, str);
  }
  // update the actual filters
  //	this->current_database->getVOIFilters()->setVOIManager(this);
  //	this->current_database->getVOIFilters()->setFunc (filterString);
  this->_query_processor->setVOIFilter (filterString);
  
  // tell listeners that filter string has changed.
  broadcastFilterString (filterString);

}

//----------------------------------------------------------------------------

void vtkInteractorStyleDTI::broadcastFilterString (const char *str) 
{
	for (std::list<InteractorListener *>::iterator iter = this->listeners.begin(); iter != this->listeners.end(); iter++) {
		InteractorListener *listener = *iter;
		listener->interactorFilterStringUpdate (_filter_op, str);
	}
}

//----------------------------------------------------------------------------

bool
vtkInteractorStyleDTI::setFilterText (const char *str) 
{
  bool success = this->_query_processor->setVOIFilter ((char *) str);
  if (success) {
    this->SetScenePathways( );
  }
  return success;
}

//----------------------------------------------------------------------------

void
vtkInteractorStyleDTI::setFilterOp (DTIQueryOperation op) 
{
  _filter_op = op;
  if (op == DTI_FILTER_AND || op == DTI_FILTER_OR) {
	  rebuildFilters();
  }
  this->SetScenePathways( );
}

//----------------------------------------------------------------------------
void
vtkInteractorStyleDTI::SetAlgorithm (DTIPathwayAlgorithm algo) 
{
	this->algoFilter->setFilter (algo);
	this->SetScenePathways( );
}


//----------------------------------------------------------------------------

VOI *
vtkInteractorStyleDTI::findVOIByID(int id)
{
	for (voiIter iter = this->voiList.begin(); iter != this->voiList.end(); iter++) {
		VOI *v = *iter;
		if (v->getID() == id) {
			return v;
		}
	}
//	assert (false);
	return NULL;
}

//----------------------------------------------------------------------------
void
vtkInteractorStyleDTI::SetVOIMotion (VOIMotionType type, int id)
{
	if (!activeVOI) return;
	VOI *other = findVOIByID(id);
	if (id == activeVOI->getID()) {
	  if (activeVOI->getSymmetry()) {
	    activeVOI->getSymmetry()->setSymmetry(NULL);
	  }
		activeVOI->setSymmetry(NULL);
		return;
	}
	else {
		activeVOI->setSymmetry (findVOIByID(id));
		other->setSymmetry (activeVOI);
	}
	// make me like the other guy
	MoveSymmetric (other);
	this->broadcastActiveVOI();
	if (current_database != NULL) this->SetScenePathways ( );
}

void
vtkInteractorStyleDTI::NextLineWidth()
{
  this->lineWidthSetting++;
  if (this->lineWidthSetting > 1) {
	  this->lineWidthSetting = 0;
  }
  double lineWidth;
  switch (this->lineWidthSetting) {
  case 0:
    lineWidth = 1.0;
    break;
  case 1:
    lineWidth = 2.0;
    break;
  };

  this->pathwaysActor->GetProperty()->SetLineWidth(lineWidth);

  this->pathwaysActor->Modified();
}


void
vtkInteractorStyleDTI::SetMinMeanCurvature(double curv)
{
	this->meanCurvatureFilter->setMinValue (curv);
	this->SetScenePathways ( );
}

void
vtkInteractorStyleDTI::SetMaxMeanCurvature(double curv)
{
	this->meanCurvatureFilter->setMaxValue (curv);
	this->SetScenePathways ( );
}

void vtkInteractorStyleDTI::StainPathways()
{
  if(this->scene->curPathwayColoring == COLOR_STAIN)
    {
      if (curPathwayRegion != 0 && curPathwayRegion != 2 && curPathwayRegion != 5 && curPathwayRegion != 4) {
	this->curPathwayRegion++; }
      else if (curPathwayRegion == 2) {
	curPathwayRegion = 5; 
      }
      else if (curPathwayRegion == 0) {
	curPathwayRegion = 2;
      }
      else if (curPathwayRegion == 5) {
	curPathwayRegion = 3; }
      else if (curPathwayRegion == 4) {
	curPathwayRegion = 6; 
      }
      if (this->curPathwayRegion >= this->pathway_colormap_class.size())
	this->curPathwayRegion = 0;
      this->SetPathwayGroup(this->curPathwayRegion);
      this->SetScenePathways( );
    }
  this->frame->RefreshVTK();
}

void vtkInteractorStyleDTI::PickPoint()
{
  #if 0
    this->picker = vtkCellPicker::New();
  this->picker->SetTolerance(0.001);
  this->picker->PickFromListOn();
  //  this->picker->AddPickList (this->GetMeshActor());
  
  //this->Interactor->SetPicker (picker);
    int id = this->picker->Pick (this->Interactor->GetEventPosition()[0],
  		       this->Interactor->GetEventPosition()[1],
  		       0.0,
  		       this->CurrentRenderer);
  //int id = this->picker->Pick (0,0,0,this->CurrentRenderer);

    double p[3];
  this->picker->GetPickPosition (p);
  cerr << "pick position: " << p[0] << ", " << p[1] << ", " << p[2] << endl;

  //  this->sphereActor->SetPosition (p);
  cerr << "id is: " << this->picker->GetCellId() << endl;
  //  cerr << "foo" << endl;
  #endif
}


// XXX communicate with the meshes class for this
vtkActor *
vtkInteractorStyleDTI::BuildActorFromMesh (vtkPolyData *data) {
  vtkPolyDataMapper *meshMapper = vtkPolyDataMapper::New();
  meshMapper->SetInput(data);
  vtkActor *meshActor = vtkActor::New();
  meshActor->SetMapper(meshMapper);
  return meshActor;
}


// XXX communicate with the meshes class for this
vtkPolyData *
vtkInteractorStyleDTI::StripTriangles (vtkPolyData *meshPD) {

  vtkStripper *daveStripper = vtkStripper::New();
  daveStripper->SetInput(meshPD);
  vtkPolyData *output = daveStripper->GetOutput();
  output->SetReferenceCount(555);
  cerr << "meshing triangles..." << endl;	
  daveStripper->Update();
  cerr << "done meshing triangles!" << endl;
  daveStripper->Delete();
  meshPD->Delete();
  return output;
}

// XXX communicate with the meshes class for this
void
vtkInteractorStyleDTI::SetSmoothMesh (vtkPolyData *data) {
  cerr << "smooth mesh getting set." << endl;
  #ifdef TRIANGLE_STRIP
  vtkActor *actor = BuildActorFromMesh (StripTriangles(data));
  #else
  vtkActor *actor = BuildActorFromMesh (data);
  #endif
  m_smoothMeshActor = actor;
  m_smoothMeshLocator = vtkCellLocator::New();
  m_smoothMeshLocator->SetDataSet (data);
  m_smoothMeshLocator->BuildLocator();
}


// XXX communicate with the meshes class for this
void
vtkInteractorStyleDTI::SetBumpyMesh (vtkPolyData *data) {
  #ifdef TRIANGLE_STRIP
  vtkActor *actor = BuildActorFromMesh (StripTriangles(data));
  #else
  vtkActor *actor = BuildActorFromMesh (data);
  #endif
  m_bumpyMeshActor = actor;
  m_bumpyMeshLocator = vtkCellLocator::New();
  m_bumpyMeshLocator->SetDataSet (data);
  m_bumpyMeshLocator->BuildLocator();
}

void vtkInteractorStyleDTI::SavePathwayState(ostream &os)
{
  // Save out the color group for each pathway
	for(int j=0;j<this->whole_database->getNumFibers();j++)
	{
	  DTIPathway* pathway;
	  pathway = this->whole_database->getPathway(j);
	  short group = GetPathwayGroup(pathway); // Get the color group this pathway is in
	  os << group << " ";
	}
}

void vtkInteractorStyleDTI::RestorePathwayState(istream &is)
{
  // Restore the color group for each pathway

	DTIPathway* pathway;
	vtkUnsignedCharArray* colors = vtkUnsignedCharArray::New();
	colors->SetNumberOfComponents(3);

	for(int j=0;j<this->whole_database->getNumFibers();j++)
	{
		pathway = this->whole_database->getPathway(j);
		short g;
		is >> g; 
		this->pathwayVOImap[pathway] = g % this->pathway_colormap_class.size();

		// figure out what color we are going to give the next pathway
		// random jitter
		double vRand = this->GetPathwayJitter(pathway);
		double drgb[3],hsv[3];
		unsigned char rgb[3];
		std::vector<color>::iterator c;
		if(this->scene->curPathwayColoring==COLOR_STAIN)
		{
			c=this->pathway_colormap_class.begin();
			int group = this->GetPathwayGroup(pathway);
			c= c + group;
			drgb[0]=(*c).r/255.; 
			drgb[1]=(*c).g/255.; 
			drgb[2]=(*c).b/255.;
			vtkMath::RGBToHSV(drgb,hsv);
			//if(g==0)
			hsv[2]+=vRand;
			if( hsv[2] > 1)
				hsv[2]=1;
			if( hsv[2] < 0)
				hsv[2]=0;
			vtkMath::HSVToRGB(hsv,drgb);	
			rgb[0]=(unsigned char) floor(255 * drgb[0]);
			rgb[1]=(unsigned char) floor(255 * drgb[1]);
			rgb[2]=(unsigned char) floor(255 * drgb[2]);
		}
		for(int i=0;i<pathway->getNumPoints();i++)
			colors->InsertNextTuple3(rgb[0],rgb[1],rgb[2]);
	}
	colors->SetName("colors");

	pathwaysPolyData->GetPointData()->SetScalars(colors);
	colors->Delete();

	this->SetScenePathways( );
}

void vtkInteractorStyleDTI::SaveState()
{
  // Bogus renderer poke just to get the current renderer
  this->FindPokedRenderer(0,0);

  // Save the entire state of the system.

  // Use a file dialog to get filename

#if wxUSE_UNICODE
  wxString defaultDir (_data_directory_path, wxConvUTF8);
#else
  wxString defaultDir (_data_directory_path);
#endif
  //char defaultDir[255];
  //sprintf (defaultDir, "%s", _data_directory_path);
  wxFileDialog fd(this->frame,_T("Choose state filename."),defaultDir,_T(""),_T("Query State (*.qst)|*.qst"),wxSAVE | wxOVERWRITE_PROMPT, wxDefaultPosition);
  fd.SetDirectory (defaultDir);
  if(fd.ShowModal() != wxID_OK)
    return;
  char pathToUse[255];
  const char *pathReturned = fd.GetPath().mb_str();
  if (!strstr (pathReturned, ".qst")) {
    sprintf (pathToUse, "%s.qst", pathReturned);
  } 
  else {
    strcpy (pathToUse, pathReturned);
  }  
  std::ofstream stream(pathToUse);
  stream << "%% dtivis software version: " << _version << std::endl;

  // Save Camera
  double pos[3];
  double viewup[3];
  double focalpt[3];

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

  camera->GetPosition(pos);
  camera->GetViewUp(viewup);
  camera->GetFocalPoint(focalpt);

  stream << "Camera Position: " << pos[0] << "," << pos[1] << "," << pos[2] << std::endl;
  stream << "Camera View Up: " << viewup[0] << "," << viewup[1] << "," << viewup[2] << std::endl;
  stream << "Camera Focal Point: " << focalpt[0] << "," << focalpt[1] << "," << focalpt[2] << std::endl;

  // Query operation string
  stream << "Query Mode: " << _filter_op << std::endl;
  stream << "Query String:" << this->_query_processor->getVOIFilter() << std::endl;

  // Save Units
  stream << "Units: " << scene->curUnits << std::endl;
  // Tomo Position
  stream << "Tomo. Position: " << scene->curSSlice << "," << scene->curCSlice << "," << scene->curASlice << std::endl;

  stream << "Tomo. Visibility: " << this->scene->sagittal->GetVisibility() << "," << this->scene->coronal->GetVisibility() <<"," << this->scene->axial->GetVisibility() << std::endl;

  // VOI information
  stream << "No. VOIs: " << this->voiList.size() << std::endl;
  for(voiIter i = voiList.begin(); i != voiList.end(); i++)
    {
      double scale[3];
      double center[3];
      int id;
      (*i)->GetDimensions(scale);
      (*i)->GetCenter(center);
      id = (*i)->getID();
      stream << '\t' << "VOI Scale: " << scale[0] << "," << scale[1] << "," << scale[2] << std::endl;
      stream << '\t' << "VOI Center: " << center[0] << "," << center[1] << "," << center[2] << std::endl;      
      stream << '\t' << "VOI ID: " << id << std::endl;
      stream << std::endl;
    }

  stream << "Pathway Min Length: " << this->lengthFilter->getMinValue() << endl;
  stream << "Pathway Max Length: " << this->lengthFilter->getMaxValue() << endl;
  stream << "Pathway Min FA: " << this->medianFAFilter->getMinValue() << endl;
  stream << "Pathway Max FA: " << this->medianFAFilter->getMaxValue() << endl;
  stream << "Pathway Min Curvature: " << this->meanCurvatureFilter->getMinValue() << endl;
  stream << "Pathway Max Curvature: " << this->meanCurvatureFilter->getMaxValue() << endl;

  stream << "Algorithm: " << this->algoFilter->getFilter() << endl;

  this->SavePathwayState(stream);
  stream << std::endl;
  
  stream.close();
}

void ignoreLine(istream& is) {
  is.ignore(numeric_limits<std::streamsize>::max(),'\n');
}

void ignoreBeforeColon(istream& is) {
  is.ignore(numeric_limits<std::streamsize>::max(),':');
}

void getValue(istream &is, float& v) {
  ignoreBeforeColon(is);
  is >> v;
  ignoreLine(is);
}

void getValue(istream &is, double& v) {
  ignoreBeforeColon(is);
  is >> v;
  ignoreLine(is);
}

void getValue(istream &is, char *foo) {
  ignoreBeforeColon(is);
  is.getline(foo, 255);
}

void getValue(istream &is, int& v) {
  ignoreBeforeColon(is);
  is >> v;
  ignoreLine(is);
}
void getValue(istream &is, unsigned int& v) {
  ignoreBeforeColon(is);
  is >> v;
  ignoreLine(is);
}
void getValue(istream &is, unsigned int v[3]) {
   ignoreBeforeColon(is);
   char c;
   is >> v[0] >> c >> v[1] >> c >> v[2];
   ignoreLine(is);
}
void getValue(istream &is, double v[3]) {
   ignoreBeforeColon(is);
   char c;
   is >> v[0] >> c >> v[1] >> c >> v[2];
   ignoreLine(is);
}
void getValue(istream &is, bool v[3]) {
  ignoreBeforeColon(is);
  char c;
  is >> v[0] >> c >> v[1] >> c >> v[2];
  ignoreLine(is);
}


void vtkInteractorStyleDTI::ResetState()
{
  // Reset the state of the filtering system to nothing.
  voiIter iter;
  
  if(!voiList.empty()) {
    // First remove all the scene components of the VOIs  
    for(iter=this->voiList.begin();iter!=this->voiList.end() && activeVOI != NULL;iter++)
      {
	if(this->CurrentRenderer != NULL) 
	  {
	    this->CurrentRenderer->RemoveActor((*iter)->getActor());
	    this->CurrentRenderer->RemoveActor((vtkProp*)(*iter)->getTextActor());
	  }
	delete (*iter);
      }
    activeVOI = NULL;
    //    this->voiList.erase(this->voiList.begin(),this->voiList.end());
    this->voiList.clear();
    
    
    // Notify the GUI that the filters are gone
    broadcastActiveVOI();
    this->_query_processor->resetVOIFilter(this);

    // Refilter the pathways
    if(this->current_database != NULL) 
      {
	if (_filter_op == DTI_FILTER_AND || _filter_op == DTI_FILTER_OR) {
	  rebuildFilters();
      }
	//this->SetScenePathways( );
      }
    // Remove previous highlights
    this->HighlightProp(NULL);
    // Automatically go into axial slice move mode
    this->ActorSelected = DTI_ACTOR_AXIAL_TOMO;
  }
}

void vtkInteractorStyleDTI::RestoreState()
{
  // Restore the entire state of the system.

  // First lets do an artificial state restore without looking at
  // the saved file
  unsigned int ver;
  double pos[3];
  double viewup[3];
  double focalpt[3];
  unsigned int tomoPos[3];
  bool tomoVis[3];
  unsigned int nVOIs;
  int units;

  char filterString[255];
  int filterOp;

  double minPathwayLength;
  double maxPathwayLength;

#if wxUSE_UNICODE
  wxString defaultDir (_data_directory_path, wxConvUTF8);
#else
  wxString defaultDir (_data_directory_path);
#endif
 // char defaultDir[255];
 // sprintf (defaultDir, "%s", _data_directory_path);

  // Use a file dialog to get filename
  wxFileDialog fd(this->frame,_T("Choose a program-state file to load:"),defaultDir,_T(""),_T("Query State (*.qst)|*.qst"),0,wxDefaultPosition);
  fd.SetDirectory (defaultDir);
  if(fd.ShowModal() != wxID_OK)
    return;

  std::ifstream stream(fd.GetPath().mb_str());
  getValue(stream,ver);		// Version
  getValue(stream,pos);		// Camera Position
  getValue(stream,viewup);	// Camera View Up
  getValue(stream,focalpt);	// Camera Focal Pt.

  getValue(stream,filterOp);    // Query filter op
  getValue(stream,filterString); // Query string

  getValue(stream,units);	// Current Unit System
  getValue(stream,tomoPos);	// Tomogram positions
  getValue(stream,tomoVis);     // Tomogram visibility
  getValue(stream,nVOIs);	// Number of VOIS
  

  // Restore Camera

  // Bogus renderer poke just to get the current renderer
  this->FindPokedRenderer(0,0);

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

  camera->SetPosition(pos);
  camera->SetViewUp(viewup);
  camera->SetFocalPoint(focalpt);
  camera->ComputeViewPlaneNormal();

  // Restore Units
  while(units != scene->curUnits) {
    this->scene->NextUnits();
  }
  for (std::list<InteractorListener *>::iterator iiter = this->listeners.begin(); iiter != this->listeners.end(); iiter++) {
    InteractorListener *listener = *iiter;
    listener->interactorWindowUnitsChanged (this->scene->curUnits);
  }
 
  if (GetCurrentBackground()) {
    unsigned int dim[4];
    GetCurrentBackground()->getDimension(dim[0], dim[1], dim[2], dim[3]);
    
    // Restore Tomos
    scene->curSSlice = tomoPos[0];
    if(scene->curSSlice >= dim[0])
      scene->curSSlice = dim[0]-1;
    scene->sagittal->SetDisplayExtent(scene->curSSlice,scene->curSSlice, 0,dim[1]-1, 0,dim[2]-1);
    scene->sagittal->Modified();
    
    scene->curCSlice = tomoPos[1];
    if(scene->curCSlice >= dim[0])
      scene->curCSlice = dim[0]-1;
    scene->coronal->SetDisplayExtent(0,dim[0]-1,scene->curCSlice,scene->curCSlice, 0,dim[2]-1);
    scene->coronal->Modified();
    
    scene->curASlice = tomoPos[2];
    if(scene->curASlice >= dim[0])
      scene->curASlice = dim[0]-1;
    scene->axial->SetDisplayExtent(0,dim[0]-1, 0,dim[1]-1,scene->curASlice,scene->curASlice);
    scene->axial->Modified();
    this->scene->ReportPositionFromTomos(scene->sagittal->GetUserMatrix());
    
    scene->sagittal->SetVisibility(tomoVis[0]);
    scene->coronal->SetVisibility(tomoVis[1]);
    scene->axial->SetVisibility(tomoVis[2]);
  }

  //  cerr << "about to reset state." << endl;

  // Now remove the previous VOIs
  this->ResetState();

  //  cerr << "state reset." << endl;

  // This call just refreshes the screen
  this->StopState();
  // Insert the new VOIs

  int maxVOIID = 0;

  for(int i=0; i<nVOIs; i++) {
      double scale[3];
      double center[3];    
      int id;
      getValue(stream,scale);	// VOI Length
      getValue(stream,center);	// VOI Position
      getValue(stream,id);      // VOI ID
      maxVOIID = id > maxVOIID ? id : maxVOIID;
      ignoreLine(stream);
      this->InsertVOI(center,scale,id);
      //      this->InsertVOI(center,scale,this->voiCounter);

      if(this->CurrentRenderer != NULL && activeVOI != NULL)
	{
	  this->CurrentRenderer->AddActor(activeVOI->getActor());
	  this->CurrentRenderer->AddActor2D((vtkProp*)activeVOI->getTextActor());
	}
      // Automatically go int VOI size adjusting mode
      //      this->ActorSelected = DTI_ACTOR_VOI;
  }
  this->voiCounter = maxVOIID+1;

  getValue(stream,minPathwayLength); // minimum length of pathways
  getValue(stream,maxPathwayLength); // maximum length of pathways

  double minFA, maxFA, minCurv, maxCurv;

  getValue(stream,minFA);
  getValue(stream,maxFA);

  getValue(stream,minCurv);
  getValue(stream,maxCurv);

  int algoChoice;
  getValue (stream, algoChoice);

  // Must do this in case the border was being displayed in the scene before we restore
  // has nothing to do with the saved border condition
  if(this->TomoBorderActor!=NULL)
    DisplayTomoBorder(this->TomoBorderActor->GetVisibility());

  RestorePathwayState(stream);

  // Restore query filter op

  setFilterOp ((DTIQueryOperation) filterOp);
  this->_query_processor->setVOIFilter (filterString);
  broadcastFilterString (filterString);

  this->lengthFilter->setMinValue(minPathwayLength);
  this->lengthFilter->setMaxValue(maxPathwayLength);
  this->medianFAFilter->setMinValue(minFA);
  this->medianFAFilter->setMaxValue(maxFA);
  this->meanCurvatureFilter->setMinValue(minCurv);
  this->meanCurvatureFilter->setMaxValue(maxCurv);
  this->algoFilter->setFilter(algoChoice);

  this->broadcastSliderValues();
  this->broadcastAlgoChoice();
  //  SetMinLength (minPathwayLength);
  //  SetMaxLength (maxPathwayLength);
  this->SetScenePathways();
  stream.close();

  this->CurrentRenderer->ResetCameraClippingRange();
  this->frame->RefreshVTK ();
  
}


void
vtkInteractorStyleDTI::LoadPathways() 
{
#if wxUSE_UNICODE
  wxString defaultDir (_data_directory_path, wxConvUTF8);
#else
  wxString defaultDir (_data_directory_path);
#endif
  wxFileDialog fd(this->frame,_T("Choose a pathway file to load:"),defaultDir,_T(""),_T("Pathways (*.pdb)|*.pdb"),0,wxDefaultPosition);
  fd.SetDirectory (defaultDir);
  if(fd.ShowModal() != wxID_OK)
    return;
  
  std::ifstream stream(fd.GetPath().mb_str(), ios::binary);
  if (!stream) return;

  LoadPathwaysInternal (stream);
  stream.close();

  this->frame->RefreshVTK ();
  
  frame->PathwaysLoaded (fd.GetPath().mb_str());

}


void
vtkInteractorStyleDTI::SavePathwaysPDB() 
{
  // Choose filename
#if wxUSE_UNICODE
  wxString defaultDir (_data_directory_path, wxConvUTF8);
#else
  wxString defaultDir (_data_directory_path);
#endif
  wxFileDialog fd(this->frame,_T("Choose pathway database filename."),defaultDir,_T(""),_T("Pathways (*.pdb)|*.pdb"),wxSAVE | wxOVERWRITE_PROMPT, wxDefaultPosition);
  fd.SetDirectory (defaultDir);

  if(fd.ShowModal() != wxID_OK)
    return;

  // Create output DB
  DTIPathwayDatabase *dbOut = new DTIPathwayDatabase();
  double mmScale[3];
  whole_database->getVoxelSize(mmScale);
  dbOut->setVoxelSize(mmScale);
  unsigned int dim[3];
  whole_database->getSceneDimension(dim);
  dbOut->setSceneDimension(dim);
  dbOut->setTransformMatrix (whole_database->getTransformMatrix());
  for(int hh=0; hh<whole_database->getNumAlgorithms(); hh++)
    dbOut->addAlgorithmHeader(whole_database->getPathAlgorithmHeader(hh));
  for(int hh=0; hh<whole_database->getNumPathStatistics(); hh++)
    dbOut->addStatisticHeader(whole_database->getPathStatisticHeader(hh));


// Find the visible fibers
  int numSaved = 0;
  for(int j=0;j<this->whole_database->getNumFibers();j++) {
    DTIPathway* pathway = this->whole_database->getPathway(j);
    if (this->_query_processor->doesPathwayMatch(pathway)) {
      dbOut->addPathway(pathway);
      numSaved++;
    }
  }
  char pathToUse[255];
  const char *pathReturned = fd.GetPath().mb_str();
  if (!strstr (pathReturned, ".pdb")) {
    sprintf (pathToUse, "%s.pdb", pathReturned);
  } 
  else {
    strcpy (pathToUse, pathReturned);
  }  
  // Write the database to a file
  double ACPC[3] = {0,0,0};
  string strFilename = pathToUse;
  DTIPathwayIO::saveDatabaseAny(dbOut, true, strFilename, ACPC);
  // cleanup
  dbOut->clearAllPathways();
  delete dbOut;

  //OLD

//   DTIPathwaySelection *selection = new DTIPathwaySelection (this->whole_database->getNumFibers(), false);  
//   // Find the visible fibers
//   int numSaved = 0;
//   for(int j=0;j<this->whole_database->getNumFibers();j++) {
//     DTIPathway* pathway = this->whole_database->getPathway(j);
//     if (this->_query_processor->doesPathwayMatch(pathway)) {
//       selection->setValue (j, true);
//       numSaved++;
//     }
//     else {
//       selection->setValue (j, false);
//     }
//   }
//   char pathToUse[255];
//   const char *pathReturned = fd.GetPath().mb_str();
//   if (!strstr (pathReturned, ".pdb")) {
//     sprintf (pathToUse, "%s.pdb", pathReturned);
//   } 
//   else {
//     strcpy (pathToUse, pathReturned);
//   }  

//   //DTIPathwayIO::openDatabase (this->whole_database, pathToUse);
//   //uint64_t *offsets = DTIPathwayIO::appendDatabaseFile (this->whole_database, pathToUse, selection);
//   //DTIPathwayIO::appendFileOffsetsToDatabaseFile (numSaved, offsets, pathToUse); 
//   // XXX NOT USING SELECTION
//   //DTIPathwayIO::appendDatabaseFile (this->whole_database, pathToUse);
}

void
vtkInteractorStyleDTI::SavePathwaysImage() 
{
  // Choose filename

#if wxUSE_UNICODE
  wxString defaultDir (_data_directory_path, wxConvUTF8);
#else
  wxString defaultDir (_data_directory_path);
#endif
  wxFileDialog fd(this->frame,_T("Choose image filename."),defaultDir,_T(""),_T("NIFTI (*.nii;*.nii.gz;*.hdr)|*.nii;*.nii.gz;*.hdr"),wxSAVE | wxOVERWRITE_PROMPT, wxDefaultPosition);
  fd.SetDirectory (defaultDir);
  if(fd.ShowModal() != wxID_OK)
    return;
			      
  DTIPathwayIntersectionVolume *vol = new DTIPathwayIntersectionVolume (GetCurrentBackground());
  
  // Find the visible fibers
  std::vector< DTIPathway* > pathsToSave;
  for(int j=0;j<this->whole_database->getNumFibers();j++) {
    DTIPathway* pathway = this->whole_database->getPathway(j);
    if (this->_query_processor->doesPathwayMatch(pathway)) {
      vol->munchPathway(pathway);
    }
  }

  vol->normalize();
  DTIVolumeIO::saveVolumeNifti (vol, fd.GetPath().mb_str());  
}


bool
//vtkInteractorStyleDTI::LoadBackgroundInternal(ifstream &stream) 
vtkInteractorStyleDTI::LoadBackgroundInternal(const char* filename, DTIScalarVolume *oldVol) 
{

  this->FindPokedRenderer(0, 0);
  //    cerr << "calling loadScalarVolumeNifti..." << endl;

  DTIScalarVolume *scalarVol = DTIVolumeIO::loadScalarVolumeNifti (filename);
  //    cerr << "Done calling loadDTIScalarVolumeNifti!" << endl;
  if(scalarVol==NULL)
    return false; // error loading file

  if (scalarVol->getSformCode()) { scene->matrixCode = scalarVol->getSformCode(); }
  else {
    scene->matrixCode = scalarVol->getQformCode();
  }
  //cerr << "matrixCode: " << scene->matrixCode << endl;

  //  cerr << "Loaded file." << endl;
  //  this->scalarVolumeList.push_back (scalarVol);
  //  cerr << "about to set scalarvolume" << endl;
  //  cerr << this->current_background_id << endl;
  //  cerr << "scalarvolumelist size: " << scalarVolumeList.size() << endl;
  this->scalarVolumeList[this->current_background_id] = scalarVol;
  //  cerr << "setting scalarvolume list at " << this->current_background_id << endl;
    //  current_background_id = this->scalarVolumeList.size()-1;

  strcpy (scene->backgroundFilename, (*_background_filenames)[current_background_id]);

  unsigned int dim[4];
  scalarVol->getDimension(dim[0], dim[1], dim[2], dim[3]);
  double voxSize[3];
  scalarVol->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
  if (oldVol == NULL) {
    scene->curSSlice = dim[0]/2;
    scene->curCSlice = dim[1]/2;
    scene->curASlice = dim[2]/2;
  }

  SetTomograms(scalarVol, oldVol);

  this->scene->ReportPositionFromTomos(scene->sagittal->GetUserMatrix());
  return true;
  
}


void
vtkInteractorStyleDTI::UpdateLUT (double window, double level) {
   _bw_lut->SetValueRange (0, 1);
   _bw_lut->SetTableRange (level - window/2.0, level + window/2.0);
   _bw_lut->Modified();
}


void 
vtkInteractorStyleDTI::InitializeTomograms()
{
   this->bInitializedTomograms = true;

  _bw_lut = vtkLookupTable::New();

  _bw_lut->SetRampToLinear( );
  _bw_lut->SetSaturationRange (0, 0);
  _bw_lut->SetHueRange (0, 0);
 
  _color_lut = vtkColorTransferFunction::New();
  _color_lut->SetColorSpaceToRGB();
  _color_lut->AddRGBSegment  	(0,
				 0,0,0,
				 1,
				 1,1,1);
  //  _bw_lut->SetRampToLinear( );
  //  _bw_lut->SetSaturationRange (0, 0);
  //  _bw_lut->SetHueRange (0, 0);

  scene->sagittal = vtkImageActor::New();
  scene->axial = vtkImageActor::New();
  scene->coronal = vtkImageActor::New();

  this->CurrentRenderer->AddActor(scene->sagittal);
  this->CurrentRenderer->AddActor(scene->axial);
  this->CurrentRenderer->AddActor(scene->coronal);

  //UpdateLUT (_lut_window, _lut_level);

  scene->sagittal->Delete();
  scene->axial->Delete();
  scene->coronal->Delete();
}


void 
vtkInteractorStyleDTI::SetTomograms(DTIScalarVolume *scalarVol, DTIScalarVolume *oldVol)
{
  
  //  this->CurrentRenderer->GetActiveCamera()->SetWindowCenter (0.28, 0);
  // this breaks depth computations
  scalarVol->getVoxelSize(scene->bVoxSize[0], scene->bVoxSize[1], scene->bVoxSize[2]);

  if (!this->bInitializedTomograms) {
    InitializeTomograms();
  }

  _bw_lut->SetTableRange (scalarVol->getCalcMinVal(), scalarVol->getCalcMaxVal());
  _bw_lut->SetValueRange (0, 1);
  //scalarVol->getCalcMinVal(), scalarVol->getCalcMaxVal());
  _bw_lut->Modified();
  //  cerr << "bw-lut: " << (double) scalarVol->getCalcMinVal() << " " << (double) scalarVol->getCalcMaxVal() << endl;

  vtkMatrix4x4 *mx = vtkMatrix4x4::New();
  mx->DeepCopy (scalarVol->getTransformMatrix());
  scene->sagittal->SetUserMatrix (mx);
  scene->coronal->SetUserMatrix (mx);
  scene->axial->SetUserMatrix (mx);
  
  mx->Delete();
  
  unsigned int dim[4];
  double voxSize[3];
  scalarVol->getDimension(dim[0], dim[1], dim[2], dim[3]);
  scalarVol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);

  vtkScalarsToColors *lut = _bw_lut;

  fa_imagedata = vtkImageData::New();
   if (dim[3] == 1) {
    //    lut = _bw_lut;
    //fa_imagedata->SetScalarTypeToDouble();
     fa_imagedata->SetScalarTypeToFloat();
  }
  else {
    fa_imagedata->SetScalarTypeToUnsignedChar();
    //    lut = _color_lut;
  }
  fa_imagedata->SetNumberOfScalarComponents(dim[3]);
 
  this->fa_imagedata->SetDimensions (dim[0], dim[1], dim[2]);
  this->fa_imagedata->SetSpacing (1,1,1);
  //  this->fa_imagedata->SetSpacing(voxSize[0], voxSize[1], voxSize[2]);
  this->fa_imagedata->AllocateScalars();
  if (dim[3] == 1) {
    vtkImageMapToColors *sagittalColors = vtkImageMapToColors::New();
    sagittalColors->SetInput(fa_imagedata);
    sagittalColors->SetLookupTable(lut);
    scene->sagittal->SetInput(sagittalColors->GetOutput());
    sagittalColors->Delete();
  }
  else {
    scene->sagittal->SetInput(fa_imagedata);
  }
  
  if (dim[3] == 1) {
    vtkImageMapToColors *axialColors = vtkImageMapToColors::New();
    axialColors->SetInput(fa_imagedata);
    axialColors->SetLookupTable(lut);
    scene->axial->SetInput(axialColors->GetOutput());
    axialColors->Delete();
  }
  else {
    scene->axial->SetInput(fa_imagedata);
  }
  if (dim[3] == 1) {
    vtkImageMapToColors *coronalColors = vtkImageMapToColors::New();
    coronalColors->SetInput(fa_imagedata);
    coronalColors->SetLookupTable(lut);
    scene->coronal->SetInput(coronalColors->GetOutput());
    coronalColors->Delete();
  }
  else {
    scene->coronal->SetInput(fa_imagedata);
  }

  if (oldVol != NULL) {
    double oldVoxSize[3];
    oldVol->getVoxelSize (oldVoxSize[0], oldVoxSize[1], oldVoxSize[2]);
    scene->curSSlice = (int) floor (scene->curSSlice*oldVoxSize[0]/voxSize[0]);
    if (scene->curSSlice > dim[0]-1) scene->curSSlice = dim[0]-1;
    scene->curCSlice = (int) floor (scene->curCSlice*oldVoxSize[1]/voxSize[1]);
    if (scene->curCSlice > dim[1]-1) scene->curCSlice = dim[1]-1;
    scene->curASlice = (int) floor (scene->curASlice*oldVoxSize[2]/voxSize[2]);
    if (scene->curASlice > dim[2]-1) scene->curASlice = dim[2]-1;
  }

  this->fa_imagedata->Modified();

  scene->sagittal->SetDisplayExtent(scene->curSSlice,scene->curSSlice, 0, dim[1]-1, 0, dim[2]-1);

  scene->axial->SetDisplayExtent(0,dim[0]-1, 0,dim[1]-1, scene->curASlice,scene->curASlice);

  scene->coronal->SetDisplayExtent(0,dim[0]-1, scene->curCSlice,scene->curCSlice, 0,dim[2]-1);

  if (dim[3] == 1) {
    float *dataPtr = scalarVol->getDataPointer();
    float *destinationPtr = (float *)fa_imagedata->GetScalarPointer();  
    memcpy (destinationPtr, dataPtr, dim[0]*dim[1]*dim[2]*dim[3]*sizeof(float));
  }
  else {  
    unsigned int i,j,k,c;
    for(k=0;k<dim[2];k++) {
      for(j=0;j<dim[1];j++) {
	for(i=0;i<dim[0];i++){
	  for (c = 0; c<dim[3];c++) {
	    float faValue = scalarVol->getScalar(i,j,k,c)*255;
	    this->fa_imagedata->SetScalarComponentFromFloat (i,j,k,c,faValue);
	  }
	}
      }
    }
  }

  scene->axial->Modified();
  scene->coronal->Modified();
  scene->sagittal->Modified();

  if (!oldVol) {
    vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
    vtkMatrix4x4 *mx = this->scene->coronal->GetUserMatrix();
    this->camera_controls->Update (camera, dim, voxSize, mx);
    this->CurrentRenderer->ResetCameraClippingRange();
  }

  fa_imagedata->Delete();
}

void
vtkInteractorStyleDTI::ResetView()
{
  DTIScalarVolume *scalarVol = GetCurrentBackground();
  if (!scalarVol) return; 

  unsigned int dim[4];
  scalarVol->getDimension (dim[0], dim[1], dim[2], dim[3]);
  double voxSize[3];
  scalarVol->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  vtkMatrix4x4 *mx = this->scene->coronal->GetUserMatrix();

  this->camera_controls->ResetPosition();
  this->camera_controls->Update (camera, dim, voxSize, mx);
  this->CurrentRenderer->ResetCameraClippingRange();
  this->frame->RefreshVTK();
}

void
vtkInteractorStyleDTI::SetScene (Scene *sceneParam) 
{
  this->scene = sceneParam;
  delete this->camera_controls;
  this->camera_controls = new CameraControls();
}

DTIScalarVolume *
vtkInteractorStyleDTI::GetCurrentBackground()
{
  if (current_background_id != -1 && current_background_id < scalarVolumeList.size()) {
    return scalarVolumeList[current_background_id];
  }
  else {
    return NULL;
  }
}

void
vtkInteractorStyleDTI::CycleBackground(bool dir)
{
  
  DTIScalarVolume *oldBackground = GetCurrentBackground();
  if (dir) {
    current_background_id++;
  }
  else {
    current_background_id--;
  }
  if (current_background_id < 0) {
    current_background_id = this->scalarVolumeList.size() -1;
  }
  else if (current_background_id > this->scalarVolumeList.size() -1) {
    current_background_id = 0;
  }
  //  cerr << "cur background id: " << current_background_id << endl;
    // if (this->scalarVolumeList.size() < _background_paths->GetCount()) {
  if (!GetCurrentBackground()) {
    //    cerr << "no cur background" << endl;
    wxString filename = (*_background_paths)[this->current_background_id];
    LoadBackgroundInternal (filename.mb_str(), oldBackground);
  }
  else {
     SetTomograms (GetCurrentBackground(), oldBackground);
     strcpy (scene->backgroundFilename, (*_background_filenames)[current_background_id]);
  }
  
  scene->ReportPositionFromTomos (scene->sagittal->GetUserMatrix());
  DisplayTomoBorder(true);
  
  this->frame->RefreshVTK ();

}

void
vtkInteractorStyleDTI::ToggleVOIVisibility()
{
  this->voiVisibilityToggle = !(this->voiVisibilityToggle);
  for(vector<VOI*>::iterator iter=this->voiList.begin();iter!=this->voiList.end();iter++)
    (*iter)->getActor()->SetVisibility(voiVisibilityToggle);
  if (this->voiVisibilityToggle == false) {
    this->HighlightProp(NULL);
  }
  else {
    if (activeVOI) {
      this->HighlightProp(activeVOI->getActor());
    }
  }
  this->frame->RefreshVTK ();
  
}

bool 
vtkInteractorStyleDTI::LoadBackgrounds()
{

  wxString defaultDir;
  if (strcmp (_data_directory_path, "")) {

#if wxUSE_UNICODE
    defaultDir = wxString(_data_directory_path, wxConvUTF8);
#else
    defaultDir = wxString(_data_directory_path);
#endif
  }
  else {
    defaultDir = _T("");
    // try to set a reasonable default? Store in a file?
    // it's annoying that it always goes to the pathways directory here 
    // (since this was the last file loaded).
  }
    
  wxFileDialog fd(this->frame,_T("Choose backgrounds image(s) to load:"),defaultDir,_T(""),_T("NIFTI (*.nii;*.nii.gz;*.hdr)|*.nii;*.nii.gz;*.hdr"),wxMULTIPLE,wxDefaultPosition);
  fd.SetDirectory (defaultDir);
  if(fd.ShowModal() != wxID_OK)
    return false;

  bool clearExistingBackgrounds = true;

  if (scalarVolumeList.size() > 0) {
    // we might be appending
    ButtonChoiceDialog dlg(this->frame, _T("Choose an option"), _T("You have already loaded background images. What would you like to do?"), _T("Append to existing images"), _T("Replace existing images"));
    int response = dlg.ShowModal();
    if (response == ButtonChoiceDialog::BUTTON_CHOICE_1) {
      clearExistingBackgrounds = false;
    }
  }
  

  if (clearExistingBackgrounds) {
    scalarVolumeList.clear();
    this->current_background_id = 0;
    
    delete _background_paths;
    delete _background_filenames;
    _background_paths = new wxArrayString();
    _background_filenames = new wxArrayString();
    fd.GetPaths(*_background_paths);
    fd.GetFilenames(*_background_filenames);
    for (int k = 0; k < _background_paths->GetCount(); k++) {
      scalarVolumeList.push_back (NULL);
    }
  }
  else {
    wxArrayString newPaths;
    fd.GetPaths (newPaths);
    wxArrayString newFilenames;
    fd.GetFilenames (newFilenames);
    for (int i = 0; i < newPaths.GetCount(); i++) {
      _background_paths->Add (newPaths[i]);
      _background_filenames->Add(newFilenames[i]);
      scalarVolumeList.push_back (NULL);
    }
  }

  //  cerr << "num backgrounds now is: " << _background_paths->GetCount();

  //  cerr << "data directory path was: " << _data_directory_path << endl;

#ifndef __WXMSW__
  wxString str = (*_background_paths)[0].BeforeLast('/');
#else
  wxString str = (*_background_paths)[0].BeforeLast('\\');
#endif

  strcpy (_data_directory_path, str.mb_str());

//    cerr << "data directory path is: " << _data_directory_path << endl;

  for (int i = 0; i < _background_paths->GetCount(); i++) {
    wxString foo = (*_background_paths)[i];
    //    cerr << i << ": " << (*_background_paths)[i] << endl;
    const char *cstr = foo.mb_str();
    if (strstr (cstr, "t1.nii.gz")) {
      wxString str = (*_background_paths)[i];
      _background_paths->Remove (str);
      _background_paths->Insert (str, 0);
      break;
    }
  }

  if (clearExistingBackgrounds) {
    // set the current background
    wxString filename = (*_background_paths)[0];
    LoadBackgroundInternal (filename.mb_str(), NULL);
  }

  DisplayTomoBorder (true);

  this->frame->RefreshVTK ();
  return true;
}

/*
bool
vtkInteractorStyleDTI::SetSubjectDatapath () 
{

 wxDirDialog dd(this->frame,_T("Choose a directory containing subject data:"), _T(""), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

  if(dd.ShowModal() != wxID_OK)
    return false;

  SetSubjectDatapathInternal (dd.GetPath().mb_str());
  return true;
}*/


void
vtkInteractorStyleDTI::SetSubjectDatapathInternal (const char *pathName)

{
  strcpy (_data_directory_path, pathName);
  // copy file names into array:

  delete _background_paths;
  _background_paths = new wxArrayString();
  delete _background_filenames;
  _background_filenames = new wxArrayString();

#if wxUSE_UNICODE
  wxString bpathDir (_data_directory_path, wxConvUTF8);
#else
  wxString bpathDir (_data_directory_path);
#endif
  wxDir::GetAllFiles (bpathDir, _background_paths, _T("*.nii.gz"));
  wxDir::GetAllFiles (bpathDir, _background_paths, _T("*.nii"));
  wxDir::GetAllFiles (bpathDir, _background_paths, _T("*.hdr"));

  for (int kk = 0; kk < _background_paths->GetCount(); kk++) {
#ifndef __WXMSW__
    wxString str = (*_background_paths)[kk].AfterLast('/');
#else
    wxString str = (*_background_paths)[kk].AfterLast('\\');
#endif
    _background_filenames->Add(str);
    this->scalarVolumeList.push_back (NULL);
  }

  if (_background_paths->GetCount() == 0) {
    cerr << "You must have at least one background image in the directory." << endl;
    exit(0);
  }

  for (int i = 0; i < _background_paths->GetCount(); i++) {
    wxString foo = (*_background_paths)[i];
    const char *cstr = foo.mb_str();
    if (strstr (cstr, "t1.nii.gz")) {
      wxString str = (*_background_paths)[i];
      _background_paths->Remove (str);
      _background_paths->Insert (str, 0);
      break;
    }
  }
  wxString filename = (*_background_paths)[0];
  this->current_background_id = 0;
  LoadBackgroundInternal (filename.mb_str(), NULL);

  DisplayTomoBorder (true);
  this->frame->RefreshVTK ();
}

void
vtkInteractorStyleDTI::SelectPlane (DTISceneActorID actorID)
{
   // Move the axial plane around
  this->ActorSelected = actorID;
  if (!getSceneProp (actorID)->GetVisibility()) {
    this->ToggleVisibility(actorID);
    this->frame->SetVisibilityCheck(actorID, true);
  }
  DisplayTomoBorder(true);
  this->frame->RefreshVTK();
}

void
vtkInteractorStyleDTI::SelectNextVisiblePlane ()
{
  if (getSceneProp(DTI_ACTOR_SAGITTAL_TOMO)->GetVisibility()) {
    SelectPlane (DTI_ACTOR_SAGITTAL_TOMO);
  }
  else if (getSceneProp(DTI_ACTOR_CORONAL_TOMO)->GetVisibility()) {
    SelectPlane (DTI_ACTOR_CORONAL_TOMO);
  }
  else if (getSceneProp(DTI_ACTOR_AXIAL_TOMO)->GetVisibility()) {
    SelectPlane (DTI_ACTOR_AXIAL_TOMO);
  }
}
