/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_CONTROLLER_H
#define DTI_CONTROLLER_H

class vtkInteractorStyleDTI;
class wxVTKRenderWindowInteractor;
class VOIPanel;
class FilterPanel;
class MyFrame;

//#include "FilterListener.h"
#include "VOIListener.h"
#include "FilterListener.h"
#include "InteractorListener.h"
#include "DTI_typedefs.h"

class DTIController : public VOIListener, public InteractorListener, public FilterListener {

 public:
  DTIController(vtkInteractorStyleDTI *renderer, VOIPanel *roiPanel, FilterPanel *filterPanel, wxVTKRenderWindowInteractor *window, MyFrame *frame);
  virtual ~DTIController();

  virtual void roiPanelTranslate (double x, double y, double z);
  virtual void roiPanelScale (double x, double y, double z);
  virtual void roiPanelLabel (const char *value);
  virtual void roiPanelSetVOI (int roiID);
  virtual void roiPanelMotion (VOIMotionType type, int id);
  virtual void roiPanelEnterPressed();

  virtual void interactorFilterStringUpdate (DTIQueryOperation op, const char *newstr);
  virtual void interactorWindowUnitsChanged (int units);
  void interactorWindowVOIUpdate (double x, double y, double z,
								  double xsize, double ysize, double zsize, int activeID, const char *label, int numVOIs, int *roiIDs, VOIMotionType motionType, int motionID);

  void interactorWindowSliderUpdate (double minLength, double maxLength, 
				     double minFA, double maxFA,
				     double minCurv, double maxCurv);

  void interactorWindowDatabaseLoaded (DTIPathwayDatabase *database);
  void interactorWindowAlgoUpdate (DTIPathwayAlgorithm algo);
  
  virtual void filterPanelLength (double minLength, double maxLength);
  virtual void filterPanelMedianFA (double minFA, double maxFA);
  virtual void filterPanelAlgorithm (DTIPathwayAlgorithm algo);
  virtual bool filterPanelFilterText (const char *filter);
  virtual void filterPanelFilterOp (DTIQueryOperation op);
  virtual void filterPanelMeanCurvature (double minCurv, double maxCurv);
  virtual void filterPanelEnterPressed();

 private:
  wxVTKRenderWindowInteractor *_window;
  vtkInteractorStyleDTI *_renderer;
  VOIPanel *_roi_panel;
  FilterPanel *_filter_panel; 
  MyFrame *_frame;
};

#endif
