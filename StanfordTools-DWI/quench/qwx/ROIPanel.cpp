#include "vtkFilledCaptionActor2D.h"
#include "ROIPanel.h"
#include "vtkROI.h"
//#include "Quench.h"
#include "wx/colordlg.h"
#include <wx/mstream.h>
#include "icons/mesh.h"
#include "icons/sphere.h"
#include "icons/cube.h"
#include "icons/delete.h"

#define LAZY_SET(CTRL, VAL) if(CTRL->GetValue()!=VAL)CTRL->SetValue(VAL);
#define LAZY_SPIN_SET(CTRL, VAL) if(CTRL->GetValue()!=ToString(VAL,2)){CTRL->SetValue(ToString(VAL,2));programatically++;}

MultiValueSpinner::MultiValueSpinner( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );
	
	m_textCtrl2 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, DEFAULT_TEXTBOX_SIZE, wxTE_PROCESS_ENTER );
	m_textCtrl2->SetFont( DEFAULT_FONT );
	bSizer6->Add( m_textCtrl2, 1, wxALL, 0 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("  "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->SetFont( DEFAULT_FONT );
	m_staticText4->Wrap( -1 );
	m_staticText4->Enable( false );
	
	bSizer6->Add( m_staticText4, 0, wxALL, 0 );
	
	m_spinBtn2 = new wxSpinButton( this, wxID_ANY, wxDefaultPosition, wxSize( 16,-1 ), 0 );
	m_spinBtn2->SetMaxSize( wxSize( 16,-1 ) );
	m_spinBtn2->SetRange(-0x8000, 0x7fff);
	bSizer6->Add( m_spinBtn2, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 0 );
	
	this->SetSizer( bSizer6 );
	this->Layout();
	
	// Connect Events
	m_textCtrl2->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MultiValueSpinner::OnTextEnter ), NULL, this );
	m_spinBtn2->Connect( wxEVT_SCROLL_LINEDOWN, wxSpinEventHandler( MultiValueSpinner::OnSpinDown ), NULL, this );
	m_spinBtn2->Connect( wxEVT_SCROLL_LINEUP, wxSpinEventHandler( MultiValueSpinner::OnSpinUp ), NULL, this );
	_step = 1;
	_index = 0;
	SetValue(_value);
}

MultiValueSpinner::~MultiValueSpinner()
{
	// Disconnect Events
	m_textCtrl2->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MultiValueSpinner::OnTextEnter ), NULL, this );
	m_spinBtn2->Disconnect( wxEVT_SCROLL_LINEDOWN, wxSpinEventHandler( MultiValueSpinner::OnSpinDown ), NULL, this );
	m_spinBtn2->Disconnect( wxEVT_SCROLL_LINEUP, wxSpinEventHandler( MultiValueSpinner::OnSpinUp ), NULL, this );
}
bool Equal(Vector3d &a, Vector3d &b)
{
	for(int i = 0; i < 3; i++)
		if(fabsf(a[i]-b[i]) > 0.0001f)return false;
	return true;
}
void MultiValueSpinner::OnTextEnter( wxCommandEvent& event )
{
	double v[3];
	// check if exactly three values have been entered separated by exactly 2 commas
	if(sscanf(m_textCtrl2->GetValue().c_str(), "%lf, %lf, %lf",&v[0],&v[1],&v[2]) == 3 && m_textCtrl2->GetValue().Freq(',')==2)
	{
		m_textCtrl2->SetValue("0,0,0");
		Vector3d temp_val(v);
		SetValue(temp_val);
	}
}
void MultiValueSpinner::OnSpin(double amt) 
{ 
	int len = m_textCtrl2->GetInsertionPoint()-1;
	// get the index of the value that needs to be updated
	_index = m_textCtrl2->GetValue().SubString(0,len).Freq(',');

	// increment the value by 'amt'
	Vector3d v=_value; 
	v[_index]+=amt; 
	// if we are still in bounds update the text control
	if(v[_index] > _min[_index] && v[_index] < _max[_index])
		SetValue(v); 
}
void MultiValueSpinner::SetMin(Vector3d &v)
{
	// if min has changed then return
	if(Equal(_min , v)) return;
	_min = v;
	bool bValueChanged = false;

	// clamp the values to min if need be
	for(int i = 0; i < 3; i++)
		if(_value[i] < _min[i])
		{
			_value[i] = _min[i];
			bValueChanged = true;
		}
	
	if(bValueChanged)
	SetValue(_value);
}
void MultiValueSpinner::SetMax(Vector3d &v)
{
	// if max hasnt changed do nothing
	if(Equal(_max , v)) return;
	_max = v;
	bool bValueChanged = false;

	// clamp the values to max if need be
	for(int i = 0; i < 3; i++)
		if(_value[i] > _max[i])
		{
			_value[i] = _max[i];
			bValueChanged = true;
		}
	
	if(bValueChanged)
	SetValue(_value);
}
void MultiValueSpinner::SetValue(Vector3d &v)
{
	if(Equal(_value , v)) return;
	_value = v;
	for(int i = 0; i < 3; i++)
	{
	  //	if(_value[i] > _max[i]) _value[i] = _max[i];
	  //	if(_value[i] < _min[i]) _value[i] = _min[i];
	  // xxx dla temporarily removed, since it was causing problems.
	}
	char text[100];
	sprintf(text, "%.1lf, %.1lf, %.1lf", _value[0], _value[1], _value[2]); 
	int pos = m_textCtrl2->GetInsertionPoint();
	if(wxString(text) !=m_textCtrl2->GetValue())
	{
	m_textCtrl2->SetValue(text);
	m_textCtrl2->SetInsertionPoint(pos);
		NotifyAllListeners(PEvent(new EventMultiValueSpinnerChanged(this)));
	}
}

