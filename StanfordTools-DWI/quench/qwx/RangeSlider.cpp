#include <wx/wx.h>
#include <wx/mstream.h>
#include "RangeSlider.h"
#include "icons/sr_png.cpp"
#include "icons/sl_png.cpp"
#include "icons/ss_png.cpp"
#include "icons/s_png.cpp"
#include "icons/trkbut2_png.cpp"
#include <sstream>
#include <wx/stattext.h>
#include <wx/statline.h>
using namespace std;

wxRect GetTopButDims(wxPoint &pt, wxSize &size2, int rbSize){return wxRect(pt.x-rbSize-5, pt.y, rbSize, rbSize);}
wxRect GetOffButDims(wxPoint &pt, wxSize &size2, int rbSize){return wxRect(pt.x+size2.x+20, pt.y+(size2.y-rbSize)/2, rbSize, rbSize);}
wxRect GetBotButDims(wxPoint &pt, wxSize &size2, int rbSize){return wxRect(pt.x-rbSize-5, pt.y+size2.y-rbSize,rbSize,rbSize);}
inline wxBitmap _wxGetBitmapFromMemory(const unsigned char *data, int length) {
	wxMemoryInputStream is(data, length);
	wxImage tmp_img = wxImage(is, wxBITMAP_TYPE_ANY, -1); 
	return wxBitmap(tmp_img, -1);
}
#define wxGetBitmapFromMemory(name) _wxGetBitmapFromMemory(name ## _png, sizeof(name ## _png))

int num[] = { 75, 150, 225, 300, 375, 450, 525, 600, 675 };
int asize = sizeof(num)/sizeof(num[1]);
float round(float x)  { return x<0 ? ceil((x)-0.5) : floor((x)+0.5); }

