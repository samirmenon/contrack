///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "typedefs_quench.h"
#include "OverlayPanel.h"
#include "Overlay.h"
#include "OverlayItem.h"
#include "RangeSlider.h"
#include <wx/mstream.h>
#include "icons/opacity.h"
#include "icons/contrast_less.h"
#include "icons/contrast_more.h"
#include "icons/brightness_dim.h"
#include "icons/brightness_full.h"

#include "ColorMapPanel.h"
#include "qVolumeViz.h"

wxBitmap CreateBitmapFromMemory(void *data, size_t size, int type)
{
	wxMemoryInputStream memstream(data, size);
	return wxBitmap( wxImage( memstream , type) );
}

using namespace std;
///////////////////////////////////////////////////////////////////////////

OverlayPanel::OverlayPanel( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );

	m_panel1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxTAB_TRAVERSAL );
	bSizerOverlayItems = new wxBoxSizer( wxVERTICAL );

	m_staticText41 = new wxStaticText( m_panel1, wxID_ANY, wxT("ColorMap      Visible      Background      Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->SetFont( DEFAULT_FONT );
	m_staticText41->Wrap( -1 );
	bSizerOverlayItems->Add( m_staticText41, 0, wxALL|wxEXPAND, 5 );

	m_staticline3 = new wxStaticLine( m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizerOverlayItems->Add( m_staticline3, 0, wxEXPAND, 5 );

	m_panel1->SetSizer( bSizerOverlayItems );
	//m_panel1->Layout();
	bSizerOverlayItems->Fit( m_panel1 );
	bSizer2->Add( m_panel1, 1, wxEXPAND | wxALL, 5 );

	wxBoxSizer* bSizerRangeSlider;
	bSizerRangeSlider = new wxBoxSizer( wxVERTICAL );

	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizerRangeSlider->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );

	_rangeSlider = new RangeSlider(this, 0, "Color Range",0,100);
	_rangeSlider->SetWindowStyle(wxNO_BORDER);
	_rangeSlider->push_back(this);
	bSizerRangeSlider->Add(_rangeSlider,wxSizerFlags(0).Expand().Border());

	wxBoxSizer* bSizer10;

	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("  Transparent"), wxDefaultPosition, wxDefaultSize, 0 );

	m_staticText4->SetFont( DEFAULT_FONT );
	m_staticText4->Wrap( -1 );

	bSizer10->Add( m_staticText4, 0, wxALIGN_LEFT, 5 );

	bSizer10->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText5 = new wxStaticText( this, wxID_ANY, wxT("Opaque  "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );

	m_staticText5->SetFont( DEFAULT_FONT );
	m_staticText5->Wrap( -1 );

	bSizer10->Add( m_staticText5, 0, wxALIGN_LEFT, 5 );

	bSizerRangeSlider->Add( bSizer10, 0, wxEXPAND, 5 );

	m_slider1 = new wxSlider( this, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );

	bSizerRangeSlider->Add( m_slider1, 1, wxEXPAND, 5 );

	bSizer2->Add( bSizerRangeSlider, 0, wxEXPAND, 5 );
	m_staticText51 = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_staticText51, 0, wxALL, 0 );

	this->SetSizer( bSizer2 );
	//this->Layout();

	_selected=-1;
	// Connect Events
	m_slider1->Connect( wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( OverlayPanel::OnSliderUpdate ), NULL, this);
}