// XXX WHAT THE HELL IS THIS GLOBAL DOING??
int programatically=-1;


ROIPanel::ROIPanel(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	m_panel2 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bROIPanelSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bQuerySizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText12 = new wxStaticText( m_panel2, wxID_ANY, wxT("QueryOp"), wxPoint( -1,-1 ), wxSize( -1,20 ), 0 );
	m_staticText12->SetFont( DEFAULT_FONT );
	m_staticText12->Wrap( -1 );
	bQuerySizer->Add( m_staticText12, 0, wxALL, 5 );
	
	wxString _queryOpsChoices[] = { wxT("AND"), wxT("OR"), wxT("IGNORE") };
	int _queryOpsNChoices = sizeof( _queryOpsChoices ) / sizeof( wxString );
	_queryOps = new wxChoice( m_panel2, wxID_ANY, wxPoint( -1,-1 ), wxSize( -1,20 ), _queryOpsNChoices, _queryOpsChoices, 0 );
	_queryOps->SetSelection( 0 );
	bQuerySizer->Add( _queryOps, 0, wxALL, 5 );
	
	_Query = new wxTextCtrl( m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, DEFAULT_TEXTBOX_SIZE, wxTE_PROCESS_ENTER );
	_Query->SetFont( DEFAULT_FONT );
	bQuerySizer->Add( _Query, 1, wxALL, 5 );

	
	_showAllChecked = new wxCheckBox( m_panel2, wxID_ANY, wxT("Show All"), wxDefaultPosition, wxSize( -1,23 ));
	_showAllChecked->SetFont( DEFAULT_FONT );
	_showAllChecked->SetValue(true);
	_showLabelsChecked = new wxCheckBox( m_panel2, wxID_ANY, wxT("Show Labels"), wxDefaultPosition, wxSize( -1,23 ));
	_showLabelsChecked->Enable(false);  // dakers disabled this control 8/3/11 - global control only of ROI visibility now.
	_showLabelsChecked->SetValue(true);
	_showLabelsChecked->SetFont( DEFAULT_FONT );
	
	bQuerySizer->Add( _showAllChecked, 0, wxALL, 5 );
	bQuerySizer->Add( _showLabelsChecked, 0, wxALL, 5 );
	
	bROIPanelSizer->Add( bQuerySizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bCommonROISizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString _voiListChoices;
	_voiList = new wxChoice( m_panel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, _voiListChoices, 0 );
	_voiList->SetSelection( 0 );
	_voiList->SetFont( DEFAULT_FONT );
	bCommonROISizer->Add( _voiList, 0, wxALL, 15 );
	
	wxBitmap cube = CreateBitmapFromMemory(ICON_CUBE, sizeof(ICON_CUBE), wxBITMAP_TYPE_PNG );
	_buttonCreateCube = new wxBitmapButton(m_panel2, wxID_ANY, cube );
	bCommonROISizer->Add( _buttonCreateCube, 0, wxALL, 5 );
	
	wxBitmap sphere = CreateBitmapFromMemory(ICON_SPHERE, sizeof(ICON_SPHERE), wxBITMAP_TYPE_PNG );
	_buttonCreateSphere = new wxBitmapButton( m_panel2, wxID_ANY, sphere);
	bCommonROISizer->Add( _buttonCreateSphere, 0, wxALL, 5 );
	
	wxBitmap mesh = CreateBitmapFromMemory(ICON_MESH, sizeof(ICON_MESH), wxBITMAP_TYPE_PNG );
	_buttonCreateMesh= new wxBitmapButton( m_panel2, wxID_ANY, mesh);
	bCommonROISizer->Add( _buttonCreateMesh, 0, wxALL, 5 );

	wxBitmap delete1 = CreateBitmapFromMemory(ICON_DELETE, sizeof(ICON_DELETE), wxBITMAP_TYPE_PNG );
	_buttonDelete = new wxBitmapButton( m_panel2, wxID_ANY, delete1);
	bCommonROISizer->Add( _buttonDelete, 0, wxALL, 5 );
	
	bROIPanelSizer->Add( bCommonROISizer, 1, wxSHAPED, 5 );
	
	m_staticline1 = new wxStaticLine( m_panel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	//bROIPanelSizer->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bROISizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bNameSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxPanel *pROI = new wxPanel(m_panel2,wxID_ANY,wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER);
	pROI->SetSizer(bROISizer);

	m_staticText2 = new wxStaticText( pROI, wxID_ANY, wxT("Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->SetFont( DEFAULT_FONT );
	m_staticText2->Wrap( -1 );
	bNameSizer->Add( m_staticText2, 0, wxALL, 9 );
	
	_nameCtrl = new wxTextCtrl( pROI, wxID_ANY, wxEmptyString, wxPoint( -1,-1 ), DEFAULT_TEXTBOX_SIZE, wxTE_PROCESS_ENTER );
	_nameCtrl->SetFont( DEFAULT_FONT );
	bNameSizer->Add( _nameCtrl, 1, wxALL, 5 );
	
	//_checkboxVisible = new wxCheckBox( pROI, wxID_ANY, wxT("Visible"), wxPoint( -1,-1 ), wxSize( -1,23 ), 0 );
	//bNameSizer->Add( _checkboxVisible, 0, wxALL, 5 );
	
	//bNameSizer->Add( _colorButton, 0, wxALL, 5 );
	
	bROISizer->Add( bNameSizer, 1, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	//m_color = new wxStaticText( m_panel2, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(16,16), 0 );
	//m_color->SetBackgroundColour(wxColour(DEFAULT_ROI_COLOR.r*255, DEFAULT_ROI_COLOR.g*255, DEFAULT_ROI_COLOR.b*255));
	_colorButton = new wxButton( pROI, wxID_ANY, wxT(""), wxPoint( -1,-1 ), wxSize( 24,24 ), 0 );
	_colorButton->SetBackgroundColour(*wxRED);
	bSizer3->Add( _colorButton, 0, wxALL, 5 );
	m_staticText3 = new wxStaticText( pROI, wxID_ANY, wxT("   Opacity"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->SetFont( DEFAULT_FONT );
	bSizer3->Add( m_staticText3, 0, wxALL, 9 );
	m_opacityCtrl = new wxSpinCtrl( pROI, wxID_ANY, wxT("50"), wxDefaultPosition, wxSize(50,-1));
	bSizer3->Add( m_opacityCtrl, 0, wxALL, 5 );
	m_opacityCtrl->SetValue(DEFAULT_ROI_COLOR.r*100);
	_checkboxVisible = new wxCheckBox( pROI, wxID_ANY, wxT("Visible"), wxPoint( -1,-1 ), wxSize( -1,23 ), 0 );
	bSizer3->Add( _checkboxVisible, 0, wxALL, 5 );
	bROISizer->Add( bSizer3, 1, wxEXPAND, 0 );
	
	wxBoxSizer* bPosSizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText3 = new wxStaticText( pROI, wxID_ANY, wxT("Position"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->SetFont( DEFAULT_FONT );
	m_staticText3->Wrap( -1 );
	bPosSizer->Add( m_staticText3, 0, wxALL, 9 );
	
	_posCtrl = new MultiValueSpinner(pROI);
	bPosSizer->Add( _posCtrl , 1, wxALL, 5 );

	m_staticText8 = new wxStaticText( pROI, wxID_ANY, wxT("   Scale"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->SetFont( DEFAULT_FONT );
	m_staticText8->Wrap( -1 );
	bPosSizer->Add( m_staticText8, 0, wxALL, 9 );
	
	_scaleCtrl = new MultiValueSpinner(pROI);
	bPosSizer->Add( _scaleCtrl, 1, wxALL, 5 );
	
	bROISizer->Add( bPosSizer, 1, wxEXPAND, 5 );
	
	pROI->Layout();
	bROISizer->Fit( pROI );

	bROIPanelSizer->Add( pROI, 0, wxEXPAND, 5 );
	
	m_panel2->SetSizer( bROIPanelSizer );
	m_panel2->Layout();
	bROIPanelSizer->Fit( m_panel2 );
	bSizer2->Add( m_panel2, 0, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( bSizer2 );
	this->Layout();
	
	// Connect Events
	_Query->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( ROIPanel::OnQueryChanged ), NULL, this );
	_voiList->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ROIPanel::OnSelectionChanged ), NULL, this );
	_buttonCreateCube->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ROIPanel::OnCreateCube ), NULL, this );
	_buttonCreateSphere->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ROIPanel::OnCreateSphere ), NULL, this );
	_buttonCreateMesh->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ROIPanel::OnCreateMesh ), NULL, this );
	_buttonDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ROIPanel::OnDelete ), NULL, this );
	_nameCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( ROIPanel::OnNameChanged ), NULL, this );
	_colorButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ROIPanel::OnColorChanged ), NULL, this );
	m_opacityCtrl->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( ROIPanel::OnSpinCtrl ), NULL, this );
	m_opacityCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( ROIPanel::OnSpinCtrlText ), NULL, this );
	_queryOps->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ROIPanel::OnQueryOpChanged ), NULL, this );
	_checkboxVisible->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ROIPanel::OnROIVisible ), NULL, this );
	_showAllChecked->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ROIPanel::OnShowAll ), NULL, this );
	_showLabelsChecked->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ROIPanel::OnShowLabels ), NULL, this );
	SetEnabled(false);

	Vector3d vmin(0.1,0.1,0.1); _scaleCtrl->SetMin(vmin);
	Vector3d vmax(10,10,10); _scaleCtrl->SetMax(vmax);
	Vector3d v(1,1,1); _scaleCtrl->SetValue(v);
	_posCtrl->push_back(this);
	_scaleCtrl->push_back(this);
}

