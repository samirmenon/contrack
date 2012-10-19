///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////
#include "ColorMapPanel.h"
#include <wx/wx.h>
#include "wx/rawbmp.h"
#include "icons/colormaps_png.cpp"
#include "QuenchFrame.h"

using namespace std;
static const int COLOR_MAP_WIDTH = 96;
static const int COLOR_MAP_HEIGHT = 16;
static const int SINGLE_WIDTH = 128;
static const int SINGLE_HEIGHT = 48;
static const int ASPECT_RATIO = 2;
static const int BORDER_X = (SINGLE_WIDTH  - COLOR_MAP_WIDTH )/2;
static const int BORDER_Y = 10;

static const int COLORMAP_HEIGHT = 28;
static const int COLORMAP_START_X = 130;
static const int COLORMAP_IMAGE_WIDTH = 921;
static const int COLORMAP_WIDTH = COLORMAP_IMAGE_WIDTH - COLORMAP_START_X;
static const int NUM_COLORMAPS = 14;
static const int COLORMAPS_PER_ROW = 4;
static const char COLORMAP_NAMES[][20]={"Gray","Hot","Hsv","Jet","Pink","Spectral","Spring","Summer","Winter","Autumn","Binary","Bone","Cool","Copper"};
///////////////////////////////////////////////////////////////////////////



ColorMapPanel::ColorMapPanel( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	_qFrame = NULL;
	_selectedColormapIndex =0;
	_enabled=true;
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	//Load the colormaps
	unsigned char *pdata = colormaps_raw;
	// for each colomap...
	for(int i = 0; i < NUM_COLORMAPS; i++)
	{
		pdata+=COLORMAP_START_X*3;
		_colorMaps.push_back(ColorMap(COLORMAP_NAMES[i]));
		_colorMaps[i].Colors.push_back(Coloruc(0,0,0,0));

		// read the color map information from the raw bitmap stored in colorsmaps_raw
		for(int j = 0; j < COLORMAP_WIDTH; j++, pdata+=3)
			_colorMaps[i].Colors.push_back(Coloruc(pdata[0],pdata[1],pdata[2],255));
	
		// move the pointer to the next colormap in colormaps_raw
		pdata+=(COLORMAP_HEIGHT-1)*COLORMAP_IMAGE_WIDTH*3;
	}

	// forech colormap...
	for(unsigned i = 0; i < _colorMaps.size(); i++)
	{
		// create a bitmap from the filled data
		wxBitmap *bmp = new wxBitmap(COLOR_MAP_WIDTH, COLOR_MAP_HEIGHT, 32);
		FillBitmapWithColorMap(bmp, i);
		// add the bitmap to the list of bitmaps
		_bitmaps.push_back( PwxBitmap(bmp));
	}
	unsigned y = ((unsigned int)_colorMaps.size()+COLORMAPS_PER_ROW-1)/COLORMAPS_PER_ROW;
	SetSize(COLORMAPS_PER_ROW*SINGLE_WIDTH,y*SINGLE_HEIGHT+35);
	// Connect Events
	this->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( ColorMapPanel::OnKillFocus ) );
	this->Connect( wxEVT_ACTIVATE, wxActivateEventHandler( ColorMapPanel::OnActivate ) );
	this->Connect( wxEVT_PAINT, wxPaintEventHandler( ColorMapPanel::OnPaint ) );
	
	// Note: This used to be listening to the LEFT_UP msg, but that doesn't work for OSX
	//this->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( ColorMapPanel::OnLeftUp ) );
	this->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( ColorMapPanel::OnLeftDown ) );
	
	Show(false); 
}

