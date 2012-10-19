///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __COLOR_MAP_PANEL
#define __COLOR_MAP_PANEL

#include "typedefs_quench.h"
///////////////////////////////////////////////////////////////////////////

class QuenchFrame;

///////////////////////////////////////////////////////////////////////////////
/// Class ColorMapPanel
///////////////////////////////////////////////////////////////////////////////
class ColorMapPanel : public wxDialog , public IEventSource
{
	protected:
		// Virtual event handlers, overide them in your derived class
		void OnKillFocus( wxFocusEvent& event ){ Show(false); PopListeners(); }
		void OnLeftDown( wxMouseEvent& event );		
		void OnPaint( wxPaintEvent& event );
		void OnActivate( wxActivateEvent& event );
		std::vector< PwxBitmap > _bitmaps;
		bool _enabled;
		PROPERTY(int, _selectedColormapIndex , SelectedColorMap);
		PROPERTY_READONLY(std::vector<ColorMap>, _colorMaps, ColorMaps);
		IListenerList _oldList;

	public:
		ColorMapPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 256,81 ), long style = wxDEFAULT_DIALOG_STYLE );
		void SetEnabled(bool b){_enabled = b;}
		void FillBitmapWithColorMap(wxBitmap *bmp, int index);
		void ShowWithListener(IEventListener* listener){ PushListeners(); push_back(listener); Show(); }
		void PushListeners(){_oldList.clear(); _oldList = *this; clear(); }
		void PopListeners(){ if(_oldList.size()) { clear(); (*(IListenerList*)this) = _oldList; _oldList.clear(); } }
	QuenchFrame* _qFrame;
};

#endif 
