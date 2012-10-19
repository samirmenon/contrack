/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIController.cpp
 *    DATE: Tue Mar  9  3:04:06 PST 2004
 *************************************************************************/


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "SystemInfoDialog.h"
#include "query_typedefs.h"
#include "DTIController.h"
#include "vtkInteractorStyleDTI.h"
#include "wxVTKRenderWindowInteractor.h"
#include "VOIPanel.h"
#include "FilterPanel.h"
#include <util/DTIPathwayDatabase.h>
#include <util/DTIFilterTree.h>
#include "DTIFilterApp.h"
#include <algorithm>

#define MAX(a,b) a > b ? a : b

/*************************************************************************
 * Function Name: DTIController::DTIController
 * Parameters: vtkInteractorStyleDTI *renderer, VOIPanel *roiPanel, FilterPanel *filterPanel
 * Effects: 
 *************************************************************************/

DTIController::DTIController(vtkInteractorStyleDTI *renderer, VOIPanel *roiPanel, FilterPanel *filterPanel, wxVTKRenderWindowInteractor *window, MyFrame *frame)

{
  _frame = frame;
  _renderer = renderer;
  _roi_panel = roiPanel;
  _filter_panel = filterPanel;
  _window = window;
  roiPanel->addListener (this);
  renderer->AddListener (this);
  filterPanel->addListener (this);
}