RangeSlider::RangeSlider(wxWindow *parent, int id, string name, float Min, float Max)
: wxPanel(parent, id, wxDefaultPosition, wxSize(-1, 85), 0)
{
	SetFont(DEFAULT_FONT);
	
	_name = name;
	_thumb = wxGetBitmapFromMemory(trkbut2); 
	_sldr_left = wxGetBitmapFromMemory(sl);
	_sldr_right = wxGetBitmapFromMemory(sr);
	
	wxBitmap tmp = wxGetBitmapFromMemory(s); 
	_slider = tmp.ConvertToImage();
	tmp = wxGetBitmapFromMemory(ss);
	_slider_select = tmp.ConvertToImage();

	wxBoxSizer* bSizer;
	bSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer *labelSizer = new wxBoxSizer( wxHORIZONTAL);
	
	_text_label = new wxStaticText( this, wxID_ANY, _name.c_str(), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	_text_label->Wrap( -1 );
	_text_label->SetFont(DEFAULT_FONT);
	labelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	labelSizer->Add( _text_label, 0, wxEXPAND, 5 );
	labelSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer->Add( labelSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerText;
	bSizerText = new wxBoxSizer( wxHORIZONTAL );

	_stext_left = new wxStaticText( this, wxID_ANY, "0.000", wxDefaultPosition, wxDefaultSize, 0 );
	_stext_left->SetFont(DEFAULT_FONT);
	_stext_left->Wrap( -1 );
	bSizerText->Add( _stext_left, 0, wxALIGN_CENTER, 5 );
	
	bSizerText->Add( 0, 0, 1, wxEXPAND, 5 );
	
	_stext_right = new wxStaticText( this, wxID_ANY, "1.000", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	_stext_right->SetFont(DEFAULT_FONT);
	_stext_right->Wrap( -1 );
	_stext_right->SetLabel("1.000");
	bSizerText->Add( _stext_right, 0, wxALIGN_CENTER, 5 );
	
	bSizer->Add( bSizerText, 1, wxEXPAND, 5 );
	
	m_staticText9 = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,  0 );
	m_staticText9->SetFont(DEFAULT_FONT);
	m_staticText9->Wrap( -1 );
	bSizer->Add( m_staticText9, 0, 0, 5 );
	
	wxBoxSizer* bSizerTextCtrl;
	bSizerTextCtrl = new wxBoxSizer( wxHORIZONTAL );
	
	_text_left = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, DEFAULT_TEXTBOX_SIZE, wxTE_PROCESS_ENTER );
	_text_left->SetFont(DEFAULT_FONT);
	//bSizerTextCtrl->Add( _text_left, 0, wxALIGN_LEFT, 5 );
	bSizerTextCtrl->Add( _text_left, 0, wxALIGN_LEFT | wxTOP, 10 );
	
	//bSizerTextCtrl->Add( 0, 0, 1, wxALIGN_LEFT, 5 );
	bSizerTextCtrl->Add( 0, 0, 1, wxALIGN_LEFT | wxTOP, 10 );
	
	_text_percent = new wxTextCtrl( this, wxID_ANY, "0.000", wxDefaultPosition, DEFAULT_TEXTBOX_SIZE, wxTE_PROCESS_ENTER );
	_text_percent->SetFont(DEFAULT_FONT);
	//bSizerTextCtrl->Add( _text_percent, 0, 0, 5 );
	bSizerTextCtrl->Add( _text_percent, 0, wxTOP, 10 );
	
	m_staticText5 = new wxStaticText( this, wxID_ANY, wxT(" %"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->SetFont(DEFAULT_FONT);
	m_staticText5->Wrap( -1 );
	//bSizerTextCtrl->Add( m_staticText5, 0, wxALIGN_CENTER, 5 );
	bSizerTextCtrl->Add( m_staticText5, 0, wxALIGN_CENTER | wxTOP, 10 );
	
	//bSizerTextCtrl->Add( 0, 0, 1, wxALIGN_LEFT, 5 );
	bSizerTextCtrl->Add( 0, 0, 1, wxALIGN_LEFT | wxTOP, 10 );
	
	_text_right = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, DEFAULT_TEXTBOX_SIZE, wxTE_PROCESS_ENTER );
	_text_right->SetFont(DEFAULT_FONT);
	//bSizerTextCtrl->Add( _text_right, 0, wxALIGN_RIGHT, 5 );
	bSizerTextCtrl->Add( _text_right, 0, wxALIGN_RIGHT | wxTOP, 10 );
	
	//bSizer->Add( bSizerTextCtrl, 0, wxEXPAND, 5 );
	bSizer->Add( bSizerTextCtrl, 0, wxEXPAND | wxTOP, 10 );
	
	wxStaticLine *m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer->Add( m_staticline1, 0, wxEXPAND | wxTOP, 5 );

	this->SetSizer( bSizer );
	this->Layout();
	
	// Connect Events
	_text_left->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( RangeSlider::OnTextEnter ), NULL, this );
	_text_percent->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( RangeSlider::OnTextEnter ), NULL, this );
	_text_right->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( RangeSlider::OnTextEnter ), NULL, this );

	_update = false;
	_id = id;
	m_parent = parent;
	_selection = NONE;
	lastx=lasty = -1;

	borderX = 65;
	borderY = 20;

	thumbw = _thumb.GetWidth();
	thumbh = _thumb.GetHeight();
	
	_left = _min = 0.0f; _right = _max = 1.0f;
	SetMin(Min); SetMax(Max);
	SetLeft(_min); SetRight(_max);

	Connect(wxEVT_PAINT, wxPaintEventHandler(RangeSlider::OnPaint));
	Connect(wxEVT_SIZE, wxSizeEventHandler(RangeSlider::OnSize));
	Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(RangeSlider::OnLeftDown));
	Connect(wxEVT_LEFT_UP, wxMouseEventHandler(RangeSlider::OnLeftUp));
	Connect(wxEVT_MOTION, wxMouseEventHandler(RangeSlider::OnMouseMove));
//	Connect(wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler(RangeSlider::OnTextEnter));

	wxSize size = GetSize(); size.x++;
	SetSize(size);

	int tb_h = 20;
	_pbState = PERCENTILE_STATE_OFF;
}
RangeSlider::~RangeSlider()
{
	SAFE_DELETE(_text_left);
	SAFE_DELETE(_text_percent);
	SAFE_DELETE(_stext_left);
	SAFE_DELETE(_text_label);
	SAFE_DELETE(_stext_right);
}

