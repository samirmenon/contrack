#include "ImagePreview.h"
#include <wx/filedlg.h>
#include <wx/rawbmp.h>
#include "vtkInteractorStyleQuench.h"
#include "qPathwayViz.h"
#include <wx/bitmap.h>

//#include "Quench.h"
#include "qPathwayViz.h"

ImagePreview::ImagePreview( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	_pathway_viz = NULL;
	
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_panel1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxTAB_TRAVERSAL|wxVSCROLL|wxHSCROLL );
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	
	m_screen = new wxStaticBitmap( m_panel1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	m_screen->SetMinSize( wxSize( 400,-1 ) );
	
	bSizer4->Add( m_screen, 1, wxALL|wxEXPAND, 5 );
	
	m_panel1->SetSizer( bSizer4 );
	m_panel1->Layout();
	bSizer4->Fit( m_panel1 );
	bSizer1->Add( m_panel1, 1, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Lines"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer3->Add( m_staticText1, 0, wxALL, 5 );
	
	m_slider1 = new wxSlider( this, wxID_ANY, 0, 0, 1, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_slider1->SetMinSize( wxSize( 150,-1 ) );
	
	bSizer3->Add( m_slider1, 0, wxALL, 5 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("Tubes"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer3->Add( m_staticText2, 0, wxALL, 5 );
	
	
	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bSizer1->Add( bSizer3, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_button1 = new wxButton( this, wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_button1, 0, wxALL, 5 );
	
	m_button2 = new wxButton( this, wxID_ANY, wxT("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_button2, 0, wxALL, 5 );
	
	m_button3 = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_button3, 0, wxALL, 5 );
	
	
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bSizer1->Add( bSizer2, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSizer1 );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( ImagePreview::OnClose ) );
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( ImagePreview::OnSize ) );
	m_slider1->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_button1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImagePreview::OnRefresh ), NULL, this );
	m_button2->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImagePreview::OnSave ), NULL, this );
	m_button3->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImagePreview::OnCancel ), NULL, this );
}

ImagePreview::~ImagePreview()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( ImagePreview::OnClose ) );
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( ImagePreview::OnSize ) );
	m_slider1->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_slider1->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( ImagePreview::OnQualityChanged ), NULL, this );
	m_button1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImagePreview::OnRefresh ), NULL, this );
	m_button2->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImagePreview::OnSave ), NULL, this );
	m_button3->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImagePreview::OnCancel ), NULL, this );
}
void ImagePreview::OnQualityChanged( wxScrollEvent& event ){ event.Skip(); }
void ImagePreview::OnRefresh( wxCommandEvent& event )
{ 
	int width, height;
	unsigned char *pixels ;
	_pathway_viz->TakeScreenshot( (void**)&pixels, width , height, m_slider1->GetValue());
	wxBitmap bmp(width, height, 24);
    wxNativePixelData data( bmp );
    if ( !data )
        return;

    wxNativePixelData::Iterator p(data);
	int off = 0;
    for ( int y = 0; y < height; ++y )
    {
        wxNativePixelData::Iterator rowStart = p;
		off = (height-y-1)*width*3;
        for ( int x = 0; x < width; ++x, ++p, off+=3 )
        {
            p.Red()   = pixels[off+0];
            p.Green() = pixels[off+1];
            p.Blue()  = pixels[off+2];
        }
        p = rowStart;
        p.OffsetY(data, 1);
    }

	delete []pixels;
	m_screen->SetBitmap(bmp);
	
	m_screen->FitInside();
	Layout();
}
void ImagePreview::OnSave( wxCommandEvent& event )
{ 
	wxFileDialog saveFileDialog(this, _("Save Screenshot file"), "", "", "PNG files (*.png)|*.png", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	if (saveFileDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...
	m_screen->GetBitmap().SaveFile( saveFileDialog.GetPath(), wxBITMAP_TYPE_PNG );
}
void ImagePreview::OnSize( wxSizeEvent& event )
{ 
	event.Skip();
	Refresh();
}
