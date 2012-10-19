#include "ColorMapPanel.h"
#include "PerPointColor.h"
#include "RangeSlider.h"
#include "wx/rawbmp.h"
using namespace std;

///////////////////////////////////////////////////////////////////////////

PerPointColor::PerPointColor( wxWindow* parent, ColorMapPanel* cmpanel, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	_cmapPanel = cmpanel;
	
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Statistic"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer2->Add( m_staticText1, 0, wxALL, 5 );
	
	wxArrayString m_choice1Choices;
	m_choice1 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice1Choices, 0 );
	m_choice1->SetSelection( 0 );
	bSizer2->Add( m_choice1, 0, wxALL, 5 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("       Color Map"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer2->Add( m_staticText2, 0, wxALL, 5 );
	
	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_bitmap1, 0, wxALL, 5 );
	
	bSizer1->Add( bSizer2, 1, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( m_staticline1, 0, wxEXPAND, 5 );

	_rangeSlider = new RangeSlider(this, 0, " ",0,100);
	_rangeSlider->push_back(this);
	bSizer1->Add( _rangeSlider, 1, wxEXPAND, 5 );

	this->SetSizer( bSizer1 );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( PerPointColor::OnClose ) );
	m_bitmap1->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( PerPointColor::OnColorMapClick ), NULL, this );
	m_choice1->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( PerPointColor::OnSelectedStatChanged ), NULL, this );

	dummy_bmp = new wxBitmap(16,16,32);
	m_bitmap1->SetBitmap(*dummy_bmp);
	_colormap_index = 0;
	OnUpdateColorMap(_colormap_index);
	SetEnabled(false);
}
PerPointColor::~PerPointColor()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( PerPointColor::OnClose ) );
	m_bitmap1->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( PerPointColor::OnColorMapClick ), NULL, this );
	m_choice1->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( PerPointColor::OnSelectedStatChanged ), NULL, this );
}
bool PerPointColor::Show(bool show)
{
	if(show)
	{
		string stat_name(m_choice1->GetStringSelection().c_str());
		NotifyAllListeners(PEvent( new EventShowPerPointColorPanel(stat_name ,_colormap_index, _rangeSlider->Left(), _rangeSlider->Right()) ));
	}
	else 
		NotifyAllListeners(PEvent( new Event(HIDE_PERPOINTCOLOR_PANEL)));
	return wxDialog::Show(show);
}
void PerPointColor::OnSelectedStatChanged( wxCommandEvent& event ) 
{ 
	SetRangeSliderValues(); 
	Show();
}
void PerPointColor::OnColorMapClick(wxMouseEvent& event )
{
	// get the colormap panel
	ColorMapPanel *cmp = this->ColorMapPanel_();
	cmp->ShowWithListener(this);
	
	// show the panel just next to this overlay item
	wxPoint pos;
	m_bitmap1->GetScreenPosition(&pos.x, &pos.y);
	pos.x+=m_bitmap1->GetSize().x;
	cmp->SetPosition(pos);
}
void PerPointColor::OnUpdateColorMap(int colorMap)
{
	// update the colormap icon 
	_colormap_index = colorMap;
	wxBitmap bmp = m_bitmap1->GetBitmap();
	
	this->ColorMapPanel_()->FillBitmapWithColorMap(&bmp, colorMap);
	m_bitmap1->Refresh();
	GetParent()->Refresh();

	// dakers added this - we were refreshing everything except the display
	// itself:
	Refresh();
}
void PerPointColor::OnEvent(PEvent evt)
{
	switch(evt->EventType())
	{
	case RANGE_SLIDER_UPDATED:
		Show();
		break;
	case COLOR_MAP_CHANGED:
		{
			EventColorMapChanged *e = dynamic_cast<EventColorMapChanged*>(evt.get());
			OnUpdateColorMap(e->Value());
			Show();
		}
		break;
	}
}
void PerPointColor::SetEnabled(bool enabled)
{
	_rangeSlider->Enable(enabled);
	m_bitmap1	->Enable(enabled);
	m_choice1	->Enable(enabled);
}
void PerPointColor::SetRangeSliderValues()
{
	int i = m_choice1->GetSelection();
	string name(_statistic_header[i]._aggregate_name);
	_rangeSlider->SetName	(name); 
	_rangeSlider->SetMin	(_statistic_threshold[i]._min);
	_rangeSlider->SetMax	(_statistic_threshold[i]._max);
	_rangeSlider->SetLeft	(_statistic_threshold[i]._left);
	_rangeSlider->SetRight	(_statistic_threshold[i]._right);
	_rangeSlider->Refresh();
}
void PerPointColor::AddStatistic(DTIPathwayStatisticHeader& header, DTIPathwayStatisticThreshold &threshold)
{
	_statistic_header.push_back(header);
	_statistic_threshold.push_back(threshold);
	strcpy(_statistic_header.back()._aggregate_name, header._aggregate_name);
	m_choice1->Append(header._aggregate_name);

	//if first item then select it
	if(_statistic_header.size() == 1)
	{
		m_choice1->Select(0);
		SetRangeSliderValues();
	}
	SetEnabled();
}
void PerPointColor::Clear()
{
	m_choice1->Clear();
	_statistic_header.clear();
	_statistic_threshold.clear();
	SetEnabled(false);
}