void RangeSlider::OnPaint(wxPaintEvent& event)
{
	//wxPanel::OnPaint(event);
	wxPaintDC dc(this);
	wxSize size = GetSize();
	int w = size.GetWidth(), h = size.GetHeight();
	int step = 50;
	wxString formatstr ="%.4f";

	//Draw slider
	wxBitmap sldr_bitmap = wxBitmap(_slider.Scale(max(1,w-2*borderX-_sldr_left.GetWidth()-_sldr_right.GetWidth()),_slider.GetHeight())); 
	dc.DrawBitmap(sldr_bitmap, borderX+_sldr_left.GetWidth(), h/2-_slider.GetHeight()/2);
	dc.DrawBitmap(_sldr_left,borderX,h/2-_sldr_left.GetHeight()/2);
	dc.DrawBitmap(_sldr_right,w-borderX-_sldr_right.GetWidth(),h/2-_sldr_right.GetHeight()/2);
	
	//Draw selected region
	int left = (w-2*borderX)*(_left-_min)/(_max-_min)+borderX;
	int right= (w-2*borderX)*(_right-_min)/(_max-_min)+borderX;
	dc.DrawBitmap(wxBitmap(_slider_select.Scale(max(right-left,1),_slider_select.GetHeight())),left,h/2-_slider_select.GetHeight()/2	);
	
	//Draw thumbs
	dc.DrawBitmap(_thumb,left-thumbw/2,h/2-thumbh/2);
	dc.DrawBitmap(_thumb,right-thumbw/2,h/2-thumbh/2);
	
	//Draw percentile box and buttons
	wxSize size2 = _text_percent->GetSize(); wxPoint pt = _text_percent->GetPosition();
	int rbSize = 8;
	bool bpressed  = _pbState == PERCENTILE_STATE_TOP;
	bool bpressed2 = _pbState == PERCENTILE_STATE_OFF;
	_text_percent->Enable(!bpressed2);
	drawRadioButton(dc, GetTopButDims(pt,size2,rbSize), !bpressed2 & bpressed);
	drawRadioButton(dc, GetBotButDims(pt,size2,rbSize), !bpressed2 & !bpressed);

	wxBrush brush; brush.SetColour(128,128,128); dc.SetBrush(brush);
	wxPoint pt2 = pt; pt2.x += size2.x; pt2.y += (size2.y-rbSize)/2;
	wxRect rect = GetOffButDims(pt,size2,rbSize);
	dc.DrawRectangle(rect);
	if(bpressed2)
	{
		brush.SetColour(0,0,0); dc.SetBrush(brush);
		dc.DrawLine(rect.x, rect.y, rect.x+rect.width, rect.y+rect.height);
		dc.DrawLine(rect.x, rect.y+rect.height-1, rect.x+rect.width-1, rect.y);
	}
}

void RangeSlider::drawRadioButton(wxPaintDC &dc, wxRect r, bool pressed)
{
	dc.DrawBitmap(_thumb,r.x,r.y);
	if(pressed)
		dc.DrawCircle(r.x+4,r.y+4,2);
}

void RangeSlider::OnSize(wxSizeEvent& event)
{
	// TONY: wxWidgets2.9.1 does not have OnSize for windows anymore
	//wxPanel::OnSize(event);
	wxPanel::Layout();
	Refresh();
}

void RangeSlider::OnLeftDown(wxMouseEvent &event)
{
	int x=event.GetX(), y=event.GetY();
	wxSize size = GetSize();
	int w = size.GetWidth(), h = size.GetHeight();
	float fx = (x-borderX)*(_max-_min)/(w-2*borderX)+_min;
	float tw = thumbw*(_max-_min)/(w-2*borderX);
	// did we click on the left thumb?
	if(fx >= _left-tw/2 && fx <= _left+tw/2  && y >= h/2-thumbh/2 && y <= h/2+thumbh/2)
	{
		_selection = LEFT;
		lastx = x; lasty = y;
	}
	// did we click on the right thumb
	else if(fx >= _right-tw/2 && fx <= _right+tw/2  && y >= h/2-thumbh/2 && y <= h/2+thumbh/2)
	{
		_selection = RIGHT;
		lastx = x; lasty = y;
	}
	// did we click on the range
	else if(fx > _left+tw/2 && fx < _right-tw/2  && y >= h/2-thumbh/2 && y <= h/2+thumbh/2)
	{
		_selection = RANGE;
		lastx = x; lasty = y;
	}
}
bool RangeSlider::getTextBoxValue(wxTextCtrl *tb, float &f)
{
	istringstream sstr((string)tb->GetLineText(0));
	bool b; b = (sstr>>f)?true:false; return b;
}