ROIPanel::~ROIPanel()
{
	// Disconnect Events
	_Query->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( ROIPanel::OnQueryChanged ), NULL, this );
	_voiList->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ROIPanel::OnSelectionChanged ), NULL, this );
	_buttonCreateCube->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ROIPanel::OnCreateCube ), NULL, this );
	_buttonCreateSphere->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ROIPanel::OnCreateSphere ), NULL, this );
	_buttonDelete->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ROIPanel::OnDelete ), NULL, this );
	_colorButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ROIPanel::OnColorChanged ), NULL, this );
	_queryOps->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ROIPanel::OnQueryOpChanged ), NULL, this );
	_checkboxVisible->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ROIPanel::OnROIVisible ), NULL, this );
	_nameCtrl->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( ROIPanel::OnNameChanged ), NULL, this );
	
	delete _queryOps;
	delete m_opacityCtrl;
}
void ROIPanel::SetROIBounds(Vector3d min, Vector3d max) 
{ 
	_min = min; _max = max; 
	_posCtrl->SetMin(_min);
	_posCtrl->SetMax(_max);
}

void ROIPanel::OnQueryChanged( wxCommandEvent& event ){ event.Skip(); }
void ROIPanel::OnQueryOpChanged( wxCommandEvent& event )
{
	// query operation can be and or or
	const char *op = _queryOps->GetSelection()==0 ? " AND ": " OR ";
	// notify quench that the query op has changed
	NotifyAllListeners(PEvent(new EventROIQueryOpChanged(_queryOps->GetSelection())));
	
	// build the query string using the voi names
	if(_queryOps->GetSelection()!=2)
	{
		std::string str = "1";
		for(int i = 1; i < (int)_voiList->GetCount(); i++)
		{
			char s[100]; sprintf(s,"%d",i+1);
			str = str + op + s;
		}
		_Query->SetValue(str.c_str());
	}
	_Query->Enable(_queryOps->GetSelection()==2);
//	_listener->SetQueryOp((DTIQueryOperation)_queryOps->GetSelection(), std::string(_Query->GetValue()));
}
void ROIPanel::OnColorChanged( wxCommandEvent& event )
{ 
	wxPoint pt=wxGetMousePosition();
	pt.y-=400;

	// show a color choosing dialog
	wxColourDialog colordlg(this,0);
	wxColourData &colorData = colordlg.GetColourData();
	/*colorData.SetCustomColour(15, wxColour((unsigned char) floor(PATHWAY_COLORS[0].r*255),
					      (unsigned char) floor(PATHWAY_COLORS[0].g*255),
					      (unsigned char) floor(PATHWAY_COLORS[0].b*255)));	*/			      
	for (int i = 1; i < 16; i++) {
	  colorData.SetCustomColour(i-1, wxColour((unsigned char) floor(PATHWAY_COLORS[i].r*255),
						(unsigned char) floor(PATHWAY_COLORS[i].g*255),
						(unsigned char) floor(PATHWAY_COLORS[i].b*255)));
	}
	colordlg.SetPosition(pt);

	if(colordlg.ShowModal()==wxID_OK)
	{
		wxColour color2 = colordlg.GetColourData().GetColour();
		// show the new color on screen
		_colorButton->SetBackgroundColour(color2);
		_colorButton->Refresh();
		Colord col3(color2.Red()/255.0, color2.Green()/255.0, color2.Blue()/255.0, m_opacityCtrl->GetValue()/100.0);
		
		// notify quench of the change in color
		NotifyAllListeners(PEvent(new EventROIColor(col3)));
	}
}
void ROIPanel::OnROIVisible( wxCommandEvent& event )
{ 
	NotifyAllListeners(PEvent(new EventROIShow(_checkboxVisible->GetValue())));
}
void ROIPanel::OnShowAll( wxCommandEvent& event )
{
	NotifyAllListeners(PEvent(new EventROIShowAll(_showAllChecked->GetValue())));
}
void ROIPanel::OnShowLabels( wxCommandEvent& event )
{
	NotifyAllListeners(PEvent(new EventROIShowLabel(_showLabelsChecked->GetValue())));
}

