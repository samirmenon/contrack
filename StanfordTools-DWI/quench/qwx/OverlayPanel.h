///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef _OVERLAY_PANEL
#define _OVERLAY_PANEL

#include "typedefs_quench.h"
#include "OverlayItem.h"
#include <wx/statline.h>
#include <wx/spinctrl.h>

#include <DTIVolume.h>
#include "SubjectData.h"
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class OverlayPanel
///////////////////////////////////////////////////////////////////////////////
class OverlayItem;
class RangeSlider;
class qVolumeViz;
class ColorMapPanel;

class OverlayPanel : public wxDialog , public IEventSource, public IEventListener, public std::vector<POverlayItem>, public ISerializable
{
protected:
	wxBoxSizer* bSizerOverlayItems;
	wxPanel* m_panel1;
	wxStaticText* m_staticText41;
	wxStaticLine* m_staticline3;
	wxStaticText* m_staticText51;
	wxStaticLine* m_staticline2;
	wxStaticBitmap* m_bitmap1;
	wxSlider* m_slider2;
	wxSlider* m_slider1;
	wxStaticBitmap* m_bitmap2;
	wxStaticBitmap* m_bitmap77;
	wxSpinCtrl* m_spinCtrl1;
	wxStaticBitmap* m_bitmap3;
	wxSlider* m_slider3;
	wxStaticBitmap* m_bitmap4;
	wxStaticText* m_staticText3;
	wxStaticText* m_staticText4;
	wxStaticText* m_staticText5;

	// Virtual event handlers, overide them in your derived class
	void OnAddOverlayItem( wxCommandEvent& event ){ event.Skip(); }
	void OnRemoveOverlayItem( wxCommandEvent& event ){ event.Skip(); }
	void OnSliderUpdate( wxCommandEvent& event );
	RangeSlider *_rangeSlider;
	PROPERTY_READONLY(int, _selected, Selected);
	PROPERTY(qVolumeViz*, _volviz, VolViz);

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);

public:
	OverlayPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Background and Overlays"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 458,220 ), long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );
	~OverlayPanel();
	void Clear();
	void AddVolume(VolumeInfo &info, ColorMapPanel* cmpanel);
	void SelectItem(OverlayItem *newItem);
	void SelectItem(int i);
	void SetBackgroundVolume(OverlayItem *item);
	void SetBackgroundVolume(int i);
	void SetOverlayVisibilty(OverlayItem *item , bool vis);

	void OnEvent(PEvent e);
};

#endif 