void RangeSlider::selectTopPercent()
{
	float f;
	if(!getTextBoxValue(_text_percent,f))f=100;
	_pbState = PERCENTILE_STATE_TOP;
	SetLeft(_min + (_max-_min)*(100-f)/100);
	SetRight(_max);
	_update = true;
}

void RangeSlider::selectBotPercent()
{
	float f;
	if(!getTextBoxValue(_text_percent,f))f=0;
	_pbState = PERCENTILE_STATE_BOT;
	SetLeft(_min);
	SetRight(_min + (_max-_min)*f/100);
	_update = true;
}

void RangeSlider::OnLeftUp(wxMouseEvent &event)
{
	int x=event.GetX(), y=event.GetY();
	wxSize size2 = _text_percent->GetSize(); 
	wxPoint pt = _text_percent->GetPosition();
	int rbSize=8;

	//Check if top button is hit
	if(GetTopButDims(pt,size2,rbSize).Contains(x,y))
		selectTopPercent();
	else if(GetBotButDims(pt,size2,rbSize).Contains(x,y))
		selectBotPercent();
	else if(GetOffButDims(pt,size2,rbSize).Contains(x,y))
	{
		_pbState = PERCENTILE_STATE_OFF;
		SetLeft(_min);
		SetRight(_max);
		_update = true;
		//_left = _min; _right = _max;
	}
	if(_update){NotifyAllListeners(PEvent( new Event(RANGE_SLIDER_UPDATED))); Refresh(); }
	_update = false;
	lastx=lasty=-1;
}
void RangeSlider::OnMouseMove(wxMouseEvent &event)
{
	if(_selection == NONE || !event.ButtonIsDown(wxMOUSE_BTN_LEFT) || lastx == -1)
		return;
	wxSize size = GetSize();
	int w = size.GetWidth(), h = size.GetHeight();
	float dx = (event.GetX()-lastx)*(_max-_min)/(w-2*borderX);

	// move the range slider depending on whether the range, left thumb or right thumb were selected
	switch(_selection)
	{
	case LEFT: // move left thumb if we dont overshoot 
		if(_left+dx >= _min && _left+dx <= _right)
			SetLeft(_left +dx);
		_update = true;
		break;
	case RIGHT: // move right thumb if we dont overshoot 
		if(_right +dx >= _left && _right+dx <= _max)
			SetRight(_right + dx);
		_update = true;
		break;
	case RANGE: // select the range
		if(_left+dx >= _min && _right+dx <=_max)
		{
			SetLeft(_left + dx);
			SetRight(_right + dx);
			_update = true;
		}
		break;
	}
	lastx=event.GetX(); 
	if(_update) Refresh();
}
void RangeSlider::Reset(bool bUpdate)
{
	_left = _min; _right = _max;
	if(bUpdate)
		NotifyAllListeners(PEvent( new Event(RANGE_SLIDER_UPDATED)));
	Refresh();
}

void RangeSlider::check_bounds()
{
	if(_left < _min) _left = _min;
	if(_right < _min) _right = _min;
	
	if(_left > _max) _left = _max;
	if(_right > _max) _right = _max;
}

void RangeSlider::OnTextEnter(wxCommandEvent &event)
{
	wxTextCtrl *tc = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if(!tc)return;
	float f;
	char buf[100];
	if(getTextBoxValue(tc,f))
	{
		if(tc == _text_left)
		{
			if( f < _min) f = _min;
			if( f > _right) f = _right;
			SetLeft(f);
		}
		else if(tc == _text_right)
		{
			if( f > _max) f = _max;
			if( f < _left) f = _left;
			SetRight(f);
		}
		else if(tc == _text_percent)
		{
			if(f >=0 && f < 100)
			{
				_pbState == PERCENTILE_STATE_TOP? selectTopPercent():selectBotPercent();
			}
			else
			{
				if(f<0)f=0; if(f > 100)f=100;
			}
		}
		NotifyAllListeners(PEvent( new Event(RANGE_SLIDER_UPDATED)));
		Refresh();
	}
	else
	{
		f = tc == _text_left ? _left : _right;
		wxMessageBox("Should be anumber");
	}
	sprintf(buf,"%.4f",f);
	tc->SetValue(buf);
}