// TONY: wxWidgets2.9.1 changed the way wxString works
//void ROIPanel::OnSelectionChanged	( wxCommandEvent& event ) { NotifyAllListeners(PEvent(new EventROISelectByName(_voiList->GetStringSelection().c_str()))); }
//void ROIPanel::OnNameChanged		( wxCommandEvent& event ) { NotifyAllListeners( PEvent( new EventROINameChanged( _nameCtrl->GetValue().c_str()))); }
void ROIPanel::OnSelectionChanged	( wxCommandEvent& event ) { NotifyAllListeners(PEvent(new EventROISelectByName(_voiList->GetStringSelection().mb_str()))); }
void ROIPanel::OnNameChanged		( wxCommandEvent& event ) { NotifyAllListeners( PEvent( new EventROINameChanged( _nameCtrl->GetValue().mb_str()))); }
void ROIPanel::OnCreateCube			( wxCommandEvent& event ) { NotifyAllListeners(PEvent(new Event(ROI_CREATE_CUBE)));		}
void ROIPanel::OnCreateSphere		( wxCommandEvent& event ) { NotifyAllListeners(PEvent(new Event(ROI_CREATE_SPHERE)));	}
void ROIPanel::OnCreateMesh			( wxCommandEvent& event ) { NotifyAllListeners(PEvent(new Event(ROI_CREATE_MESH)));		}
void ROIPanel::OnDelete				( wxCommandEvent& event ) { NotifyAllListeners(PEvent(new Event(ROI_DELETE)));			}

