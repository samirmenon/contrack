/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: VOIManager.cpp
 *    DATE: Mon Mar  8  6:15:00 PST 2004
 *************************************************************************/
#include "VOIPanel.h"
#include "SliderPanel.h"
#include <iostream>
#include "VOIListener.h"
#include <util/typedefs.h>
#include "scene.h"
#include "DTI_typedefs.h"

#include <iostream>
using namespace std;

#define TEXT_CTRL_ID 662
#define SLIDER_ID     663

#define SYMMETRY_CHOICE_ID 668
#define VOI_CHOICE_ID 669

BEGIN_EVENT_TABLE(VOIPanel, wxPanel)
  EVT_TEXT (TEXT_CTRL_ID, VOIPanel::OnKeyDown)
  EVT_TEXT_ENTER (TEXT_CTRL_ID, VOIPanel::OnEnter)
  EVT_CHOICE ( SYMMETRY_CHOICE_ID, VOIPanel::OnSymmetryChoice)
  EVT_CHOICE ( VOI_CHOICE_ID, VOIPanel::OnVOIChoice)
END_EVENT_TABLE()

/*************************************************************************
 * Function Name: VOIPanel::VOIPanel
 * Parameters: wxWindow* parent
 * Effects: 
 *************************************************************************/

VOIPanel::VOIPanel(wxWindow* parent) : wxPanel (parent) 
{

  wxStaticBox *roiBox = new wxStaticBox (this, -1, _T("VOIs"),
					 wxDefaultPosition, wxDefaultSize);
  wxStaticBoxSizer *vSizer = new wxStaticBoxSizer (roiBox, wxVERTICAL);

  _units = W_UNIT; // anatomical by default.

  //  wxStaticText *idText = new wxStaticText (this, -1, _T("ID: "), wxDefaultPosition, wxDefaultSize);

  //  _id_text = new wxTextCtrl (this, TEXT_CTRL_ID, "", wxDefaultPosition, wxSize(30,23));
  //  _id_text->Enable(false);
  wxStaticText *labelText = new wxStaticText (this, -1, _T("Label: "), wxDefaultPosition, wxDefaultSize);
  _label_text = new wxTextCtrl (this, TEXT_CTRL_ID, _T(""), wxDefaultPosition, wxSize(60,23),wxTE_PROCESS_ENTER);
  /*
  _roi_next_button = new wxButton (this, VOI_NEXT_BUTTON_ID, _T(">"), wxDefaultPosition, wxSize(25,23));
  _roi_prev_button = new wxButton (this, VOI_PREV_BUTTON_ID, _T("<"), wxDefaultPosition, wxSize(25,23));
  _roi_state_text = new wxStaticText (this, -1, _T("VOI 0 of 0"), wxDefaultPosition, wxDefaultSize);
  */
  _roi_choice = new wxChoice (this, VOI_CHOICE_ID,
			      wxDefaultPosition, wxDefaultSize, 0, (const wxString *) __null);

  wxString str = _T("Free");
  wxStaticText *symmetryText = new wxStaticText (this, -1, _T("Constrain: "), wxDefaultPosition, wxDefaultSize);

  wxString *choices = new wxString[2];
  choices[0] = _T("Free");
  choices[1] = _T("Surface");

  _symmetry_choice = new wxChoice(this, SYMMETRY_CHOICE_ID, 
								  wxDefaultPosition, wxDefaultSize, 2, choices);
  _symmetry_choice->SetSelection (0);

  wxStaticBox *posBox = new wxStaticBox (this, -1, _T("Position (mm)"),
					 wxDefaultPosition, wxDefaultSize);
  wxStaticBox *sizeBox = new wxStaticBox (this, -1, _T("Size (mm)"),
					 wxDefaultPosition, wxDefaultSize);

  wxBoxSizer *posSizer = new wxStaticBoxSizer (posBox, wxVERTICAL);
  wxBoxSizer *sizeSizer = new wxStaticBoxSizer (sizeBox, wxVERTICAL);

  _x_size_slider = new SliderPanel (this, _T("sag: "), 1);
  _x_size_slider->addListener (this);
  _y_size_slider = new SliderPanel (this,_T("cor: "), 1);
  _y_size_slider->addListener (this);
  _z_size_slider = new SliderPanel (this,_T("ax: "), 1);
  _z_size_slider->addListener (this);
  _x_pos_slider = new SliderPanel (this, _T("sag: "), 1);
  _x_pos_slider->addListener (this);
  _y_pos_slider = new SliderPanel (this, _T("cor: "), 1);
  _y_pos_slider->addListener (this);
  _z_pos_slider = new SliderPanel (this, _T("ax: "), 1);
  _z_pos_slider->addListener (this);
  int smallVal = 5;
  int bigVal = 10;

  //wxGridSizer *settingSizer = new wxBoxSizer (wxVERTICAL);
  wxBoxSizer *gridSizer = new wxBoxSizer (wxVERTICAL);

  sizeSizer->Add (_x_size_slider, 1, wxGROW);
  sizeSizer->Add (_y_size_slider, 1, wxGROW);
  sizeSizer->Add (_z_size_slider, 1, wxGROW);
  posSizer->Add (_x_pos_slider, 1, wxGROW);
  posSizer->Add (_y_pos_slider, 1, wxGROW);
  posSizer->Add (_z_pos_slider, 1, wxGROW);
  gridSizer->Add (posSizer, 1, wxGROW);
  gridSizer->Add (0,smallVal);
  gridSizer->Add (sizeSizer, 1, wxGROW);

  this->_topVOISizer = new wxBoxSizer (wxHORIZONTAL);
  _topVOISizer->Add (_roi_choice, 0);
  _topVOISizer->Add (bigVal,0);
  /*
  _topVOISizer->Add (_roi_prev_button, 0);
  _topVOISizer->Add (smallVal,0);
  _topVOISizer->Add (_roi_state_text, 0, wxALIGN_CENTRE_VERTICAL);
  _topVOISizer->Add (smallVal,0);
    _topVOISizer->Add (_roi_next_button, 0);
  _topVOISizer->Add (bigVal,0);
  */
  //  _topVOISizer->Add (idText, 0, wxALIGN_CENTRE_VERTICAL);
  //  _topVOISizer->Add (smallVal,0);
  //  _topVOISizer->Add (_id_text, 0, wxALIGN_CENTRE_VERTICAL);
  //  _topVOISizer->Add (bigVal,0);
  _topVOISizer->Add (labelText, 0, wxALIGN_CENTRE_VERTICAL);
  _topVOISizer->Add (smallVal,0);
  _topVOISizer->Add (_label_text, 1);
  _topVOISizer->Add (bigVal,0);
  _topVOISizer->Add (symmetryText, 0, wxALIGN_CENTRE_VERTICAL);
  _topVOISizer->Add (smallVal,0);
  _topVOISizer->Add (_symmetry_choice, 0);
  vSizer->Add (0, smallVal);
  vSizer->Add (_topVOISizer, 0, wxGROW);
  vSizer->Add (0, bigVal);
  vSizer->Add (gridSizer, 1, wxGROW);
  SetSizer(vSizer);
  gridSizer->SetSizeHints(this);
  Layout();

  setVOIPanelActive(false);
}


