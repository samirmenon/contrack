///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __ABOUT_DLG__
#define __ABOUT_DLG__

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class MyDialog1
///////////////////////////////////////////////////////////////////////////////
class AboutDlg : public wxDialog 
{
	protected:
		wxPanel* m_panel1;
		wxStaticText* m_staticText3;
		wxStaticText* m_staticText4;
		wxHyperlinkCtrl* m_hyperlink2;
		wxButton* m_button1;
	
	public:
		AboutDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("About Quench"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,227 ), long style = wxDEFAULT_DIALOG_STYLE )
			 : wxDialog( parent, id, title, pos, size, style )
		{
			this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
			wxBoxSizer* bSizer2;
			bSizer2 = new wxBoxSizer( wxVERTICAL );
	
			m_panel1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
			m_panel1->SetBackgroundColour( wxColour( 255, 255, 255 ) );
	
			wxBoxSizer* bSizer3;
			bSizer3 = new wxBoxSizer( wxVERTICAL );
	
			m_staticText3 = new wxStaticText( m_panel1, wxID_ANY, wxT("\n\nQuench - August 4, 2011"), wxDefaultPosition, wxDefaultSize, 0 );
			m_staticText3->Wrap( -1 );
			m_staticText3->SetFont( DEFAULT_FONT );
			bSizer3->Add( m_staticText3, 0, wxALIGN_CENTER|wxALL, 5 );
	
			m_staticText4 = new wxStaticText( m_panel1, wxID_ANY, wxT("\nAnthony Sherbondy (asherbondy@stanfordalumni.org)\nShireesh Agrawal(shireesh@stanford.edu)\nDavid Akers(dakers@cs.stanford.edu)"), wxDefaultPosition, wxDefaultSize, 0 );
			//	m_staticText4->SetSizeHints(-1, -1, 600, -1, -1, -1); // set maximum width
			m_staticText4->Wrap( -1 );
			m_staticText4->SetFont( DEFAULT_FONT );
			bSizer3->Add( m_staticText4, 0, wxALIGN_CENTER|wxALL, 5 );
	
			m_hyperlink2 = new wxHyperlinkCtrl( m_panel1, wxID_ANY, wxT("http://white.stanford.edu/newlm/index.php/QUENCH"), wxT("http://white.stanford.edu/newlm/index.php/QUENCH"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
			m_hyperlink2->SetFont( DEFAULT_FONT );
			bSizer3->Add( m_hyperlink2, 0, wxALIGN_CENTER|wxALL, 5 );
	
			m_panel1->SetSizer( bSizer3 );
			m_panel1->Layout();
			bSizer3->Fit( m_panel1 );
			bSizer2->Add( m_panel1, 1, wxEXPAND | wxALL, 0 );
	
			m_button1 = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, DEFAULT_BUTTON_SIZE, 0 );
			m_button1->SetFont( DEFAULT_FONT );
			bSizer2->Add( m_button1, 0, wxALIGN_RIGHT|wxALL, 8 );
			
			this->SetSizeHints(-1, -1, 600, -1, -1, -1); // set maximum width
			this->SetSizerAndFit( bSizer2, false );
			this->Layout();

			// Connect Events
			m_button1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( AboutDlg::OnClick ), NULL, this );
		}	
		virtual void OnClick( wxCommandEvent& event ){ Close(); }
};

#endif
