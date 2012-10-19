///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __IMAGE_PREVIEW__
#define __IMAGE_PREVIEW__

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/slider.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include "typedefs.h"

///////////////////////////////////////////////////////////////////////////

class qPathwayViz;

///////////////////////////////////////////////////////////////////////////////
/// Class ImagePreview
///////////////////////////////////////////////////////////////////////////////
class ImagePreview : public wxDialog 
{
private:

protected:
	wxPanel* m_panel1;
	wxStaticBitmap* m_screen;

	wxStaticText* m_staticText1;
	wxSlider* m_slider1;
	wxStaticText* m_staticText2;


	wxButton* m_button1;
	wxButton* m_button2;
	wxButton* m_button3;


	// Virtual event handlers, overide them in your derived class
	void OnQualityChanged( wxScrollEvent& event );
	void OnRefresh( wxCommandEvent& event );
	void OnSave( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event ){ Hide(); }
	void OnClose( wxCloseEvent& event ){ Hide(); }
	void OnSize( wxSizeEvent& event );

public:
	ImagePreview( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Image preview"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 745,617 ), long style = wxDEFAULT_DIALOG_STYLE |wxRESIZE_BORDER);
	~ImagePreview();
	
	PROPERTY(qPathwayViz*, _pathway_viz, PathwayViz);

};

#endif //__IMAGE_PREVIEW__
