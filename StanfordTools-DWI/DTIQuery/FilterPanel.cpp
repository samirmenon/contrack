/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: FilterPanel.cpp
 *    DATE: Wed Mar 10 17:24:28 PST 2004
 *************************************************************************/
#include "FilterPanel.h"
#include "SliderPanel.h"
#include "FilterListener.h"

#include <iostream>
#include "scene.h"
#include <util/typedefs.h>
#include "query_typedefs.h"

static const int ALGORITHM_NUM_CHOICES = 3;
static const wxString ALGORITHM_CHOICE_STRINGS[] = {
        _T("1 - STT"),
        _T("2 - TEND"),
        _T("3 - Any")
};

#define ALGORITHM_CHOICE_ID 662
#define FILTER_MODE_CHOICE_BOX_ID 663
#define FILTER_TEXT_CTRL_ID 664

static const int FILTER_MODE_NUM_CHOICES = 3;
static const wxString FILTER_MODE_STRINGS[] = {
	_T("1 - AND"),
	_T("2 - OR"),
	_T("3 - Custom")
};

BEGIN_EVENT_TABLE(FilterPanel, wxPanel)
  EVT_CHOICE ( ALGORITHM_CHOICE_ID, FilterPanel::OnAlgorithmChoice)
  EVT_CHOICE ( FILTER_MODE_CHOICE_BOX_ID, FilterPanel::OnFilterModeChoice)
  EVT_TEXT (FILTER_TEXT_CTRL_ID, FilterPanel::OnFilterText)
  EVT_TEXT_ENTER (FILTER_TEXT_CTRL_ID, FilterPanel::OnEnter)
END_EVENT_TABLE()

/*************************************************************************
 * Function Name: FilterPanel::FilterPanel
 * Parameters: wxWindow *parent
 * Effects: 
 *************************************************************************/

FilterPanel::FilterPanel(wxWindow *parent) : wxPanel (parent)
{
  wxStaticBox *filterBox = new wxStaticBox (this, -1, _T("Queries"),
					 wxDefaultPosition, wxDefaultSize, wxTHICK_FRAME);

  wxBoxSizer *vSizer = new wxStaticBoxSizer (filterBox, wxVERTICAL);

  wxStaticBox *lengthBox = new wxStaticBox (this, -1, _T("Length of Pathway (mm)"),
					    wxDefaultPosition, wxDefaultSize);
  //wxStaticBox *medianBox = new wxStaticBox (this, -1, _T("Average FA Along Pathway"),
  wxStaticBox *medianBox = new wxStaticBox (this, -1, _T("Log Score"),
					    wxDefaultPosition, wxDefaultSize);

  //  wxStaticBox *angleBox = new wxStaticBox (this, -1, _T("Average Curvature Along Pathway (1/mm)"),
  wxStaticBox *angleBox = new wxStaticBox (this, -1, _T("Average Log Score"),
					    wxDefaultPosition, wxDefaultSize);

  wxStaticBoxSizer *lengthSizer = new wxStaticBoxSizer (lengthBox, wxVERTICAL);
  wxStaticBoxSizer *medianSizer = new wxStaticBoxSizer (medianBox, wxVERTICAL);
  wxStaticBoxSizer *angleSizer = new wxStaticBoxSizer (angleBox, wxVERTICAL);


  wxStaticText *choiceText = new wxStaticText (this, -1, _T("Query Op: "), wxDefaultPosition, wxDefaultSize);
  _filter_mode_choice = new wxChoice (this, FILTER_MODE_CHOICE_BOX_ID,
		wxDefaultPosition, wxDefaultSize, FILTER_MODE_NUM_CHOICES, FILTER_MODE_STRINGS);

  _filter_mode_choice->SetSelection (DTI_FILTER_AND);
  wxStaticText *filterText = new wxStaticText (this, -1, _T("Query: "), wxDefaultPosition, wxDefaultSize);
  _filter_text = new wxTextCtrl (this, FILTER_TEXT_CTRL_ID, _T(""), wxDefaultPosition, wxSize(60,23), wxTE_PROCESS_ENTER);

  _filter_text->Enable(false);

  _min_length_slider = new SliderPanel (this, _T("min: "), 1);
  _max_length_slider = new SliderPanel (this, _T("max: "), 1);
  _min_median_fa_slider = new SliderPanel (this, _T("min: "), 1);
  _max_median_fa_slider = new SliderPanel (this, _T("max: "), 1);
  _min_max_angle_slider = new SliderPanel (this, _T("min: "), 1);
  _max_max_angle_slider = new SliderPanel (this, _T("max: "), 1);

  //_min_length_slider->setDecimalPlaces(1);
  //_max_length_slider->setDecimalPlaces(1);
  
  _min_length_slider->addListener(this);
  _max_length_slider->addListener(this);
  _min_median_fa_slider->addListener(this);
  _max_median_fa_slider->addListener(this);
  _min_max_angle_slider->addListener(this);
  _max_max_angle_slider->addListener(this);

  wxStaticText *algorithmText = new wxStaticText (this, -1, _T("Algorithm: "), wxDefaultPosition, wxDefaultSize);

  _algorithm_choice = new wxChoice (this, ALGORITHM_CHOICE_ID, 
								    wxDefaultPosition, wxDefaultSize, ALGORITHM_NUM_CHOICES, ALGORITHM_CHOICE_STRINGS);

  _algorithm_choice->SetSelection (PATHWAY_ALGORITHM_STT);

  wxBoxSizer *roiSizer = new wxBoxSizer (wxHORIZONTAL);

  int smallVal = 5;
  int bigVal = 10;
  roiSizer->Add (algorithmText, 0, wxALIGN_CENTRE_VERTICAL);
  roiSizer->Add (smallVal, 0);
  roiSizer->Add (_algorithm_choice, 0);
  roiSizer->Add (bigVal,0);
  roiSizer->Add (choiceText, 0, wxALIGN_CENTRE_VERTICAL);
  roiSizer->Add (smallVal, 0);
  roiSizer->Add (_filter_mode_choice, 0);
  roiSizer->Add (bigVal,0);
  roiSizer->Add (filterText, 0, wxALIGN_CENTRE_VERTICAL);
  roiSizer->Add (smallVal,0);
  roiSizer->Add (_filter_text, 1);

  lengthSizer->Add (0,5);
  lengthSizer->Add (_min_length_slider, 1, wxGROW);
  lengthSizer->Add (_max_length_slider, 1, wxGROW);
  medianSizer->Add (0,5);
  medianSizer->Add (_min_median_fa_slider, 1, wxGROW);
  medianSizer->Add (_max_median_fa_slider, 1, wxGROW);
  medianSizer->Add (0,5);
  angleSizer->Add (_min_max_angle_slider, 1, wxGROW);
  angleSizer->Add (_max_max_angle_slider, 1, wxGROW);

  vSizer->Add (0,5);
  vSizer->Add (roiSizer, 0, wxGROW);
  vSizer->Add (0,10);
  vSizer->Add (lengthSizer, 1, wxGROW);
  vSizer->Add (0,10);
  vSizer->Add (medianSizer, 1, wxGROW);
  vSizer->Add (0,10);
  vSizer->Add (angleSizer, 1, wxGROW);
  //minSizer->Show(FALSE);

  enableWidgets(false);
  SetSizer(vSizer);
  vSizer->SetSizeHints(this);
  Layout();
  
}


