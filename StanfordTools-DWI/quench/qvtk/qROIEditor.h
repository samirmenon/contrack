#ifndef Q_ROI_EDITOR_H
#define Q_ROI_EDITOR_H

#include "DTIVolume.h"
#include "typedefs_quench.h"

class qVolumeViz;
class vtkRenderer;
class vtkROI;
class vtkTextActor;

class qROIEditor : public IEventSource {
 public:
  qROIEditor(vtkRenderer *renderer, qVolumeViz *volViz);
  ~qROIEditor();

  void Init(DTIScalarVolume *roiImageBase);
  void OnLeftDown(int x, int y);
  void OnMouseMove(int x, int y);
  void OnRightDown(int x, int y);
  DTIScalarVolume *GetROIImageMask();

  void DecreaseBrushSize();
  void IncreaseBrushSize();

  void SetROI (vtkROI *newROI) { _editing_voi = newROI;}
  vtkROI *GetROI() { return _editing_voi; }

  void GetBrushSizeMM(double &xSize, double &ySize, double &zSize);

  void UpdateStatusDisplay();

  bool GetEditingROIMode() { return _editing_voi_mode; }
  void SetEditingROIMode(bool mode) { _editing_voi_mode = mode; }

  bool ClickInsideTomogram(int x, int y);

 protected:
  void  DrawBrush(int x, int y, int brushRadius, bool penDown);
  void  Brush3D(DTISceneActorID plane, int x, int y, int z, int brushRadius, bool penDown);
  void  DrawLine(int startx, int starty, int endx, int endy, bool penDown);
  void DrawPoint(int horizCenter, int vertCenter, int slice, DTISceneActorID plane, bool penDown);
  bool SelectSlicePoint (int selectionX, int selectionY, DTIVoxel &volumePosition);

 private:

  bool _editing_voi_mode;
  vtkTextActor *_status_text_actor;
  vtkRenderer *_renderer;
  vtkROI *_editing_voi;
  qVolumeViz *_volviz;
  bool _pen_down;
  int _old_x, _old_y;
  int _brush_radius;

  static const int MAX_BRUSH_RADIUS = 10;
};

#endif
