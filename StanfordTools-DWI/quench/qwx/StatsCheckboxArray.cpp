#include "StatsCheckboxArray.h"
#include "VisibilityPanel.h"
#include "PathwayGroup.h"
#include "PDBHelper.h"
//#include "PathwayGroup.h"
#include <wx/mstream.h>
#include "icons/trash_16x16.h"

static const int border = 5;
static const int BAR_WIDTH = 16;
static const int BAR_HEIGHT = 100;
static const int GRAPH_STARTX = 5;
static const int GRAPH_STARTY = 5;
static const int BAR_BORDER = 5;
static const int BORDER = 5;
wxBitmap trash_bmp;

StatsCheckboxArray::StatsCheckboxArray(wxWindow *parent) :wxPanel(parent, wxID_ANY, wxPoint(0,0), wxSize(1000, 150))
{
	trash_bmp = CreateBitmapFromMemory(ICON_TRASH_16x16, sizeof(ICON_TRASH_16x16), wxBITMAP_TYPE_PNG );
	_dc=0; _assn=0; _array = 0; 

	wxSizer *bSizer1 = new wxBoxSizer( wxVERTICAL );
	wxSizer *bSizer2 = new wxBoxSizer( wxVERTICAL );
	_assignmentLocked = new wxCheckBox( this, wxID_ANY, wxT("Assignment locked       "), wxDefaultPosition, wxSize(200,20), 0 );
	_assignmentLocked->SetFont(DEFAULT_FONT);
	bSizer1->Add( bSizer2, 1, wxEXPAND, 5 );
	this->SetSizer( bSizer1 );
	this->Layout();

	hoverGroup = -1;
	Connect(wxEVT_PAINT, wxPaintEventHandler(StatsCheckboxArray::OnPaint));
	
	// TONY: wxWidgits2.9.1 windows don't have OnSize anymore
	//Connect(wxEVT_SIZE, wxSizeEventHandler(StatsCheckboxArray::OnSize));
	
	Connect(wxEVT_LEFT_UP, wxMouseEventHandler(StatsCheckboxArray::OnLeftUp));
	Connect(wxEVT_MOTION, wxMouseEventHandler(StatsCheckboxArray::OnMouseMove));
	_assignmentLocked->SetValue(false);
	_assignmentLocked->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( StatsCheckboxArray::OnCheckBox ), NULL, this );
}
StatsCheckboxArray::~StatsCheckboxArray()
{
	delete _assignmentLocked ;
}

void StatsCheckboxArray::Update(DTIPathwayAssignment *assn, PathwayGroupArray *array) 
{ 
	_assn=assn; _array = array; 
	_assignmentLocked->SetValue(_assn->Locked());
	Refresh(); 
}

