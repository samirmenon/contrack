///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef _OVERLAY_ITEM
#define _OVERLAY_ITEM

#include "typedefs_quench.h"
///////////////////////////////////////////////////////////////////////////

class ColorMapPanel;
///////////////////////////////////////////////////////////////////////////////
/// Class OverlayItem
///////////////////////////////////////////////////////////////////////////////
class OverlayItem : public wxPanel , public IEventSource, public IEventListener
{
	protected:
		wxBitmap *dummy_bmp;
		wxStaticBitmap* m_bitmap1;
		wxCheckBox* m_checkBox2;
		wxRadioButton* m_radioBtn2, *dummyLinuxRB;
		wxStaticText* m_Name;
		wxStaticText *m_txt1, *m_txt2, *m_txt3, *m_txt4; 
		// Virtual event handlers, overide them in your derived class
		void OnOverlayVisibilityChanged( wxCommandEvent& event );
		void OnSelect( wxMouseEvent& event ) { OnSelected(); }
		void OnColorMapClick(wxMouseEvent& event );
		IListenerList _oldColorMapPanelListeners;
	ColorMapPanel* _cmapPanel;

	public:
		OverlayItem( wxWindow* parent, std::string name,  bool bIsBkgnd, bool bIsOverlay, int colorMap, ColorMapPanel* cmpanel, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 166,15 ), long style = wxTAB_TRAVERSAL );
		~OverlayItem();
		void OnSelected();
		void Select(bool select) { SetBackgroundColour(select ? wxColour(81,168,255) : GetParent()->GetBackgroundColour()); }
		void SelectBackground(bool select){ m_radioBtn2->SetValue(select); dummyLinuxRB->SetValue(!select); }
		bool IsBackground() { return m_radioBtn2->GetValue(); }
		void SelectOverlay(bool select) { m_checkBox2->SetValue(select); }
		void OnUpdateColorMap(int colorMap);
		void OnEvent(PEvent e);
		void OnBackgroundVolumeChanged( wxCommandEvent& event );
};
typedef std::vector<POverlayItem> VPOverlayItem;

#endif 
