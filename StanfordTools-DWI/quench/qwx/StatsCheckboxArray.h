#ifndef StatsCheckboxArray_H
#define StatsCheckboxArray_H

#include "typedefs_quench.h"

class StatsPanel;
class DTIPathwayAssignment;
class PathwayGroupArray;

//! This class represents the graph and buttons displayed in the refine selection panel
class StatsCheckboxArray : public wxPanel , public IEventSource
{
public:
	StatsCheckboxArray(wxWindow *parent);
	~StatsCheckboxArray();
	//! Redraws the screen with the new data
	void Update(DTIPathwayAssignment *assn, PathwayGroupArray *array);

protected:
	void OnCheckBox( wxCommandEvent& event );
	void OnPaint(wxPaintEvent& event);  
	void OnLeftUp(wxMouseEvent &event);
	void OnMouseMove(wxMouseEvent &event);
	//! Gets the swatch under mouse cursor, -1 if none
	int GetSwatchUnderCursor(int x, int y);
	
	int _ymax;
    wxPaintDC *_dc;
	wxCheckBox* _assignmentLocked;
	DTIPathwayAssignment *_assn; 
	PathwayGroupArray *_array;
	int hoverGroup;
};

#endif