void StatsCheckboxArray::OnPaint(wxPaintEvent& event)  
{
	if(!_array || !_assn)return;

	int len  = (int)_array->size();
	// get the output group
	int outputGroup = _assn->SelectedGroup();

	// get the extents 
	int _xmax; GetExtents(0,0,_xmax,_ymax, (int)_array->size());
	wxPaintDC dc(this);
	wxFont font( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) ;
	wxFont groupFont = DEFAULT_FONT;//( 9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) ;
	int endx = GRAPH_STARTX+BAR_BORDER+ len*(BAR_WIDTH+BAR_BORDER)+20;
	int outy = BAR_HEIGHT*2/3;

	char str[1024];
	wxColour col;
	wxBrush brush;
	wxPen pen;
	wxColour bkcol = GetBackgroundColour();

	//Draw Axes in black
	col.Set(0,0,0); brush.SetColour(col); dc.SetBrush(brush); pen.SetColour(col); dc.SetPen(pen);
	dc.DrawLine(GRAPH_STARTX, GRAPH_STARTY+BAR_HEIGHT, endx-20, GRAPH_STARTY+BAR_HEIGHT);
	dc.DrawLine(GRAPH_STARTX, GRAPH_STARTY, GRAPH_STARTX, GRAPH_STARTY +BAR_HEIGHT);
	dc.SetFont(font);

	int total = (int)_assn->size();

	// for each group
	for(int i = 1; i < len; i++)
	{
		// height of this bar is proportional to the number of fibers in this group
		int startx = GRAPH_STARTX+BAR_BORDER+ (i-1)*(BAR_WIDTH+BAR_BORDER);
		int numPaths = _assn->NumAssigned(i);
		int ht =BAR_HEIGHT * numPaths / total;

		// is this the selected group
		if(i==outputGroup)
		{
			col.Set(0,0,0);	brush.SetColour(col); dc.SetBrush(brush); pen.SetColour(col); dc.SetPen(pen);
			
			// draw a selection rectangle
			dc.DrawRectangle(startx-1,GRAPH_STARTY+BAR_HEIGHT-ht-1,BAR_WIDTH+2, ht+2);
			dc.DrawRectangle(startx-1,GRAPH_STARTY+BAR_HEIGHT+BORDER+1,BAR_WIDTH+2, BAR_WIDTH+2);

			// show the information on the right hand side
			dc.DrawRectangle(endx, outy+BORDER+21,BAR_WIDTH+2, BAR_WIDTH+2);
			sprintf(str,"%d / %d (%.1f%%)", numPaths, total,100.0f * numPaths / total);
			dc.DrawText(str, endx+BAR_WIDTH+BORDER+5, outy +BORDER+27);
			dc.DrawText((*_array)[i].Name().c_str(), endx, outy+5);
			_assignmentLocked->SetPosition(wxPoint(endx,outy+BORDER+43));
		}

		//Draw grayed out
		Colord color = (*_array)[i].Color();
		col.Set(color.r*255, color.g*255, color.b*255);

		brush.SetColour(col); brush.SetStyle(wxSOLID);dc.SetBrush(brush); dc.SetPen(*wxBLACK_PEN); 

		// if assignments are locked or the group is not active, draw it crossed out
		if(!(*_array)[i].Active() || (_assignmentLocked->GetValue()&& i != outputGroup))
		{brush.SetStyle(wxCROSSDIAG_HATCH);}
		else
		{	col.Set(color.r*255, color.g*255, color.b*255); brush.SetStyle(wxSOLID);}

		brush.SetColour(col); dc.SetBrush(brush); pen.SetColour(bkcol); dc.SetPen(pen);

		// draw the fiber group icon and the bar showing the number of fibers
		dc.DrawRectangle(startx, GRAPH_STARTY+2+BAR_HEIGHT+BORDER, BAR_WIDTH, BAR_WIDTH);
		dc.DrawRectangle(startx, GRAPH_STARTY+BAR_HEIGHT-ht, BAR_WIDTH, ht);
		if(i == outputGroup)
			dc.DrawRectangle(endx+1, outy +BORDER+22,BAR_WIDTH, BAR_WIDTH);

		char snum[100]; 
		sprintf(snum,"%d",i);
		dc.SetFont(groupFont);
		dc.DrawText(snum,startx+4, GRAPH_STARTY+3+BAR_HEIGHT+BORDER);
		dc.SetFont(font);

		if(!(*_array)[i].Active() || (_assignmentLocked->GetValue()&& i != outputGroup))
		{
			col.Set(0,0,0); brush.SetColour(col); dc.SetBrush(brush); pen.SetColour(col); dc.SetPen(pen);
			//dc.DrawLine(startx, GRAPH_STARTY+BAR_HEIGHT, startx+BAR_WIDTH, GRAPH_STARTY+BAR_HEIGHT-ht);
		}
	}

	//draw the trash bin
	int startx = GRAPH_STARTX+len*(BAR_WIDTH+BAR_BORDER);
	Colord color = (*_array)[0].Color();
	col.Set(color.r*255, color.g*255, color.b*255);

	brush.SetColour(col); brush.SetStyle(wxSOLID);dc.SetBrush(brush); dc.SetPen(*wxBLACK_PEN); 
	if(!(*_array)[0].Active() || (_assignmentLocked->GetValue()))
	{
		brush.SetStyle(wxCROSSDIAG_HATCH);
		dc.DrawBitmap(trash_bmp, startx, GRAPH_STARTY+2+BAR_HEIGHT+BORDER,true);
		brush.SetColour(col); dc.SetBrush(brush); pen.SetColour(bkcol); dc.SetPen(pen);
		dc.DrawRectangle(startx, GRAPH_STARTY+2+BAR_HEIGHT+BORDER, BAR_WIDTH, BAR_WIDTH);
	}
	else
	{	
		col.Set(color.r*255, color.g*255, color.b*255); brush.SetStyle(wxSOLID);
		brush.SetColour(col); dc.SetBrush(brush); pen.SetColour(bkcol); dc.SetPen(pen);
		dc.DrawRectangle(startx, GRAPH_STARTY+2+BAR_HEIGHT+BORDER, BAR_WIDTH, BAR_WIDTH+1);
		dc.DrawBitmap(trash_bmp, startx, GRAPH_STARTY+2+BAR_HEIGHT+BORDER,true);
	}
	int ht =BAR_HEIGHT * _assn->NumAssigned(0) / (int)_assn->size();
	dc.DrawRectangle(startx, GRAPH_STARTY+BAR_HEIGHT-ht, BAR_WIDTH, ht);


	//Draw the axis
	pen.SetColour(bkcol);dc.SetPen(pen);
	for(int i = 0; i < 10; i++)
		dc.DrawLine(GRAPH_STARTX+1, GRAPH_STARTY+BAR_HEIGHT*i/10, endx-20, GRAPH_STARTY+BAR_HEIGHT*i/10);

	//Draw the hovered text
	if(hoverGroup != -1)
	{
		int numPaths = _assn->NumAssigned(hoverGroup);
		Colord color = (*_array)[hoverGroup].Color();
		col.Set(color.r*255, color.g*255, color.b*255);

		brush.SetStyle(wxSOLID);brush.SetColour(col); dc.SetBrush(brush); dc.SetPen(*wxBLACK_PEN);
		dc.DrawRectangle(endx, outy+BORDER+21-50,BAR_WIDTH+2, BAR_WIDTH+2);
		sprintf(str,"%d / %d (%.1f%%)", numPaths, total, 100.0f*numPaths/total);
		dc.DrawText(str, endx+BAR_WIDTH+BORDER+5, outy-50 +BORDER+27);
		dc.DrawText((*_array)[hoverGroup].Name().c_str(), endx, outy+5-50);
	}
}
int  StatsCheckboxArray::GetSwatchUnderCursor(int x, int y)
{
	if(!_array || !_assn)return -1;
	int n=-1;
	x-=GRAPH_STARTX; x/= (BAR_WIDTH+BAR_BORDER); y-=GRAPH_STARTY;

	// did we click on a bar
	if(y < BAR_HEIGHT+BAR_WIDTH+BORDER+2)
		if(x==_array->size()) // did we click on the trash group?
			n=0;
		else if(x < (int)_array->size()-1) // any other group?
			n=x+1;

	return n;
}
void StatsCheckboxArray::OnLeftUp(wxMouseEvent &event)
{
	if(!_array || !_assn)return;

	// if assignments are locked then none of the fiber groups state can change
	if(_assignmentLocked->GetValue())
		return;

	// get the swatch under the cursor
	int idx = GetSwatchUnderCursor(event.GetX(),event.GetY());

	// is it a valid swatch ? is it the selected group?
	if(idx != -1 && idx != _assn->SelectedGroup())
	{
		bool active = !(*_array)[idx].Active();
		(*_array)[idx].SetActive( active );
		NotifyAllListeners(PEvent( new Event(FILTER_PATHWAYS_BY_STATS) ));
	}
}
void StatsCheckboxArray::OnMouseMove(wxMouseEvent &event)
{
	if(!_array || !_assn)return;

	// we should refresh the screen only if the hover group has changed as refreshing causes flicker
	int oldGroup = hoverGroup;
	hoverGroup=GetSwatchUnderCursor(event.GetX(),event.GetY());
	if(hoverGroup != oldGroup)
		Refresh();
}
void StatsCheckboxArray::OnCheckBox( wxCommandEvent& event )
{
	// assignments were toggled, notify quench
	NotifyAllListeners(PEvent( new Event(TOGGLE_ASSIGNMENTS_LOCKED) ));
	Refresh();
}
