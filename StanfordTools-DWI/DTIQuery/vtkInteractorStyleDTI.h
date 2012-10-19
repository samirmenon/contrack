/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef __vtkInteractorStyleDTI_h
#define __vtkInteractorStyleDTI_h

#include "DTI_typedefs.h"
#include "vtkInteractorStyle.h"
class Scene;
class VOI;
class ViewCone;

#include <vector>
#include <util/VOIManager.h>

class vtkPolyData;
class vtkImageData;
class DTIQueryProcessor;

class DTIPathwayDatabase;
class DTIFilterPathwayScalar;
class DTIPathway;
class vtkLookupTable;
class MyFrame;

#include <list>
#include <map>
#include <typedefs.h>
#include <DTIVolume.h>

class CameraControls;
class DTIFilterAlgorithm;
class vtkCellPicker;
class vtkCellLocator;
class wxArrayString;
class vtkColorTransferFunction;

// vtkInteractorStyle reserves 0-9
#define VTKIS_MOVEVOI	10

//#define VTKIS_SAGITTAL  1
//#define VTKIS_CORONAL	2
//#define VTKIS_AXIAL		3
//#define VTKIS_VOI		4
//#define VTKIS_SURFACE	5
//#define VTKIS_TRACTS	6
//#define VTKIS_CUTPLANES	7

typedef std::vector<VOI*>::iterator voiIter;
class InteractorListener;

class color {
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
};


class vtkInteractorStyleDTI : public vtkInteractorStyle, public VOIManager 
{
public:
  friend class MyFrame;
  static vtkInteractorStyleDTI *New();
  vtkTypeRevisionMacro(vtkInteractorStyleDTI,vtkInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent);
  void SetFrame (MyFrame *frameParam) {this->frame = frameParam; }

   // Description:
  // The sub styles need the interactor too.

  // Description:
  // Event bindings controlling the effects of pressing mouse buttons
  // or moving the mouse.
  virtual void OnChar();
  virtual void OnMouseMove();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  virtual void OnMouseWheelForward();
  virtual void OnMouseWheelBackward();

  virtual DTIFilterVOI *getVOIFilter (const char *name);

  void NextLineWidth ();
  void SetScene(Scene* p);
  void SelectPlane(DTISceneActorID id);
  void SelectNextVisiblePlane();
  void ToggleVisibility(DTISceneActorID id);
  void ToggleVOIVisibility();
 
  void ToggleCorticalSurfaceVisibility();
  void SelectSlicePoint(int slice, int selectionX, int selectionY, double pick[3]);
  void SetVOITranslation (double x, double y, double z);
  void SetVOIScale (double x, double y, double z);
  void SetMinLength (double minLength);
  void SetMaxLength (double maxLength);
  void SetMinMedianFA (double minMedianFA);
  void SetMaxMedianFA (double maxMedianFA);
  void SetMinMeanCurvature (double curv);
  void SetMaxMeanCurvature (double curv);

  std::vector<int> *constructMeshPatch (vtkPolyData *inputData,
				   int cellID,
				   const double patchDim[3]);

  void SetAlgorithm (DTIPathwayAlgorithm algo);

  void SetVOIMotion (VOIMotionType type, int id);

  void SetVOILabel (const char *label);

  void AddListener (InteractorListener *listener);
  vtkActor *RebuildPatch(VOI *voi, vtkPolyData *patchData, bool ghostPatch);
  vtkActor *GetCurrentSurfaceActor();
  void MoveVOI();
  void DrawOnSlice();
  void InsertVOI(double center[3], double length[3], int id);
  void InsertVOI();
  void DeleteVOI();
  void InsertVOIFromFile();

  // These methods for the different interactions in different modes
  // are overridden in subclasses to perform the correct motion. Since
  // they are called by OnTimer, they do not have mouse coord parameters
  // (use interactor's GetEventPosition and GetLastEventPosition)
  virtual void Rotate();
  virtual void Spin();
  virtual void Pan();
  virtual void Dolly();

  void setFilterOp (DTIQueryOperation op);
  bool setFilterText (const char *str);

  void setVOISymmetry (int otherID);
  void StainPathways();
  void SetVOI (int activeID);
  void CycleVOI ();

  vtkActor *BuildActorFromMesh (vtkPolyData *mesh);
  vtkPolyData *StripTriangles (vtkPolyData *mesh);
  void SetSmoothMesh (vtkPolyData *mesh);
  void SetBumpyMesh (vtkPolyData *mesh);
  vtkActor *GetSmoothMeshActor () { return m_smoothMeshActor; }
  vtkActor *GetBumpyMeshActor () { return m_bumpyMeshActor; }
  
  void SaveState();
  void RestoreState();
  
  void LoadPathways();
  void SavePathwaysPDB();
  void SavePathwaysImage();

  void CycleBackground(bool dir);
  //  bool SetSubjectDatapath();
  void SetSubjectDatapathInternal(const char *path);
  bool LoadBackgrounds();
  // and some internal thing...