/*************************************************************************
 * Function Name: FilterPanel::~FilterPanel
 * Parameters: 
 * Effects: 
 *************************************************************************/

FilterPanel::~FilterPanel()
{
}


/*************************************************************************
 * Function Name: FilterPanel::setValueRanges
 * Parameters: double minLength, double maxLength, double minMedianFA, double maxMedianFA, double minMinFA, double minMinFA
 * Returns: void
 * Effects: 
 *************************************************************************/
void
FilterPanel::setValueRanges(double minLength, double maxLength, double minMedianFA, double maxMedianFA, double minMinFA, double maxMinFA, double minMaxAngle, double maxMaxAngle)
{
  _min_length_slider->setRange (minLength, maxLength);
  _max_length_slider->setRange (minLength, maxLength);
  
  _min_median_fa_slider->setRange (minMedianFA, maxMedianFA);
  _max_median_fa_slider->setRange (minMedianFA, maxMedianFA);

  _min_max_angle_slider->setRange (minMaxAngle, maxMaxAngle);
  _max_max_angle_slider->setRange (minMaxAngle, maxMaxAngle);

  //_min_min_fa_slider->setRange (minMinFA, maxMinFA);
  //_max_min_fa_slider->setRange (minMinFA, maxMinFA);
}


/*************************************************************************
 * Function Name: FilterPanel::addListener
 * Parameters: FilterListener *listener
 * Returns: void
 * Effects: 
 *************************************************************************/
void
FilterPanel::addListener(FilterListener *listener)
{

  _listeners.push_back (listener);
  
}


/*************************************************************************
 * Function Name: FilterPanel::sliderChanged
 * Parameters: SliderPanel *panel
 * Returns: void
 * Effects: 
 *************************************************************************/
void
FilterPanel::sliderChanged(SliderPanel *panel)
{
  for (std::vector<FilterListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    FilterListener *listener = *iter;
    if (panel == _min_length_slider ||
	panel == _max_length_slider) {
      listener->filterPanelLength (_min_length_slider->getValue(),
				   _max_length_slider->getValue());
    }
    else if (panel == _min_median_fa_slider ||
	     panel == _max_median_fa_slider) {
      listener->filterPanelMedianFA (_min_median_fa_slider->getValue(),
				     _max_median_fa_slider->getValue());
    }
	else if (panel == _min_max_angle_slider ||
		panel == _max_max_angle_slider) {
			listener->filterPanelMeanCurvature (_min_max_angle_slider->getValue(),
										        _max_max_angle_slider->getValue());
		}
//    else if (panel == _min_min_fa_slider ||
//	     panel == _max_min_fa_slider) {
//      listener->filterPanelMinFA (_min_min_fa_slider->getValue(),
//				  _max_min_fa_slider->getValue());
//    }
  }
}