/*************************************************************************
 * Function Name: VOIPanel::~VOIPanel
 * Parameters: 
 * Effects: 
 *************************************************************************/

VOIPanel::~VOIPanel()
{
}


/*************************************************************************
 * Function Name: VOIPanel::setLabel
 * Parameters: const char *label
 * Returns: void
 * Effects: 
 *************************************************************************/
void
VOIPanel::setLabel(const char *label)
{
#if wxUSE_UNICODE
  wxString str(label, wxConvUTF8);
  _label_text->SetValue (str);
#else
  _label_text->SetValue (label);
#endif
}


/*************************************************************************
 * Function Name: VOIPanel::setValues
 * Parameters: const double pos[3], const double size[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
VOIPanel::setValues(const double pos[3], const double size[3])
{
   if (_units == A_UNIT) {
	  _x_pos_slider->setValue (pos[0] - 2*Scene::ACPC_offset[0]);
	  _y_pos_slider->setValue (pos[1] - 2*Scene::ACPC_offset[1]);
	  _z_pos_slider->setValue (pos[2] - 2*Scene::ACPC_offset[2]);
  }
  else if (_units == W_UNIT) {
	 
	  _x_pos_slider->setValue (pos[0]);
	  _y_pos_slider->setValue (pos[1]);
	  _z_pos_slider->setValue (pos[2]);

  }
  _x_size_slider->setValue (size[0]);
  _y_size_slider->setValue (size[1]);
  _z_size_slider->setValue (size[2]);
  //_x_pos_slider->setValue (pos[0]);
  //_y_pos_slider->setValue (pos[1]);
  //_z_pos_slider->setValue (pos[2]);
}


/*************************************************************************
 * Function Name: VOIPanel::setValueRanges
 * Parameters: const double minCorner[3], const double maxCorner[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
VOIPanel::setValueRanges(const double minSize[3], const double maxSize[3],
			 const double minPos[3], const double maxPos[3])
{
  _min_pos[0] = minPos[0]; _min_pos[1] = minPos[1]; _min_pos[2] = minPos[2];
  _max_pos[0] = maxPos[0]; _max_pos[1] = maxPos[1]; _max_pos[2] = maxPos[2];
  _x_size_slider->setRange (minSize[0], maxSize[0]);
  _y_size_slider->setRange (minSize[1], maxSize[1]);
  _z_size_slider->setRange (minSize[2], maxSize[2]);
  
  _x_pos_slider->setRange (minPos[0], maxPos[0]);
  _y_pos_slider->setRange (minPos[1], maxPos[1]);
  _z_pos_slider->setRange (minPos[2], maxPos[2]);
			  
}


/*************************************************************************
 * Function Name: VOIPanel::sliderChanged
 * Parameters: SliderPanel *panel
 * Returns: void
 * Effects: 
 *************************************************************************/
