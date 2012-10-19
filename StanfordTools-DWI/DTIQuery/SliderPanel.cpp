/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: SliderPanel.cpp
 *    DATE: Sun Mar  7 22:27:48 PST 2004
 *************************************************************************/
#include "SliderPanel.h"
#include "SliderListener.h"
#include <math.h>
#include <iostream>
using namespace std;

#define VALUE_CTRL_ID 662
#define SLIDER_ID     663

BEGIN_EVENT_TABLE(SliderPanel, wxPanel)
  EVT_TEXT_ENTER( VALUE_CTRL_ID, SliderPanel::OnValueText)
  EVT_COMMAND_SCROLL( SLIDER_ID, SliderPanel::OnSlider)
END_EVENT_TABLE()



/*************************************************************************
 * Function Name: SliderPanel::SliderPanel
 * Parameters: wxWindow* parent, wxString name, double min_limit, double max_limit
 * Effects: 
 *************************************************************************/

  SliderPanel::SliderPanel(wxWindow* parent, wxString name, int decimalPlaces) : wxPanel(parent)
{
  _decimal_places = decimalPlaces;
  _value = 0;
  _min = -100;
  _max = 100;
  _label_text = new wxStaticText (this, -1, name, wxDefaultPosition, wxSize(35,-1));
  _value_text_ctrl = new wxTextCtrl (this, VALUE_CTRL_ID, _T(""), wxDefaultPosition, wxSize(40,-1), wxTE_PROCESS_ENTER);
  update_value_text_widget ();
  _min_text = new wxStaticText (this, -1, _T(""), wxDefaultPosition, wxSize(50,23), wxALIGN_RIGHT);
  _max_text = new wxStaticText (this, -1, _T(""), wxDefaultPosition, wxSize(50,23), wxALIGN_RIGHT);
  update_range_text_widgets();
  _slider = new wxSlider (this, SLIDER_ID, 0, -INTERNAL_SLIDER_SCALE, +INTERNAL_SLIDER_SCALE, wxDefaultPosition, wxSize(185,-1));
  update_slider_widget();

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add (5,0);
  sizer->Add(_label_text, 0, wxALIGN_CENTRE_VERTICAL);
  sizer->Add (5,0);
  sizer->Add(_value_text_ctrl, 0, wxALIGN_CENTRE_VERTICAL);
  sizer->Add(20,0); // spacer
  sizer->Add(_min_text, 0, wxALIGN_CENTRE_VERTICAL);
  sizer->Add(_slider, 1, wxGROW | wxALIGN_CENTRE_VERTICAL); // only element that grows
  sizer->Add(_max_text, 0, wxALIGN_CENTRE_VERTICAL);
  sizer->Add(10,0); //spacer
  SetSizer(sizer);
  sizer->SetSizeHints(this);
  Layout();
  
}


/*************************************************************************
 * Function Name: SliderPanel::setValue
 * Parameters: double value
 * Returns: void
 * Effects: 
 *************************************************************************/
void
SliderPanel::setValue(double value)
{
  _value = value;
  update_value_text_widget();
  update_slider_widget();
}


/*************************************************************************
 * Function Name: SliderPanel::setRange
 * Parameters: double minValue, double maxValue
 * Returns: void
 * Effects: 
 *************************************************************************/
void
SliderPanel::setRange(double minValue, double maxValue)
{
  _min = minValue;
  _max = maxValue;
  update_range_text_widgets();
}


/*************************************************************************
 * Function Name: SliderPanel::addListener
 * Parameters: SliderListener *listener
 * Returns: void
 * Effects: 
 *************************************************************************/
void
SliderPanel::addListener(SliderListener *listener)
{
  _listeners.push_back(listener);
}


/*************************************************************************
 * Function Name: SliderPanel::update_value_text_widget
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
SliderPanel::update_value_text_widget()
{
  wxString temp;
  if (_decimal_places == 1) {
	temp.Printf(_T("%0.1f"),_value);
  }
  else if (_decimal_places == 2) {
	temp.Printf(_T("%0.2f"),_value);
  }

  _value_text_ctrl->SetValue(temp); 
}


/*************************************************************************
 * Function Name: SliderPanel::update_slider_widget
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
SliderPanel::update_slider_widget()
{
  double slider_ratio = (_value - _min) / (_max - _min);
  double slider_exact_position = _slider->GetMin() + 
    slider_ratio * (_slider->GetMax() - _slider->GetMin());
  int slider_rounded_position = (int) floor(slider_exact_position + 0.5);
  _slider->SetValue(slider_rounded_position);
}


/*************************************************************************
 * Function Name: SliderPanel::OnValueText
 * Parameters: wxCommandEvent& event
 * Returns: void
 * Effects: 
 *************************************************************************/
void
SliderPanel::OnValueText(wxCommandEvent& event)
{
  double user_input;
  bool legal_number = _value_text_ctrl->GetValue().ToDouble(&user_input);
  if (!legal_number) {
    // restore old value
    update_value_text_widget();
  } else {
    // An actual number was entered.  Trim it to max/min.
    if (user_input < _min) {
      user_input = _min;
    }
    if (user_input > _max) {
      user_input = _max;
    }
    _value = user_input;
    update_slider_widget();
    notifyListeners();
  }
}


/*************************************************************************
 * Function Name: SliderPanel::OnSlider
 * Parameters: wxCommandEvent& event
 * Returns: void
 * Effects: 
 *************************************************************************/
void
SliderPanel::OnSlider(wxScrollEvent& event)
{

  double slider_ratio = ((double) _slider->GetValue() - _slider->GetMin()) / 
    ((double) _slider->GetMax() - _slider->GetMin());
  double new_value = _min + (slider_ratio * (_max - _min));
  _value = new_value;
  update_value_text_widget();
  notifyListeners();
}



/*************************************************************************
 * Function Name: SliderPanel::update_range_text_widgets
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
SliderPanel::update_range_text_widgets()
{
  wxString temp;
  if (_decimal_places == 1) {
	temp.Printf(_T("%0.1f"),_min);
	_min_text->SetLabel(temp); 
	temp.Printf(_T("%0.1f"),_max);
	_max_text->SetLabel(temp);
  }
  else {
	temp.Printf(_T("%0.2f"),_min);
	_min_text->SetLabel(temp); 
	temp.Printf(_T("%0.2f"),_max);
	_max_text->SetLabel(temp);
  }
}


/*************************************************************************
 * Function Name: SliderPanel::notifyListeners
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
SliderPanel::notifyListeners()
{
  for (std::vector<SliderListener *>::iterator iter = _listeners.begin(); iter != _listeners.end(); iter++) {
    SliderListener *listener = *iter;
    listener->sliderChanged (this);
  }
}

/*************************************************************************
 * Function Name: SliderPanel::notifyListeners
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/

void
SliderPanel::setEnabled (bool active)
{
  _value_text_ctrl->Enable(active);
  _slider->Enable(active);
}

void
SliderPanel::OnThumb(wxScrollEvent &event)
{
  cerr << "thumb!" << endl;
}
