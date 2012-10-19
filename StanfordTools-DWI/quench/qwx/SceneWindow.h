/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef SCENE_WINDOW_H
#define SCENE_WINDOW_H

#include "wxVTKRenderWindowInteractor.h"
#include "typedefs_quench.h"
#include <string.h>

class SceneWindow : public wxVTKRenderWindowInteractor , public IEventListener
{

 public:
  SceneWindow (wxWindow *parent,
	       wxWindowID id,
	       const wxPoint &pos = wxDefaultPosition,
	       const wxSize &size = wxDefaultSize,
	       long style = wxWANTS_CHARS | wxNO_FULL_REPAINT_ON_RESIZE,
	       const wxString &name = wxPanelNameStr);
  virtual ~SceneWindow();

  bool loadCursors();

  void OnKeyPressed(wxKeyEvent &event);
  void OnKeyReleased(wxKeyEvent &event);

  void OnDoubleClick (wxMouseEvent &event) { SetFocus(); }

  void HideMouseCursor();
  void ShowMouseCursor();

  void SetCursorToSelection();
  void SetCursorToRotation();

  void OnEvent(PEvent evt);

 private:

  wxCursor *_touch_cursors[SELECTION_MODE_NUM_CHOICES];
  wxCursor *_intersect_cursors[SELECTION_MODE_NUM_CHOICES];
  wxCursor *_match_cursors[SELECTION_MODE_NUM_CHOICES];
  
  wxCursor *_rotate_cursor;
  int _selection_tool;
  int _selection_mode;
  
  DECLARE_EVENT_TABLE();
};

#endif
