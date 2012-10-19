/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef VOI_MANAGER_H
#define VOI_MANAGER_H

#include "SliderListener.h"

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class VOIListener;

#include "DTI_typedefs.h"
#include <vector>

class VOIPanel : public wxPanel, SliderListener
{
 public:
  VOIPanel(wxWindow* parent);
  ~VOIPanel();

  void setID (const char *id);
  void setLabel (const char *label);
  void setValues (const double pos[3], const double size[3]);
  void setValueRanges (const double minSize[3], const double maxSize[3],
		       const double minPos[3], const double maxPos[3]);
  void setVOIState (int activeID, int numVOIs, int *roiIDs, VOIMotionType motionType,
		    int motionID);
  void addListener (VOIListener *listener);
  void setUnits (int unitType);

 private:

  void setVOIPanelActive(bool active);

  int _units;
  double _min_pos[3];
  double _max_pos[3];
  //  void OnSlider(wxCommandEvent& event);
  //  void OnSliderText(wxCommandEvent& event);
  virtual void sliderChanged (SliderPanel *panel);
  void OnEnter(wxCommandEvent& event);
  void OnKeyDown(wxCommandEvent& event);
  void OnNextVOIButton (wxCommandEvent& event);
  void OnPrevVOIButton (wxCommandEvent& event);
  void OnSymmetryChoice (wxCommandEvent& event);
  void OnVOIChoice (wxCommandEvent& event);

  void rebuildSymmetryChoice (int activeID, int numVOIs, int *roiIDs,
			      VOIMotionType motionType, int motionID);
  void rebuildVOIChoice (int activeID, int numVOIs, int *roiIDs);

  std::vector <VOIListener *> _listeners;
  wxTextCtrl *_label_text;
  wxTextCtrl *_id_text;
  

  //  wxButton *_roi_prev_button;
  //  wxButton *_roi_next_button;
  //  wxStaticText *_roi_state_text;

  wxChoice *_roi_choice;

  wxChoice *_symmetry_choice;

  int *_ids;
  int _current_id;

  SliderPanel *_x_size_slider;
  SliderPanel *_y_size_slider;
  SliderPanel *_z_size_slider;
  SliderPanel *_x_pos_slider;
  SliderPanel *_y_pos_slider;
  SliderPanel *_z_pos_slider;

  wxBoxSizer *_topVOISizer;
 
  DECLARE_EVENT_TABLE()
};

#endif
