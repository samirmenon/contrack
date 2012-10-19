///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "OverlayItem.h"
#include "wx/rawbmp.h"
#include "ColorMapPanel.h"
///////////////////////////////////////////////////////////////////////////

OverlayItem::OverlayItem( wxWindow* parent, std::string name,  bool bIsBkgnd, bool bIsOverlay, int colorMap, ColorMapPanel* cmpanel, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) 
	: wxPanel( parent, id, pos, size, style )
{
	_cmapPanel = cmpanel;
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );

	// these filler text are used to provide spacing between the other items
	m_txt1 = new wxStaticText( this, wxID_ANY, "  ", wxDefaultPosition, wxDefaultSize, 0 );m_txt1->Wrap( -1 );
	m_txt2 = new wxStaticText( this, wxID_ANY, "        ", wxDefaultPosition, wxDefaultSize, 0 );m_txt2->Wrap( -1 );
	m_txt3 = new wxStaticText( this, wxID_ANY, "    ", wxDefaultPosition, wxDefaultSize, 0 );m_txt3->Wrap( -1 );
	m_txt4 = new wxStaticText( this, wxID_ANY, "              ", wxDefaultPosition, wxDefaultSize, 0 );m_txt4->Wrap( -1 );

	m_txt1->SetFont( DEFAULT_FONT );
	m_txt2->SetFont( DEFAULT_FONT );
	m_txt3->SetFont( DEFAULT_FONT );
	m_txt4->SetFont( DEFAULT_FONT );

	bSizer1->Add( m_txt1, 0, wxALL, 5 );

	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_bitmap1, 0, wxALIGN_CENTER|wxALL, 5 );
	bSizer1->Add( m_txt2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_checkBox2 = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxPoint( -1,-1 ), wxDefaultSize, 0 );
	
	bSizer1->Add( m_checkBox2, 0, wxALIGN_CENTER|wxALL, 5 );
	bSizer1->Add( m_txt3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	dummyLinuxRB = new wxRadioButton( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_SINGLE );
	dummyLinuxRB->Show(false);
	//bSizer1->Add( dummyLinuxRB, 0, wxALIGN_CENTER|wxALL, 5 );

	m_radioBtn2 = new wxRadioButton( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_SINGLE );
	bSizer1->Add( m_radioBtn2, 0, wxALIGN_CENTER|wxALL, 5 );
	bSizer1->Add( m_txt4, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_Name = new wxStaticText( this, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_Name->Wrap( -1 );
	m_Name->SetFont( DEFAULT_FONT );
	bSizer1->Add( m_Name, 1, wxALIGN_CENTER|wxALL, 5 );
	
	this->SetSizer( bSizer1 );
	this->Layout();
	
	// Connect Events
	m_checkBox2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( OverlayItem::OnOverlayVisibilityChanged ), NULL, this );
	m_radioBtn2->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( OverlayItem::OnBackgroundVolumeChanged ), NULL, this );

	m_Name->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	this->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_txt1->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_txt2->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_txt3->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_txt4->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_bitmap1->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnColorMapClick ), NULL, this );
	
	m_Name->SetLabel(name.c_str());
	m_checkBox2->SetValue(bIsOverlay);
	m_radioBtn2->SetValue(bIsBkgnd);

	dummy_bmp = new wxBitmap(16,16,32);
	m_bitmap1->SetBitmap(*dummy_bmp);
	OnUpdateColorMap(colorMap);
}

OverlayItem::~OverlayItem()
{
	// Disconnect Events
	m_checkBox2->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( OverlayItem::OnOverlayVisibilityChanged ), NULL, this );
	m_radioBtn2->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( OverlayItem::OnBackgroundVolumeChanged ), NULL, this );
	m_Name->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	this->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_txt1->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_txt2->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_txt3->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_txt4->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnSelect ), NULL, this );
	m_bitmap1->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( OverlayItem::OnColorMapClick ), NULL, this );
	delete m_bitmap1;
	delete m_txt1;
	delete m_txt2;
	delete m_txt3;
	delete m_txt4;
	delete m_checkBox2;
	delete m_radioBtn2;
	delete m_Name;
	delete dummy_bmp;
	delete dummyLinuxRB;
}

void OverlayItem::OnOverlayVisibilityChanged( wxCommandEvent& event )
{ 
	// notify quench that the overlay visibility has been changed
	PEvent evt(new EventOverlayVisibilityChanged(m_checkBox2->GetValue()));
	
	// select this overlay item
	OnSelected();
	NotifyAllListeners(evt); 
}

void OverlayItem::OnSelected()
{
	// notify OverlayPanel that this overlay item was selected
	PEvent evt(new EventOverlayItemSelected(this));
	NotifyAllListeners(evt);
}
void OverlayItem::OnBackgroundVolumeChanged( wxCommandEvent& event )
{
	// select this overlay item
	OnSelected();

	// notify Quench that background image has changed
	NotifyAllListeners( PEvent( new Event(BACKGROUND_CHANGED_IN_OVERLAY_ITEM)));
	
	// this dummy linux button is needed to complete a 'radio group' 
	dummyLinuxRB->SetValue(!m_radioBtn2->GetValue());
}

void OverlayItem::OnColorMapClick(wxMouseEvent& event )
{
	// select this item
	OnSelected();
	
	// get the colormap panel
	ColorMapPanel *cmp = _cmapPanel; //the_global_frame->ColorMapPanel_();
	cmp->ShowWithListener(this);
	
	// show the panel just next to this overlay item
	wxPoint pos;
	m_bitmap1->GetScreenPosition(&pos.x, &pos.y);
	pos.x+=m_bitmap1->GetSize().x;
	cmp->SetPosition(pos);
}

void OverlayItem::OnUpdateColorMap(int colorMap)
{
	// update the colormap icon 
	wxBitmap bmp = m_bitmap1->GetBitmap();
	//the_global_frame->ColorMapPanel_()->FillBitmapWithColorMap(&bmp, colorMap);
	_cmapPanel->FillBitmapWithColorMap(&bmp, colorMap);
	m_bitmap1->Refresh();
	GetParent()->Refresh();
}

void OverlayItem::OnEvent(PEvent evt)
{
	EventColorMapChanged *e = dynamic_cast<EventColorMapChanged*>(evt.get());
	if(e)
	{
		// notify Overlay to update the colormap
		NotifyAllListeners(evt);
		OnUpdateColorMap(e->Value());
		OnSelected();
	}
}