void
VOIPanel::sliderChanged(SliderPanel *panel)
{
  for (std::vector<VOIListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    VOIListener *listener = *iter;
    if (panel == _x_size_slider ||
	panel == _y_size_slider ||
	panel == _z_size_slider) {
      listener->roiPanelScale (_x_size_slider->getValue(),
			_y_size_slider->getValue(),
			_z_size_slider->getValue());
    }
    else {
		if (_units == W_UNIT) {
		listener->roiPanelTranslate (_x_pos_slider->getValue(),
			    _y_pos_slider->getValue(),
			    _z_pos_slider->getValue());
		}
		else if (_units == A_UNIT) {
			listener->roiPanelTranslate (_x_pos_slider->getValue()+2*Scene::ACPC_offset[0],
			    _y_pos_slider->getValue()+2*Scene::ACPC_offset[1],
			    _z_pos_slider->getValue()+2*Scene::ACPC_offset[2]);
		}
    }
  }
}


/*************************************************************************
 * Function Name: VOIPanel::OnLabelText
 * Parameters: wxCommandEvent& event
 * Returns: void
 * Effects: 
 *************************************************************************/
void
VOIPanel::OnEnter(wxCommandEvent& event)
{
  for (std::vector<VOIListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    VOIListener *listener = *iter;
    listener->roiPanelEnterPressed();
  } 
}






void
VOIPanel::setID (const char *id)
{
  //	_id_text->SetValue(id);
}

void
VOIPanel::addListener (VOIListener *listener) 
{
	_listeners.push_back(listener);
}

void 
VOIPanel::OnVOIChoice(wxCommandEvent &event)
{
  int index = _roi_choice->GetSelection();

  // xxx hack - should be sending the ID instead of the index here.
  for (std::vector<VOIListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    VOIListener *listener = *iter;
    listener->roiPanelSetVOI(index);
    //    std::cerr << "setting VOI to " << index << std::endl;
  }
}

void
VOIPanel::setVOIPanelActive (bool active) {

  _roi_choice->Enable(active);
  _symmetry_choice->Enable(active);
  _label_text->Enable(active);

  _x_size_slider->setEnabled(active);
  _y_size_slider->setEnabled(active);
  _z_size_slider->setEnabled(active);

  _x_pos_slider->setEnabled(active);
  _y_pos_slider->setEnabled(active);
  _z_pos_slider->setEnabled(active);
  
}

void
VOIPanel::setVOIState(int activeID, int numVOIs, int *roiIDs, VOIMotionType motionType, int motionID) 
{
  if (numVOIs == 0) {
    setVOIPanelActive(false);
  }
  else {
    setVOIPanelActive(true);
  }
  _ids = roiIDs;
  _current_id = activeID;
  rebuildSymmetryChoice(activeID, numVOIs, roiIDs, motionType, motionID);
  rebuildVOIChoice(activeID, numVOIs, roiIDs);
}

void
VOIPanel::rebuildVOIChoice (int activeID, int numVOIs, int *roiIDs)
{
  wxString *choices = new wxString[numVOIs];
  
  int selectedIndex = 0;
  for (int i = 0; i < numVOIs; i++) {
#if wxUSE_UNICODE
    swprintf ((wchar_t *)choices[i].GetWriteBuf(10), 10, L"VOI %d", roiIDs[i]);
#else
    sprintf (choices[i].GetWriteBuf(10),"VOI %d", roiIDs[i]);
#endif
    if (activeID == roiIDs[i]) {
      selectedIndex = i; 
    }
    choices[i].UngetWriteBuf();
  }
  this->_topVOISizer->Remove(_roi_choice);
  delete _roi_choice;
  _roi_choice = new wxChoice (this, VOI_CHOICE_ID, 
				   wxDefaultPosition, wxDefaultSize, numVOIs, choices);
  this->_topVOISizer->Prepend (_roi_choice, 0);
  this->_topVOISizer->Layout();
  this->_roi_choice->SetSelection (selectedIndex);
  
}

