#include "qROIEditor.h"
#include "vtkRenderer.h"
#include "vtkMatrix4x4.h"
#include "vtkCamera.h"
#include "vtkMath.h"
#include "QuenchController.h"
#include "qVolumeViz.h"
#include "vtkROI.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"

qROIEditor::qROIEditor(vtkRenderer *renderer, qVolumeViz *volViz)
{
  _volviz = volViz;
  _renderer = renderer;
  _pen_down = true;
  _editing_voi = NULL;
  _brush_radius = 1;
  _status_text_actor = vtkTextActor::New();
  
  // set the position in the _status_text_actor text actor
  _status_text_actor->GetTextProperty()->SetFontSize(18);
  _status_text_actor->GetTextProperty()->SetFontFamilyToArial();
  _status_text_actor->GetTextProperty()->SetJustificationToLeft();
  _status_text_actor->GetPositionCoordinate()->SetValue(0,124);
  _status_text_actor->GetTextProperty()->SetColor(0.6,0,0);
  _status_text_actor->Modified();
  _renderer->AddActor(_status_text_actor);
  UpdateStatusDisplay();
}


qROIEditor::~qROIEditor()
{
  VTK_SAFE_DELETE(_status_text_actor);
}


DTIScalarVolume *
qROIEditor::GetROIImageMask()
{
  return _editing_voi->GetImageRepresentation();
}


void qROIEditor::OnLeftDown(int x, int y)
{
   // Pen is down when drawing with left mouse button:
  _pen_down = true;

  _old_x = x; _old_y = y;

  DrawBrush(x,y,_brush_radius, _pen_down); // returns true if the mouse was positioned over the active tomogram.
  
  // notify controller of change. (re-runs marching cubes, adds/updates ROI)
  NotifyAllListeners (PEvent (new Event(ROI_EDITED)));

}


// Called whenever the mouse is moved, and we're in ROI editing mode:
void qROIEditor::OnMouseMove(int x, int y)
{
  DrawLine(x,y,_old_x,_old_y,_pen_down);
  
  // notify controller of change. (re-runs marching cubes, adds/updates ROI)
  NotifyAllListeners (PEvent (new Event(ROI_EDITED)));
  _old_x = x; _old_y = y;
}


// Bresenham line function
void qROIEditor::DrawLine(int startx, int starty, int endx, int endy, bool penDown) {

  // Force line to go from left to right
  if (startx > endx) {
    int i = startx, j = starty;
    startx = endx; starty = endy;
    endx = i; endy = j;
  }

  // Compute dx and dy and uy (the direction of y)
  int dx = endx - startx, dy = endy - starty;
  int uy = 1;
  if (dy < 0) {  // If line slants downward, go opposite direction in y
    dy = -dy;
    uy = -1;
  }

  // If x is longer than y, iterate along x
  if (dx >= dy) {
    // Compute 2dy and 2dy - 2dx, and decision starting value 2dy - dx
    int neg = 2 * dy, pos = neg - 2 * dx;
    int p = neg - dx;

    // And do the loop
    int i, x = startx, y = starty;
    DrawBrush(x, y, _brush_radius, penDown);
    for (i = 0; i < dx; i++) {
      if (p < 0) {
        DrawBrush(++x, y, _brush_radius, penDown);
        p += neg;
      }
      else {
        y += uy;
        DrawBrush(++x, y, _brush_radius, penDown);
        p += pos;
      }
    }
  }
  // Otherwise iterate along y
  else {
    // Compute 2dx and 2dx - 2dy, and decision starting value 2dx - dy
    int neg = 2 * dx, pos = neg - 2 * dy;
    int p = neg - dy;
  
    // And do the loop
    int i, x = startx, y = starty;
    DrawBrush(x, y, _brush_radius, penDown);
    for (i = 0; i < dy; i++) {
      if (p < 0) {
        y += uy;
        DrawBrush(x, y, _brush_radius, penDown);
        p += neg;
      }
      else {
        y += uy;
        DrawBrush(++x, y, _brush_radius, penDown);
        p += pos;
      }
    }
  }
}

void qROIEditor::OnRightDown(int x, int y)
{
  _pen_down = false;
  _old_x = x;
  _old_y = y;
  DrawBrush(x,y,_brush_radius,_pen_down);
 
  // notify controller of change. (re-runs marching cubes, adds/updates ROI)

  NotifyAllListeners (PEvent (new Event(ROI_EDITED)));

}