void
FilterPanel::enableWidgets(bool pred) {

  _max_length_slider->setEnabled(pred);
  _min_length_slider->setEnabled(pred);
  _max_median_fa_slider->setEnabled(pred);
  _min_median_fa_slider->setEnabled(pred);
  _max_max_angle_slider->setEnabled(pred);
  _min_max_angle_slider->setEnabled(pred);

  _algorithm_choice->Enable(pred);
  _filter_mode_choice->Enable(pred);
}

void
FilterPanel::initialize() {

  enableWidgets(true);
  _max_length_slider->setValue (_max_length_slider->getMax());
  _min_length_slider->setValue (_max_length_slider->getMin());
  _max_median_fa_slider->setValue (_max_median_fa_slider->getMax());
  _min_median_fa_slider->setValue (_max_median_fa_slider->getMin());
  _max_max_angle_slider->setValue (_max_max_angle_slider->getMax());
  _min_max_angle_slider->setValue (_max_max_angle_slider->getMin());
  //	_max_min_fa_slider->setValue (_max_min_fa_slider->getMax());
  //	_min_min_fa_slider->setValue (_max_min_fa_slider->getMin());
}

void
FilterPanel::setLengths(double minLength, double maxLength)
{

	_min_length_slider->setValue (minLength);
	_max_length_slider->setValue (maxLength);

}

void
FilterPanel::setFAValues(double minFA, double maxFA)
{
  _min_median_fa_slider->setValue(minFA);
  _max_median_fa_slider->setValue(maxFA);
}

void
FilterPanel::setCurvatureValues(double minCurv, double maxCurv)
{
  _min_max_angle_slider->setValue(minCurv);
  _max_max_angle_slider->setValue(maxCurv);
}

void
FilterPanel::setAlgorithm(DTIPathwayAlgorithm algo)
{
  if (algo !=  DTI_PATHWAY_ALGORITHM_ANY) {
    _algorithm_choice->SetSelection(algo);
  }
  else {
    _algorithm_choice->SetSelection(2);
  }
}



void
FilterPanel::OnAlgorithmChoice(wxCommandEvent &event) 
{
  DTIPathwayAlgorithm algo;
  int selection = _algorithm_choice->GetSelection();
  //#ifdef __WXX11__
  // for some reason on X11, the "Any" choice appears first in the list.
  // (?!?!?)
  //  selection--;
  //  if (selection < 0) { selection = 2; }
  //#endif
  if (selection >= 2) {
    // xxx should generalize this to an arbitrary # of algos
    algo = DTI_PATHWAY_ALGORITHM_ANY;
  }
  else {
    algo = (DTIPathwayAlgorithm) selection;
  }
	for (std::vector<FilterListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
		FilterListener *listener = *iter;
		listener->filterPanelAlgorithm (algo);
	}
}





/*************************************************************************
 * Function Name: FilterPanel::OnFilterText
 * Parameters: wxCommandEvent& event
 * Returns: void
 * Effects: 
 *************************************************************************/
void
FilterPanel::OnFilterText(wxCommandEvent& event)
{
  /*
  for (std::vector<FilterListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    FilterListener *listener = *iter;
    listener->filterPanelFilterText(_filter_text->GetValue().mb_str());
    }*/
  if (_filter_text->IsModified()) {
    _filter_text->SetBackgroundColour (*wxLIGHT_GREY);
    _filter_text->Refresh(FALSE);
  }
}


/*************************************************************************
 * Function Name: FilterPanel::OnFilterModeChoice
 * Parameters: wxCommandEvent& event
 * Returns: void
 * Effects: 
 *************************************************************************/
void
FilterPanel::OnFilterModeChoice(wxCommandEvent& event)
{
  DTIQueryOperation op = (DTIQueryOperation) _filter_mode_choice->GetSelection();
  if (op == DTI_FILTER_AND || op == DTI_FILTER_OR) {
	  _filter_text->Enable(false);
  }
  else {
	  _filter_text->Enable(true);
  }
  for (std::vector<FilterListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    FilterListener *listener = *iter;
    listener->filterPanelFilterOp(op);
  }
 
}


void
FilterPanel::setFilterString (DTIQueryOperation op, const char *str) 
{
#if wxUSE_UNICODE
  wxString strval(str, wxConvUTF8);
  _filter_text->SetValue (strval);
#else
  _filter_text->SetValue (str);
#endif
  if (op == DTI_FILTER_AND || op == DTI_FILTER_OR) {
    _filter_text->Enable(false);
  }
  else {
    _filter_text->Enable(true);
  }
  _filter_mode_choice->SetSelection (op);
}

void
FilterPanel::setUnits (int unitType)
{
	_units = unitType;
}

void
FilterPanel::OnEnter (wxCommandEvent &event)
{

   for (std::vector<FilterListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    FilterListener *listener = *iter;
    if (listener->filterPanelFilterText(_filter_text->GetValue().mb_str())) {
      _filter_text->SetBackgroundColour(*wxWHITE);
      _filter_text->Refresh(FALSE);
    }
    //    listener->filterPanelEnterPressed();
  }
}