void 
VOIPanel::rebuildSymmetryChoice(int activeID, int numVOIs, int *roiIDs,
				VOIMotionType motionType, int motionID)
{
  // build pull-down for symmetry:
  wxString *choices = new wxString[numVOIs+1];
  choices[0] = _T("Free");
  int count=1;
  int motionIndex = 0; // free
  for (int i = 0; i < numVOIs; i++) {
    if (roiIDs[i] != activeID) {
#if wxUSE_UNICODE
    swprintf ((wchar_t *)choices[count].GetWriteBuf(10), 10, L"%d", roiIDs[i]);
#else
    sprintf (choices[count].GetWriteBuf(10),"%d", roiIDs[i]);
#endif
      if (roiIDs[i] == motionID) {
	motionIndex = count;
      }
      choices[count].UngetWriteBuf();
      count++;
    }
  }
  this->_topVOISizer->Remove(_symmetry_choice);
  delete _symmetry_choice;
  _symmetry_choice = new wxChoice (this, SYMMETRY_CHOICE_ID, 
				   wxDefaultPosition, wxDefaultSize, 
				   (numVOIs > 0) ? numVOIs : 1, choices);
  this->_topVOISizer->Add (_symmetry_choice, 0);
  this->_topVOISizer->Layout();
  this->_symmetry_choice->SetSelection (motionIndex);
}


void
VOIPanel::OnSymmetryChoice(wxCommandEvent &event) 
{

  int id;
  VOIMotionType type;
  if (_symmetry_choice->GetSelection() == 0) {
    id = _current_id;
    type = VOI_MOTION_FREE;
  }
  else {
    type = VOI_MOTION_SYMMETRY;
    // xxx should be fixed soon.
    id = _symmetry_choice->GetSelection();
    if (id > _current_id-1) id++; 
  }
  for (std::vector<VOIListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    VOIListener *listener = *iter;
    listener->roiPanelMotion(type, id);
  }
  return; 
#if 0
	int id;
	VOIMotionType type;
	if (_symmetry_choice->GetSelection() == 0) {
	  id = _current_id;
	  type = VOI_MOTION_FREE;
	}
	// xxx hack
	else {
	  type = VOI_MOTION_SYMMETRY;
	  // xxx need to implement
	}
	for (std::vector<VOIListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
	    VOIListener *listener = *iter;
	    listener->roiPanelMotion(type, id);
	}

#endif
}

void
VOIPanel::setUnits(int unitType)
{
  _units = unitType;
  
  if (unitType == A_UNIT) {
	  _x_pos_slider->setRange (_min_pos[0]-Scene::ACPC_offset[0]*2, _max_pos[0]-Scene::ACPC_offset[0]*2);
	  _y_pos_slider->setRange (_min_pos[1]-Scene::ACPC_offset[1]*2, _max_pos[1]-Scene::ACPC_offset[1]*2);
	  _z_pos_slider->setRange (_min_pos[2]-Scene::ACPC_offset[2]*2, _max_pos[2]-Scene::ACPC_offset[2]*2);
	  _x_pos_slider->setValue (_x_pos_slider->getValue() - 2*Scene::ACPC_offset[0]);
	  _y_pos_slider->setValue (_y_pos_slider->getValue() - 2*Scene::ACPC_offset[1]);
	  _z_pos_slider->setValue (_z_pos_slider->getValue() - 2*Scene::ACPC_offset[2]);
  }
  else if (unitType == W_UNIT) {
	  _x_pos_slider->setRange (_min_pos[0], _max_pos[0]);
	  _y_pos_slider->setRange (_min_pos[1], _max_pos[1]);
	  _z_pos_slider->setRange (_min_pos[2], _max_pos[2]);
	  _x_pos_slider->setValue (_x_pos_slider->getValue() + 2*Scene::ACPC_offset[0]);
	  _y_pos_slider->setValue (_y_pos_slider->getValue() + 2*Scene::ACPC_offset[1]);
	  _z_pos_slider->setValue (_z_pos_slider->getValue() + 2*Scene::ACPC_offset[2]);

  }

	// update the value ranges.
	// and the current values.

}

void
VOIPanel::OnKeyDown (wxCommandEvent &event)
{
  for (std::vector<VOIListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    VOIListener *listener = *iter;
    listener->roiPanelLabel(_label_text->GetValue().mb_str());
  } 
  event.Skip();
}
