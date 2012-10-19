/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FILTER_PANEL_H
#define FILTER_PANEL_H

class FilterListener;

#include "SliderListener.h"

#include "wx/wxprec.h"

#include <util/typedefs.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vector>


class FilterPanel : public wxPanel, SliderListener {
 public:
  FilterPanel(wxWindow *parent);
  ~FilterPanel();

  void setFilterString (DTIQueryOperation op, const char *str);

  void setValueRanges (double minLength, double maxLength,
		       double minMedianFA, double maxMedianFA,
		       double minMinFA, double maxMinFA,
			   double minMaxAngle, double maxMaxAngle);

  void setUnits (int unitType);

  void setLengths (double minLength, double maxLength);
  void setCurvatureValues (double minCurv, double maxCurv);
  void setFAValues (double minFA, double maxFA);
  void setAlgorithm (DTIPathwayAlgorithm algo);

  void initialize();

  void addListener (FilterListener *listener);

 private:

  void enableWidgets (bool pred);

  virtual void sliderChanged (SliderPanel *panel);

  void OnAlgorithmChoice (wxCommandEvent &event);
 
  void OnFilterModeChoice (wxCommandEvent &event);
  void OnFilterText(wxCommandEvent& event);
  void OnEnter(wxCommandEvent& event);

  SliderPanel *_min_length_slider;
  SliderPanel *_max_length_slider;
  SliderPanel *_min_median_fa_slider;
  SliderPanel *_max_median_fa_slider;
  SliderPanel *_min_max_angle_slider;
  SliderPanel *_max_max_angle_slider;
  //SliderPanel *_min_min_fa_slider;
  //SliderPanel *_max_min_fa_slider;

  wxChoice *_filter_mode_choice;
  wxTextCtrl *_filter_text;

  wxChoice *_algorithm_choice;
  std::vector <FilterListener *> _listeners;
  int _units;

  DECLARE_EVENT_TABLE()

};

#endif
