#include "StatsPanel.h"
#include "RangeSlider.h"
#include "StatsCheckboxArray.h"
#include "VisibilityPanel.h"
#include "DTIPathwayStatisticHeader.h"
#include <string>
#include <wx/statline.h>

//#include "Quench.h"
#include "DTIPathwayDatabase.h"

BEGIN_EVENT_TABLE(StatsPanel, wxDialog)
  EVT_BUTTON(RESET_BUTTON_ID , StatsPanel::OnReset)
  EVT_BUTTON(APPLY_BUTTON_ID , StatsPanel::OnApply)
  EVT_BUTTON(CANCEL_BUTTON_ID, StatsPanel::OnCancel)
  EVT_CLOSE(StatsPanel::OnClose)
END_EVENT_TABLE()
using namespace std;

StatsPanel::StatsPanel( wxWindow* parent, const wxString& title ) 
  : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(450,280), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* inputSizer;
	inputSizer = new wxBoxSizer( wxVERTICAL );
	
	inputSizer->SetMinSize( wxSize( -1,150 ) ); 
	statsCheckboxArray = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	inputSizer->Add( statsCheckboxArray, 1, wxEXPAND | wxALL, 5 );
	
	bSizer1->Add( inputSizer, 0, wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("Filters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	m_staticText2->SetFont( wxFont( 12, 74, 90, 92, false, wxT("Arial") ) );
	
	bSizer1->Add( m_staticText2, 0, wxALL, 5 );
	
	_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	bSizer1->Add( _notebook, 1, wxEXPAND | wxALL, 5 );
	
	buttonSizer = new wxBoxSizer( wxHORIZONTAL );
	
	this->SetSizer( bSizer1 );


	_cbArray = new StatsCheckboxArray(statsCheckboxArray);
	
	_statsSizer = new wxBoxSizer( wxVERTICAL );
	_dummyStatsPanel = new wxPanel(_notebook); 

	m_button1 = new wxButton( _dummyStatsPanel, RESET_BUTTON_ID, wxT("Reset"), wxDefaultPosition, DEFAULT_BUTTON_SIZE, 0 );
	m_button1->SetFont( DEFAULT_FONT );
	
	_statsSizer->Add( m_button1, 0, wxALL, 5 );
	wxStaticLine *m_staticline1 = new wxStaticLine( _dummyStatsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	_statsSizer->Add( m_staticline1, 0, wxEXPAND | wxALL, 0 );
	
	_dummyStatsPanel->SetSizer(_statsSizer);
	_notebook->AddPage(_dummyStatsPanel, "Stats", true);
	
	_voiPanel = new ROIPanel(_notebook);
	//	_notebook->AddPage(_voiPanel, "ROI", false);
	// dakers 8/4/11 - disabled ROI Panel for now.
	
	_notebook->SetFont(DEFAULT_FONT);
	
	bTempWasShown = false;
	lastPositionX=lastPositionY=-1;
	_cbArray->push_back(this);
	this->Layout();
}
StatsPanel::~StatsPanel()
{
	//SAFE_DELETE(_voiPanel);
	SAFE_DELETE(_cbArray);
	SAFE_DELETE(m_staticText2);
	SAFE_DELETE(statsCheckboxArray);
	SAFE_DELETE(buttonSizer);
	//SAFE_DELETE(_dummyStatsPanel);
	for(int i = 0; i < (int)_vRS.size(); i++)
		delete _vRS[i];
	_vRS.clear();
	SAFE_DELETE(_notebook);
}
void StatsPanel::Update(PDBHelper &helper, PathwayGroupArray &array)
{
	_cbArray->Update(&helper.Assignment(), &array);

	// add range sliders which are missing
	for(int i = (int)_vRS.size(); i < helper.PDB()->getNumPathStatistics(); i++)
	{
		RangeSlider *rs = new RangeSlider(_dummyStatsPanel, i, helper.PDB()->getPathStatisticHeader(i)->_aggregate_name, helper.PDB()->getMinValue(i), helper.PDB()->getMaxValue(i));
		rs->push_back(this);
		_vRS.push_back(rs);
		// add the range slider
		_statsSizer->Add(rs,wxSizerFlags(5).Expand().Border());
		
		// resize the panel to make space for the new range slider
		wxSize size;
		if(_cbArray)size = _cbArray->GetSize();
		SetSize(GetSize().x,rs->GetSize().y+GetSize().y);
	}

	//Update the stats for the remaining sliders
	for(unsigned int i = 0; i < _vRS.size(); i++)
	{
		RangeSlider *rs = _vRS[i];
		string name (helper.PDB()->getPathStatisticHeader(i)->_aggregate_name);
		rs->SetName(name); // Name shouldn't have changed
		rs->SetMin(helper.PDB()->getMinValue(i));
		rs->SetMax(helper.PDB()->getMaxValue(i));
		rs->SetLeft(helper.PDB()->getPathStatistic(i)->_left);
		rs->SetRight(helper.PDB()->getPathStatistic(i)->_right);
	}
}
void StatsPanel::OnClose(wxCloseEvent  &e)
{
	wxCommandEvent ce;
	OnApply(ce);
}
void StatsPanel::OnReset(wxCommandEvent &event)
{
	// reset all the stats. 
	// the range slider automatically calls stats panel to trigger the filtering
	for(unsigned int i = 0; i < _vRS.size(); i++)
		_vRS[i]->Reset( i== _vRS.size()-1 ); // update only once when the very last slider has been reset
}
void StatsPanel::OnApply(wxCommandEvent &event)
{
	Show(false);
	NotifyAllListeners(PEvent(new Event(ACCEPT_FILTER_PATHWAYS_BY_STATS)));
}
void StatsPanel::OnCancel(wxCommandEvent &event)
{
//	if(_listener)_listener->StatsClosed(false);
}
void StatsPanel::ShowPage(int i)
{
	if( i == SHOW_ROI_PANEL)
		_notebook->SetSelection(1);
	if( i == SHOW_REFINE_SELECTION_PANEL)
		_notebook->SetSelection(0);
}
void StatsPanel::TempShow(bool show)
{
	if(show)
	{
		if(bTempWasShown)
		{
			Show();
			//Stupid wxWidget wont show the window on the same position.
			if(lastPositionX!=-1)			
				SetSize(lastPositionX-1 ,lastPositionY ,wxDefaultCoord ,wxDefaultCoord);
			bTempWasShown = false;
		}
	}
	else
	{
		bTempWasShown = IsShown();
		GetPosition(&lastPositionX,&lastPositionY);
		Show(false);
	}
}
void StatsPanel::Serialize(ostream &s, int version)
{
	s<<"--- Refine Selection panel ---"<<endl;
	int x,y,w,h;
	GetPosition(&x,&y);
	s<<"Position "<<x<<" "<<y<<endl;

	GetSize(&w,&h);
	s<<"Size "<<w<<" "<<h<<endl;
	s<<"Visible "<<IsVisible()<<endl;
	s<<"Num ROIs "<<0<<endl;
	s<<endl;
}
void StatsPanel::Deserialize(istream &s, int version)
{
	s>>"--- Refine Selection panel ---">>endl;
	int x,y,w,h;
	s>>"Position ">>x>>y>>endl;
	
	s>>"Size ">>w>>h>>endl;
	SetSize(x,y,w,h);
	s>>"Visible ">>x>>endl;
	Show(x?true:false);
	s>>"Num ROIs ">>x>>endl;
	s>>endl;
}
void StatsPanel::Clear()
{
	// change the size back to original state
	SetSize(GetSize().x,GetSize().y - _dummyStatsPanel->GetSize().y);
	
	// remove the sliders from the window
	_statsSizer->Clear(true);
	
	// remove the sliders
	_vRS.clear();
}
void StatsPanel::OnEvent(PEvent evt)
{
	switch(evt->EventType())
	{
	case RANGE_SLIDER_UPDATED:
	case FILTER_PATHWAYS_BY_STATS:
		// refresh the checkbox array
		_cbArray->Refresh();
		// notify quench that a filtering by statistics event has occurred
		NotifyAllListeners(PEvent( new Event(FILTER_PATHWAYS_BY_STATS) ));
		break;

	case TOGGLE_ASSIGNMENTS_LOCKED:
		// notify quench that assignments were unlocked, refiltering if the need be
		NotifyAllListeners(PEvent( new Event(TOGGLE_ASSIGNMENTS_LOCKED) ));
		break;
	}
}
