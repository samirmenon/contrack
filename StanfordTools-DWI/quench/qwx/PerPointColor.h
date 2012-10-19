#ifndef __PER_POINT_COLOR__
#define __PER_POINT_COLOR__

#include "typedefs_quench.h"

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/statline.h>
#include <DTIPathwayStatisticHeader.h>

///////////////////////////////////////////////////////////////////////////
class	RangeSlider;
class ColorMapPanel;

///////////////////////////////////////////////////////////////////////////////
/// Class PerPointColor
///////////////////////////////////////////////////////////////////////////////
class PerPointColor : public wxDialog , public IEventSource, public IEventListener
{
public:
	PerPointColor( wxWindow* parent, ColorMapPanel* cmpanel, wxWindowID id = wxID_ANY, const wxString& title = wxT("Per point color"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 384,167 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
	~PerPointColor();
	virtual bool Show(bool show = true);
	void AddStatistic(DTIPathwayStatisticHeader& header, DTIPathwayStatisticThreshold &threshold);
	void Clear();
	PROPERTY(ColorMapPanel*,_cmapPanel,ColorMapPanel_);

protected:
	wxBitmap *dummy_bmp;
	wxStaticText* m_staticText1;
	wxChoice* m_choice1;
	wxStaticText* m_staticText2;
	wxStaticBitmap* m_bitmap1;
	wxStaticLine* m_staticline1;
	RangeSlider *_rangeSlider;
	int _colormap_index;

	void OnSelectedStatChanged( wxCommandEvent& event );
	void OnClose( wxCloseEvent& event ) { Show(false); }
	void OnColorMapClick(wxMouseEvent& event );
	IListenerList _oldColorMapPanelListeners;
	void OnUpdateColorMap(int colorMap);
	void OnEvent(PEvent e);
	void SetEnabled(bool enabled=true);
	void SetRangeSliderValues();

	std::vector<DTIPathwayStatisticHeader> _statistic_header;
	std::vector<DTIPathwayStatisticThreshold> _statistic_threshold;
};

#endif //__PER_POINT_COLOR__