OverlayPanel::~OverlayPanel()
{
	// Disconnect Events	
	//SAFE_DELETE( m_staticline3);
	SAFE_DELETE( _rangeSlider);
	SAFE_DELETE( m_slider1);
		
	//delete m_panel1;
}
void OverlayPanel::AddVolume(VolumeInfo &info, ColorMapPanel* cmpanel)
{
	int loc = (int)VPOverlayItem::size();

	// get the overlay corresponding to this volume
	Overlay *o = _volviz->GetOverlay(loc).get();

	//Deselect previous item and select this guy
	OverlayItem *overlay_item = new OverlayItem(m_panel1, info.Name(), true, o->Visible(), o->ColorMapIndex(), cmpanel);
	POverlayItem oi( overlay_item);
	VPOverlayItem::push_back(oi);
	oi->push_back(this);

	// add this overlay item 
	bSizerOverlayItems->Add(oi.get(),1,wxALL | wxEXPAND);
	wxSize size = GetSize();

	// increase the size to make room for this item
	size.y+=30;
	SetSize(size);

	//Connect the OverlayItem to the Overlay
	oi->push_back(o);

	PEvent evt(new EventOverlayItemSelected(oi.get()));
	OnEvent(evt);

	// deselect the previous selected background image
	for(int i=0; i < (int)VPOverlayItem::size(); i++)
		if(i != _selected)
			VPOverlayItem::operator[](i)->SelectBackground(false);

	Layout();
}
void OverlayPanel::OnEvent(PEvent evt)
{
	int i;
	switch(evt->EventType())
	{
	case RANGE_SLIDER_UPDATED:
		//Update Overlay range
		{
			//PEvent evt(new EventOverlayRangeChanged(_rangeSlider->Left(), _rangeSlider->Right()));
			//the_global_frame->Interactor()->VolumeViz()->GetOverlay(_selected)->OnEvent(evt);
			Overlay *o = _volviz->GetOverlay(_selected).get();
			o->SetRange(_rangeSlider->Left(), _rangeSlider->Right());
			//NotifyAllListeners(PEvent(new EventOverlayColorChanged(o))); 
			
			// if this item is the background too, notify quench controller to update the mapping info
			if ( VPOverlayItem::operator[](_selected)->IsBackground() )
				NotifyAllListeners(PEvent(new EventUpdateBackground(1)));
			
			NotifyAllListeners(PEvent(new EventUpdateMesh(1)));
		}
		break;
	case BACKGROUND_CHANGED_IN_OVERLAY_ITEM:
		// deselect the previous selected background image
		for(i=0; i < (int)VPOverlayItem::size(); i++)
			if(i != _selected)
				VPOverlayItem::operator[](i)->SelectBackground(false);
			
		// select this one
		NotifyAllListeners(PEvent(new EventSetBackground(_selected))); //the_global_frame->_qCtrlr->SetBackgroundVolume(_selected);
		break;
	case OVERLAY_ITEM_SELECTED:
		{
			//the_global_frame->_qCtrlr->UpdateMesh();
			EventOverlayItemSelected*e = dynamic_cast<EventOverlayItemSelected*>(evt.get());
			size_t i = 0;
			// figure out the selected selected item
			for(; i < VPOverlayItem::size(); i++)
				if(VPOverlayItem::operator[](i).get() == e->Value())
					break;
			// if it is already selected do nothing
			if(i == VPOverlayItem::size() || i == _selected)
				return;
			// if nothing was selected, unselect the last one
			if(_selected != -1)
				VPOverlayItem::operator[](_selected)->Select(false);

			// select the new item
			_selected = (int)i;
			VPOverlayItem::operator [](_selected)->Select(true);

			//Setup Ranges and opacity
			Overlay *o = _volviz->GetOverlay(_selected).get(); //the_global_frame->Interactor()->VolumeViz()->GetOverlay(_selected).get();

			float left,right,min,max;
			o->Range(left,right,min,max);
			_rangeSlider->SetMin(min);
			_rangeSlider->SetMax(max);
			_rangeSlider->SetLeft(left);
			_rangeSlider->SetRight(right);

			m_slider1->SetValue(o->Opacity()*100);
			
			NotifyAllListeners(PEvent(new EventUpdateMesh(1)));
			//the_global_frame->_qCtrlr->UpdateMesh();
			//the_global_frame->Refresh();
		}
	}
}
void OverlayPanel::OnSliderUpdate( wxCommandEvent& event )
{
	//Update Overlay opacity
	//PEvent evt(new EventOverlayOpacityChanged(m_slider1->GetValue()/100.0f));
	//the_global_frame->Interactor()->VolumeViz()->GetOverlay(_selected)->OnEvent(evt);
	//the_global_frame->_qCtrlr->UpdateMesh();
	//the_global_frame->Refresh();
	float opacity = m_slider1->GetValue()/100.0f;
	_volviz->GetOverlay(_selected)->SetOpacity( opacity );
	NotifyAllListeners(PEvent(new EventUpdateMesh(1)));

}
void OverlayPanel::Serialize(std::ostream &s, int version)
{
	s<<"--- Background and overlay panel ---"<<endl;
	int x,y,w,h;
	GetPosition(&x,&y);
	s<<"Position "<<x<<" "<<y<<endl;

	GetSize(&w,&h);
	s<<"Size "<<w<<" "<<h<<endl;
	s<<"Visible "<<IsVisible()<<endl;
	s<<endl;
}
void OverlayPanel::Deserialize(std::istream &s, int version)
{
	s>>"--- Background and overlay panel ---">>endl;
	int x,y,w,h;
	s>>"Position ">>x>>y>>endl;

	s>>"Size ">>w>>h>>endl;
	SetSize(x,y,w,h);
	s>>"Visible ">>x>>endl;
	Show(x?true:false);
	s>>endl;
}
void OverlayPanel::Clear()
{
	//Remove existing items
	VPOverlayItem::clear();
	bSizerOverlayItems->Clear(true);
	_selected=0;
}
void OverlayPanel::SetBackgroundVolume(int index)
{
	_selected = index;
	// deselect the previous selected background image
	for(int i=0; i < (int)VPOverlayItem::size(); i++)
	{
		VPOverlayItem::operator[](i)->SelectBackground(i == _selected);
		VPOverlayItem::operator[](i)->Select(i == _selected);
	}
}