void ROIPanel::OnSpinCtrl( wxSpinEvent& event )
{
	// change the opacity of the voi
	wxColour color2 = _colorButton->GetBackgroundColour();
	Colord col3(color2.Red()/255.0, color2.Green()/255.0, color2.Blue()/255.0, m_opacityCtrl->GetValue()/100.0);

	// notify quench of the change in the voi color
	NotifyAllListeners(PEvent(new EventROIColor(col3)));
}
void ROIPanel::OnSpinCtrlText( wxCommandEvent& event )
{
	wxColour color2 = _colorButton->GetBackgroundColour();
	Colord col3(color2.Red()/255.0, color2.Green()/255.0, color2.Blue()/255.0, m_opacityCtrl->GetValue()/100.0);

	// notify quench of the change in the voi color
	NotifyAllListeners(PEvent(new EventROIColor(col3)));
}
		
std::string ToString(float value, int precision)
{
	char format[]="%.0f";
	format[2]+=precision;
	char out[100];
	sprintf(out,format,value);
	return std::string(out);
}
void ROIPanel::Update(ROIManager &mgr)
{
	SetEnabled(mgr.size()?true:false);
	PDTIFilterROI selected = mgr.Selected();

	// if no voi are selected or no voi's exist, return
	if(!selected || mgr.size() == 0)
		return;
	int i = 0; 

	// if voi list has changed repopulate it
	if(_voiList->GetCount() != mgr.size())
	{
		_voiList->Clear();
		// for each voi
		for(PDTIFilterROIList::iterator iter = mgr.begin(); iter != mgr.end(); iter++, i++)
		{
			// append the name of the voi to the list
			_voiList->Append((*iter)->Name().c_str());
			if((*iter) == selected)
				_voiList->Select(i);
		}
	}
	// otherwise if the selected voi has changed , select the voi
	else if(_voiList->GetStringSelection() != selected->Name().c_str())
	{
		//Some other voi is selected so update the list selection
		for(PDTIFilterROIList::iterator iter = mgr.begin(); iter != mgr.end(); iter++, i++)
			if((*iter) == selected)
				_voiList->Select(i);
	}

	LAZY_SET(_nameCtrl, selected->Name().c_str());
	
	double *pos = selected->Position();
	double *scale = selected->Scale();

	// set the position and scale for the voi
	Vector3d v(pos);
	_posCtrl->SetValue(v);
	v = Vector3d(scale);
	_scaleCtrl->SetValue(v);

	//Set the extents
	Vector3d actualSize = selected->Size()*Vector3d(selected->Scale())/2;

	// dla fixed below to take into account that the center of a ROI may
	// not be at (0,0,0):
	Vector3d min_range = _min + actualSize + selected->Center();
	Vector3d max_range = _max - actualSize + selected->Center();
	_posCtrl->SetMin( min_range );
	_posCtrl->SetMax( max_range );

	// dla fixed below to take into account that the center of a ROI may
	// not be at (0,0,0):
	Vector3d bound = min( Vector3d(selected->Position())-_min-selected->Center(), _max-Vector3d(selected->Position())+selected->Center() );
	bound = bound / selected->Size() * 2;

	_scaleCtrl->SetMax(bound);

	//	cerr << endl;
	//cerr << "actualSize: " << actualSize[0] << ", " << actualSize[1] << ", " << actualSize[2] << endl;
	//cerr << "min_range: " << min_range[0] << ", " << min_range[1] << ", " << min_range[2] << endl;
	//cerr << "Position: " << selected->Position()[0] << ", " << selected->Position()[1] << ", " << selected->Position()[2] << endl;
	//cerr << "min: " << _min[0] << ", " << _min[1] << ", " << _min[2] << endl;
	//cerr << "max: " << _max[0] << ", " << _max[1] << ", " << _max[2] << endl;
	//cerr << "Selected size: " << selected->Size()[0] << ", " << selected->Size()[1] << ", " << selected->Size()[2] << endl;

	//cerr << "bound: " << bound[0] << ", " << bound[1] << ", " << bound[2] << endl;
	//Set Color
	Colord col = ((vtkROI*)selected.get())->Color();
	_colorButton->SetBackgroundColour(wxColour(col.r*255, col.g*255, col.b*255));
	m_opacityCtrl->SetValue(col.r*100);
	LAZY_SET(_checkboxVisible, ((vtkROI*)selected.get())->MeshVisible());
}
void ROIPanel::OnEvent(PEvent evt)
{
	if(evt->EventType() == MULTI_VALUE_SPINNER_CHANGED)
	{
		EventMultiValueSpinnerChanged* e = dynamic_cast<EventMultiValueSpinnerChanged*>(evt.get());
		if(e->Value() == _posCtrl) // was the position changed?
			NotifyAllListeners(PEvent(new EventROITranslate(_posCtrl->Value())));
		else // must be the scale then
			NotifyAllListeners(PEvent(new EventROIScale(_scaleCtrl->Value())));
	}
}
void ROIPanel::SetEnabled(bool enabled)
{
	// toggle the visibility based on the enabled flag
	_Query->Enable(enabled && _queryOps->GetSelection()==2);
	_voiList->Enable(enabled);
	_buttonDelete->Enable(enabled);
	_nameCtrl->Enable(enabled);
	_queryOps->Enable(enabled);
	_checkboxVisible->Enable(FALSE);  // dakers disabled this control 8/3/11 - global control only of ROI visibility now.
	_showAllChecked->Enable(enabled);
	_posCtrl->Enable(enabled);
	_scaleCtrl->Enable(enabled);
	m_opacityCtrl->Enable(enabled);
	_colorButton->Enable(enabled);
}