/*************************************************************************
 * Function Name: DTIController::~DTIController
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIController::~DTIController()
{
}


/*************************************************************************
 * Function Name: DTIController::roiPanelTranslate
 * Parameters: double x, double y, double z
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIController::roiPanelTranslate(double x, double y, double z)
{
  _renderer->SetVOITranslation(x,y,z);
  //_window->Refresh(FALSE);
  _frame->RefreshVTK();
  _window->SetFocus();
}


/*************************************************************************
 * Function Name: DTIController::roiPanelScale
 * Parameters: double x, double y, double z
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIController::roiPanelScale(double x, double y, double z)
{
  // cerr << "unimplemented: roiPanelScale!" << endl;
  _renderer->SetVOIScale(x,y,z);
  _frame->RefreshVTK();
  _window->SetFocus();
}


/*************************************************************************
 * Function Name: DTIController::roiPanelLabel
 * Parameters: const char *value
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIController::roiPanelLabel(const char *value)
{
  _renderer->SetVOILabel(value);
  _frame->RefreshVTK();
  //_window->SetFocus();
}

//----------------------------------
void
DTIController::interactorWindowVOIUpdate(double xpos, double ypos, double zpos,
					 double xsize, double ysize, double zsize,
					 int activeID, const char *label, 
					 int numVOIs, int *roiIDs,
					 VOIMotionType motionType, int motionID) 
{
  _roi_panel->setLabel(label);
  double pos[3] = {xpos, ypos,zpos};
  double size[3] = {xsize,ysize,zsize};
  _roi_panel->setValues (pos, size);
  static char idStr[255];
  sprintf (idStr, "%d", activeID);
  _roi_panel->setID (idStr);
  _roi_panel->setVOIState (activeID, numVOIs, roiIDs, motionType, motionID);
}


void
DTIController::interactorWindowDatabaseLoaded (DTIPathwayDatabase *database)
{
	
  double xRange[2];
  double yRange[2];
  double zRange[2];
  database->getBoundingBox(xRange, yRange, zRange);
  double minSize[3] = {0.1, 0.1, 0.1};
  double maxSize[3] = {xRange[1]-xRange[0], yRange[1]-yRange[0], zRange[1]-zRange[0]};
  double minPt[3] = {xRange[0], yRange[0], zRange[0]};
  double maxPt[3] = {xRange[1], yRange[1], zRange[1]};
  _roi_panel->setValueRanges (minSize, maxSize, minPt, maxPt);

  double minL = database->getMinValue(PATHWAY_LENGTH);
  double maxL = database->getMaxValue(PATHWAY_LENGTH);
  double minMedianFA = database->getMinValue(PATHWAY_MEDIAN_FA);
  double maxMedianFA = database->getMaxValue(PATHWAY_MEDIAN_FA);
  double minMeanCurvature = database->getMinValue(PATHWAY_CURVATURE);
  double maxMeanCurvature = database->getMaxValue(PATHWAY_CURVATURE);
  
  _filter_panel->setValueRanges(minL, maxL, minMedianFA, maxMedianFA, 0, 0, minMeanCurvature, maxMeanCurvature);
  _filter_panel->initialize();
  _frame->EnableVOICommands(true);
}


//----------------------------------------------------------------------------

void 
DTIController::filterPanelLength (double minLength, double maxLength)
{
	_renderer->SetMinLength(minLength);
	_renderer->SetMaxLength(maxLength);
	_frame->RefreshVTK();
	_window->SetFocus();
}

//----------------------------------------------------------------------------
void 
DTIController::filterPanelMedianFA (double minFA, double maxFA)
{
	_renderer->SetMinMedianFA (minFA);
	_renderer->SetMaxMedianFA (maxFA);
	_frame->RefreshVTK();
	_window->SetFocus();
}


//----------------------------------------------------------------------------

void
DTIController::interactorWindowSliderUpdate (double minLength, double maxLength,
					     double minFA, double maxFA,
					     double minCurv, double maxCurv)
{
	_filter_panel->setLengths (minLength, maxLength);
	_filter_panel->setFAValues (minFA, maxFA);
	_filter_panel->setCurvatureValues (minCurv, maxCurv);
}

//----------------------------------------------------------------------------

void
DTIController::interactorWindowAlgoUpdate (DTIPathwayAlgorithm algo)
{
  _filter_panel->setAlgorithm(algo);
}


//----------------------------------------------------------------------------

bool
DTIController::filterPanelFilterText (const char *filter)
{
  bool success = _renderer->setFilterText (filter);
  if (success) {
    _frame->RefreshVTK();
    _window->SetFocus();
  }
  else {
    wxString msg;
    wxString caption(_T("Parsing Error"));
    wxString message(_T("Invalid function. Legal tokens include VOI numbers, 'and', 'or', 'not' and parentheses.\n\nExamples:\n\n   1 and (2 or 3)          // All pathways passing through VOI 1 and either VOI 2 or VOI 3.\n   (not 1) and (not 2)  // All pathways that do not pass through VOI 1 or VOI 2."));

    SystemInfoDialog* info_dialog = new SystemInfoDialog(_frame, caption, message);
    info_dialog->Show();
  }
  return success;
}

//----------------------------------------------------------------------------

void
DTIController::filterPanelFilterOp (DTIQueryOperation op)
{
  _renderer->setFilterOp(op);
  _frame->RefreshVTK();
}

//----------------------------------------------------------------------------

void
DTIController::interactorFilterStringUpdate (DTIQueryOperation op, const char *newstr) 
{
	_filter_panel->setFilterString (op, newstr);
}

void
DTIController::filterPanelAlgorithm (DTIPathwayAlgorithm algo)
{
	_renderer->SetAlgorithm(algo);
    _frame->RefreshVTK();
}


void
DTIController::roiPanelSetVOI (int activeID)
{
  _renderer->SetVOI(activeID);
  _frame->RefreshVTK();
}



void
DTIController::roiPanelMotion (VOIMotionType type, int id)
{
  _renderer->SetVOIMotion (type, id);
  _frame->RefreshVTK();
  // set symmetry here.
}

void 
DTIController::filterPanelMeanCurvature (double minCurv, double maxCurv) 
{
   _renderer->SetMinMeanCurvature (minCurv);
   _renderer->SetMaxMeanCurvature (maxCurv);
   _frame->RefreshVTK();
   _window->SetFocus();
}

void
DTIController::interactorWindowUnitsChanged (int units)
{
  _roi_panel->setUnits(units);
}

void
DTIController::roiPanelEnterPressed()
{
  _window->SetFocus();
}

void 
DTIController::filterPanelEnterPressed()
{
  _window->SetFocus();
}