// Draw or erase a splotch of ROI at the specified position within the
// specified tomogram.
void qROIEditor::Brush3D(DTISceneActorID plane, int x, int y, int z, int brushRadius, bool penDown)
{
  int horizCenter;
  int vertCenter;
  int slice;

  switch (plane)
    {
    case DTI_ACTOR_SAGITTAL_TOMO:
      slice = x;
      horizCenter = y;
      vertCenter = z;
      break;
    case DTI_ACTOR_CORONAL_TOMO:
      slice = y;
      horizCenter = x;
      vertCenter = z;
      break;
    case DTI_ACTOR_AXIAL_TOMO:
      slice = z;
      horizCenter = x;
      vertCenter = y;
      break;
    };

  int top = vertCenter - brushRadius - 1;
  int bottom = vertCenter  + brushRadius + 1;
  
  if (top < 0) top = 0;

  int left = horizCenter - brushRadius - 1;
  int right = horizCenter + brushRadius + 1;
  
  if (left < 0) left = 0;

  for(int j=0; j<=2*brushRadius; j++) {
    for(int i=0; i<=2*brushRadius; i++) {
       int xpos = left+j;
       int ypos = top+i; 
       int dx = xpos - horizCenter;
       int dy = ypos - vertCenter; 
       double distance = sqrt((double)(dx*dx+dy*dy));
       if (distance > brushRadius) continue;

       DrawPoint(xpos,ypos,slice,plane,penDown);
    }
  }
}

// Update a single voxel within the ROI at the specified position within the 
// specified slice.

void qROIEditor::DrawPoint (int horizCenter, int vertCenter, int slice, DTISceneActorID plane, bool penDown)
{

  unsigned int x,y,z;
  switch (plane)
    {
    case DTI_ACTOR_SAGITTAL_TOMO:
      x = slice;
      y = horizCenter;
      z = vertCenter;
      break;
    case DTI_ACTOR_CORONAL_TOMO:
      y = slice;
      x = horizCenter;
      z = vertCenter;
      break;
    case DTI_ACTOR_AXIAL_TOMO:
      z = slice;
      x = horizCenter;
      y = vertCenter;
      break;
    };
  if (!GetROIImageMask()) {
    return;
  }
  unsigned int dim[3];
  GetROIImageMask()->getDimension(dim[0], dim[1], dim[2]);
  if (x >= dim[0]) {
    x = dim[0]-1;
  }
  if (y >= dim[1]) {
    y = dim[1]-1;
  }
  if (z >= dim[2]) {
    z = dim[2]-1;
  }

  GetROIImageMask()->setScalar(penDown ? 1.0 : 0.0, x, y, z);

}


bool qROIEditor::ClickInsideTomogram(int x, int y)
{
  DTIVoxel v = DTIVoxel(3);
  bool inside = SelectSlicePoint(x, y, v);
  return inside;
}

// draw or erase a splotch of ROI at the specified X,Y location.
void qROIEditor::DrawBrush(int screenX, int screenY, int brushRadius, bool penDown)
{
  DTISceneActorID slice = _volviz->ActiveImage();
  DTIVoxel v = DTIVoxel(3);

  // Find the location inside the tomogram:
  SelectSlicePoint(screenX, screenY, v);
  int xIndex, yIndex, zIndex;
  xIndex = (int) floor(v[0]+0.5);
  yIndex = (int) floor(v[1]+0.5);
  zIndex = (int) floor(v[2]+0.5);
  Brush3D(_volviz->ActiveImage(), xIndex, yIndex, zIndex, _brush_radius, penDown);
}

