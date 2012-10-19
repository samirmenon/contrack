#ifndef STATS_PANEL_H
#define STATS_PANEL_H

#define wxUSE_NOTEBOOK 1

class RangeSlider;
class ScoreThresholdListener;
class StatsCheckboxArray;

#include "typedefs_quench.h"
#include <wx/notebook.h>
#include "wx/statline.h"
#include "PDBHelper.h"
#include "PathwayGroup.h"
#include "ROIPanel.h"
///////////////////////////////////////////////////////////////////////////


//! the panel displayed in the refine selection dialog, handles stats based filtering 
class StatsPanel : public wxDialog , public IEventListener , public IEventSource, public ISerializable
{
protected:
	// GUI elements
	wxStaticText* m_staticText1;
	wxPanel* statsCheckboxArray;
	wxStaticLine* m_staticline1;
	wxStaticText* m_staticText2;
	wxNotebook* _notebook;
	wxStaticLine* m_staticline2;
	wxStaticText* m_staticText3;
	wxButton* m_button1;
	wxButton* m_button2;
	wxButton* m_button3;
	ScoreThresholdListener *_listener;
	wxPanel *_dummyStatsPanel;
	ROIPanel *_voiPanel;
	wxBoxSizer *_statsSizer;
	wxBoxSizer* buttonSizer;
	int lastPositionX, lastPositionY;
	wxPanel* outputPanel;

	const static int RESET_BUTTON_ID = 501;
	const static int APPLY_BUTTON_ID = 502;
	const static int CANCEL_BUTTON_ID = 503;

	bool bTempWasShown;
	PROPERTY_READONLY(StatsCheckboxArray *, _cbArray, StatsCheckboxArray_)
	PROPERTY_READONLY(std::vector<RangeSlider *>, _vRS, RangeSliders)

	DECLARE_EVENT_TABLE()

public:
	StatsPanel( wxWindow* parent, const wxString& title = wxEmptyString);
	~StatsPanel();
	//! Updates the ranges of different stats
	void Update(PDBHelper &helper, PathwayGroupArray &array);
	void OnClose(wxCloseEvent &e);
	void OnReset(wxCommandEvent &event);
	void OnApply(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
	//! Shows the refine selection or voi page
	void ShowPage(int pageId);

	//! Returns the pointer to the ROI panel
	ROIPanel* ROIPanel_(){return _voiPanel;}
	void TempShow(bool show=true);

	void Clear();
	void OnEvent(PEvent evt);

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);
};
#endif