  //void SetSphereActor (vtkActor *sphere) {this->sphereActor = sphere; }

protected:
  vtkInteractorStyleDTI();
  ~vtkInteractorStyleDTI();

  DTIScalarVolume *GetCurrentBackground ();
  void MoveCurrentTomogram (int amount);
  //void LoadBackgroundInternal (ifstream &stream);
  bool LoadBackgroundInternal (const char* filename, DTIScalarVolume *oldBackground);
  void LoadPathwaysInternal (ifstream &stream);
  void SetGhostVisibility (bool flag);
  void SetOverlayVisibility (bool flag);
  void ToggleVisibility(vtkProp *actor);

  void PickPoint();
  int PickCell (vtkActor *actor, vtkCellLocator *locator, int mouseX, int mouseY, double pickedPoint[3]);
  void ToggleDemoMode();
  bool bDemoMode;
  void AnimRotate(int dx, int dy);
  bool bAnimRotate;
  bool bAnimAxial;
  bool bAnimSagittal;
  bool bAPosSlice;
  bool bSPosSlice;
  void AnimSSlice();
  void AnimASlice();
  
  vtkProp *getSceneProp (DTISceneActorID actor);

  MyFrame *frame;

  void rebuildFilters();
  VOI *findVOIByID(int id);
  void MoveSymmetric (VOI *voi);

  double MotionFactor;

  //  vtkActor *sphereActor;
  //  vtkCellPicker *picker;

  vtkCellLocator *m_smoothMeshLocator;
  vtkCellLocator *m_bumpyMeshLocator;
  
  bool binitializedTomograms;

  //  int cellID;
  int lineWidthSetting;
  Scene* scene;
  CameraControls* camera_controls;
  DTISceneActorID ActorSelected;
  std::vector<VOI*> voiList;
  std::vector<DTIScalarVolume*> scalarVolumeList;
  VOI* activeVOI;
  ViewCone* ViewCut;
  DTIQueryOperation _filter_op;
  DTIPathwayDatabase* whole_database;
  DTIPathwayDatabase* current_database;

  int current_background_id;

  void SetScenePathways( );
  void InitializeScenePathways();

  void InitializeTomograms();
  void SetTomograms(DTIScalarVolume *vol, DTIScalarVolume *oldVol);
  void ResetView();

  bool bInitializedScenePathways;
  bool bInitializedTomograms;
  std::vector<vtkActor*> pathway_list;
  //std::vector<int> pathway_startpt_list;
  //DTIPathwayDatabase* UpdateLengthFilter(DTIPathwayDatabase* input_database, double amount);
  vtkPolyData* pathwaysPolyData;
  vtkPolyDataMapper* pathwaysMapper;
  vtkActor* pathwaysActor;

  DTIQueryProcessor *_query_processor;

  vtkImageData* fa_imagedata;

  DTIFilterPathwayScalar* lengthFilter;
  DTIFilterPathwayScalar* medianFAFilter;
  DTIFilterPathwayScalar* meanCurvatureFilter;
  DTIFilterAlgorithm* algoFilter;
  double minPathwayLength;
  std::list<InteractorListener *> listeners;

  virtual void Dolly(double factor);

  void broadcastActiveVOI();
  void broadcastSliderValues();
  void broadcastFilterString(const char *str);
  void broadcastAlgoChoice();

  std::vector<color> pathway_colormap_class;
  std::vector<color> pathway_colormap_seq;
  std::map<DTIPathway*,int> pathwayVOImap;
  std::map<DTIPathway*,double> pathwayJittermap;
  short curPathwayRegion;
  short GetPathwayGroup(DTIPathway* t);
  double GetPathwayJitter(DTIPathway* t);
  void SetPathwayGroup(short g);
  void SetPathwayGroup2(short g);
  //std::vector<vector<DTIPathway*>> fiberVOImap;
  void InitializePathwayJitter();

  int voiCounter;

  int my_meshVisibility;
  // Active tomogram border
  vtkPolyData         *TomoBorderPolyData;
  vtkPolyDataMapper	  *TomoBorderMapper;
  vtkActor            *TomoBorderActor;
  void DisplayTomoBorder(bool on);
  double AvgFPS;
  unsigned int NumFPSMeasures;

  bool m_ghosting;
  vtkActor *m_smoothMeshActor;
  vtkActor *m_bumpyMeshActor;
  int m_meshVisibility;
  
private:

  void UpdateLUT (double window, double level);

  vtkLookupTable *_bw_lut;
  vtkColorTransferFunction *_color_lut;

  double _lut_level;
  double _lut_window;

  bool voiVisibilityToggle;
  bool voiLabelsToggle;
  vtkInteractorStyleDTI(const vtkInteractorStyleDTI&);  // Not implemented.
  void operator=(const vtkInteractorStyleDTI&);  // Not implemented.

  void ResetState();
  void SavePathwayState(std::ostream &os);
  void RestorePathwayState(std::istream &is);

  wxArrayString *_background_paths;
  wxArrayString *_background_filenames;
  char _data_directory_path[255];
};

#endif