// Converts a screen click into a voxel position within the currently-selected
// tomogram:
bool qROIEditor::SelectSlicePoint(int selectionX, int selectionY, DTIVoxel &volumePosition)
{
  unsigned int dim[4];
  GetROIImageMask()->getDimension(dim[0], dim[1], dim[2], dim[3]);

  double *mx = GetROIImageMask()->getTransformMatrix();

  vtkMatrix4x4 *vtkMx = vtkMatrix4x4::New();
  vtkMx->DeepCopy(mx);

  vtkMatrix4x4 *invertedMx = vtkMatrix4x4::New();
  vtkMatrix4x4::Invert (vtkMx, invertedMx);

  double cameraP[4] = {0, 0, 0, 1};
  double cameraD[4] = {0,0,0,0};
  double display[4] = {0, 0, 0, 0};
  double* dPickPoint;

  vtkCamera* camera = _renderer->GetActiveCamera();

  // Store camera position in world coordinates
  camera->GetPosition(cameraP);

  // Determine "look at" position (based on mouse screen position)
  display[0]=selectionX;
  display[1]=selectionY;
  display[2]=0;
  _renderer->SetDisplayPoint(display);
  _renderer->DisplayToWorld();
  dPickPoint = _renderer->GetWorldPoint();

  // Create a direction vector pointing towards the "look at" from the camera position
  for(int i=0;i<3;i++)
    cameraD[i] = dPickPoint[i]-cameraP[i];

  // Normalize our direction vector
  vtkMath::Normalize(cameraD);

  // Get currently selected slice:
  Vector3d cursorPosition = _volviz->Position();

  double d, s, P, D;
  switch(_volviz->ActiveImage())
  {
  case DTI_ACTOR_SAGITTAL_TOMO:
    // Check for intersection with sagittal plane
    s = cursorPosition[DTI_ACTOR_SAGITTAL_TOMO];
    P = cameraP[0];
    D = cameraD[0];
    break;
    case DTI_ACTOR_CORONAL_TOMO:
    // Check for intersection with coronal plane
    s = cursorPosition[DTI_ACTOR_CORONAL_TOMO];
    P = cameraP[1];
    D = cameraD[1];
    break;
    case DTI_ACTOR_AXIAL_TOMO:
    // Check for intersection with axial plane
    s = cursorPosition[DTI_ACTOR_AXIAL_TOMO];
    P = cameraP[2];
    D = cameraD[2];
    break;
  }

  // Intersect our line with plane
  d = (s - P) / D;
 
  double pickObj[3];
  for (int i=0; i < 3; i++)
  {
    pickObj[i] = d*cameraD[i] + cameraP[i];
  }

  Vector3d vPos(pickObj);
  bool round = true;
  bool clamped = qVolumeViz::WorldToLocal(GetROIImageMask()->getTransformMatrix(), vPos, dim, volumePosition, round);
  return !clamped;

}

void
qROIEditor::GetBrushSizeMM(double &xSize, double &ySize, double &zSize)
{
  DTIScalarVolume *vol = GetROIImageMask();
  double voxSize[3];
  vol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);
  switch (_volviz->ActiveImage())
    {
    case DTI_ACTOR_SAGITTAL_TOMO:
      xSize = voxSize[0];
      ySize = voxSize[1]*_brush_radius;
      zSize = voxSize[2]*_brush_radius;
      break;
    case DTI_ACTOR_CORONAL_TOMO:
      xSize = voxSize[0]*_brush_radius;
      ySize = voxSize[1];
      zSize = voxSize[2]*_brush_radius;
      break;
    case DTI_ACTOR_AXIAL_TOMO:
      xSize = voxSize[0]*_brush_radius;
      ySize = voxSize[1]*_brush_radius;
      zSize = voxSize[2];
      break;
    };

}

// Update the text display to indicate whether ROI editing is on/off, 
// and show the brush size.
void
qROIEditor::UpdateStatusDisplay()
{
  char header[255] = "ROI EDITING ON.\nBrush size";

  char editingString[255];
  if (_editing_voi_mode) {
    float size;
    if (GetROIImageMask()->isIsotropic(size)) {
      sprintf (editingString, "%s: %0.1lf mm\nPress Ctrl-R to finish", header, size*_brush_radius);
    }
    else { 
      double brushSizeMM[3];
      GetBrushSizeMM(brushSizeMM[0], brushSizeMM[1], brushSizeMM[2]);
      sprintf (editingString, "%s (mm): %0.1lf, %0.1lf, %0.1lf\nPress Ctrl-R to finish", header, brushSizeMM[0], brushSizeMM[1], brushSizeMM[2]);
    }
  }
  else {
    sprintf (editingString, "Ctrl-R to edit ROI.\nCtrl-D to draw new ROI.");
  }
    
  _status_text_actor->SetInput(editingString);
  _status_text_actor->Modified();
}


// Increase brush size - always an integer multiple of the voxel size:
void 
qROIEditor::IncreaseBrushSize()
{
  _brush_radius = _brush_radius + 1;
  if (_brush_radius > MAX_BRUSH_RADIUS) {
    _brush_radius = MAX_BRUSH_RADIUS;
  }
  UpdateStatusDisplay();
}

// Decrease brush size - always an integer multiple of the voxel size:
void
qROIEditor::DecreaseBrushSize()
{
  _brush_radius = _brush_radius - 1;
  if (_brush_radius < 1) {
    _brush_radius = 1;
  }
  UpdateStatusDisplay();
}
