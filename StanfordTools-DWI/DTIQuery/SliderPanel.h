/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef SLIDER_PANEL_H
#define SLIDER_PANEL_H

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <vector>

class SliderListener;

class SliderPanel : public wxPanel
{
    public:
        SliderPanel(wxWindow* parent, wxString name, int decimalPlaces);

	void setValue (double value);
	
	double getValue() { return _value; }

	void setDecimalPlaces (int dec) { _decimal_places = dec; update_value_text_widget(); }
	
	void setRange (double minValue, double maxValue);

	double getMin() { return _min; }
	double getMax() { return _max; }

	void setEnabled (bool active);

	void addListener (SliderListener *listener);
        
    private:

	void notifyListeners();

	    int _decimal_places;

        double _value, _min, _max;
        
        wxStaticText* _label_text;
        wxTextCtrl* _value_text_ctrl;
        wxStaticText* _min_text;
        wxSlider* _slider;
        wxStaticText* _max_text;

        const static int INTERNAL_SLIDER_SCALE = 500;

        void update_value_text_widget();
	void update_range_text_widgets();
        void update_slider_widget();

        void OnValueText(wxCommandEvent& event);
        void OnSlider(wxScrollEvent& event);
	void OnThumb(wxScrollEvent& event);

	std::vector <SliderListener *> _listeners;

        DECLARE_EVENT_TABLE()
};

#endif