void ColorMapPanel::OnLeftDown( wxMouseEvent& event )
{
	// normalize the mouse x,y position to a row, col in the colormap grid
	int x, y; event.GetPosition(&x,&y);
	x/=SINGLE_WIDTH; y/=SINGLE_HEIGHT;

	///////////////////////
	//wxString msg;
	//msg= wxString::Format( _T("OnLeftUp!") );
	//wxMessageBox(msg, "degub", wxOK | wxICON_INFORMATION, NULL);
	///////////////////////

	// did we click on an exisitng colormap?
	if(y*COLORMAPS_PER_ROW+x < (int)_colorMaps.size())
	{
		_selectedColormapIndex = y*COLORMAPS_PER_ROW+x;
		// notify the listener that a colormap was clicked
		PEvent evt(new EventColorMapChanged(_selectedColormapIndex));
		NotifyAllListeners(evt);
		Refresh();
		_qFrame->RefreshViz();
	}
}

void ColorMapPanel::FillBitmapWithColorMap(wxBitmap *bmp, int index)
{
	//if(!the_global_frame) return;

	std::vector<Coloruc> &colors = _colorMaps[index].Colors;
	float scale = (float)colors.size()/bmp->GetWidth();

	// create a bitmap with alpha channel
	wxAlphaPixelData data(*bmp, wxPoint(0,0), wxSize(bmp->GetWidth(), bmp->GetHeight()));
	if(data)
	{
		data.UseAlpha();
		wxAlphaPixelData::Iterator p(data);

		// populate the bitmap from the colors stored for this(index) colormap
		for ( int y = 0; y < bmp->GetHeight(); ++y )
    	{
            wxAlphaPixelData::Iterator rowStart = p;
        	for ( int x = 0; x < bmp->GetWidth(); ++x, ++p )
        	{
				p.Red  () = colors[(int)(x*scale)].r;
            	p.Green() = colors[(int)(x*scale)].g;
            	p.Blue () = colors[(int)(x*scale)].b;
		    	p.Alpha() = colors[(int)(x*scale)].a;
        	}
            p = rowStart;
            p.OffsetY(data, 1);
		}
	}
}

void ColorMapPanel::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);
	unsigned rows = ((unsigned int)_colorMaps.size()+COLORMAPS_PER_ROW-1)/COLORMAPS_PER_ROW;
	unsigned cols = COLORMAPS_PER_ROW;

	// for each colormap
	for(int i = 0; i < (int)_colorMaps.size(); i++)
	{
		// figure out the x,y position where the ith colormap should be displayed
		int y = i/cols;
		int x = i-y*cols;
		bool bSelected = i==_selectedColormapIndex;//_item->GetOverlay()->getColorMapIndex();

		// if selected draw a selection box around the colormap
		if(bSelected)
		{
			wxBrush brush; brush.SetColour(0,0,0); dc.SetBrush(brush);
			dc.DrawRectangle(x*SINGLE_WIDTH + BORDER_X-2, y*SINGLE_HEIGHT + BORDER_Y-2, COLOR_MAP_WIDTH+4, COLOR_MAP_HEIGHT+4); 
			brush.SetColour(GetBackgroundColour()); dc.SetBrush(brush);
			dc.DrawRectangle(x*SINGLE_WIDTH + BORDER_X, y*SINGLE_HEIGHT + BORDER_Y, COLOR_MAP_WIDTH, COLOR_MAP_HEIGHT); 
		}
		// draw the bitmap
		dc.DrawBitmap(*_bitmaps[i], x*SINGLE_WIDTH + BORDER_X, y*SINGLE_HEIGHT + BORDER_Y);
		// draw a text showing the name of the colormap
		dc.DrawText(_colorMaps[i].Name.c_str(), x*SINGLE_WIDTH + BORDER_X, y*SINGLE_HEIGHT + BORDER_Y+2+COLOR_MAP_HEIGHT);
	}
}
void ColorMapPanel::OnActivate( wxActivateEvent& event )
{ 
	// hide the colormap panel if tis no longer active 
	if(!event.GetActive())
	{	
		Show(false); 
		PopListeners();
	}
	event.Skip();
}
