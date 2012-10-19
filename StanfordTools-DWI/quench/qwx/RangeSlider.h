#ifndef RangeSlider_H
#define RangeSlider_H

#include <wx/wx.h>
#include "typedefs_quench.h"

//! A generic slider class which has a range
class RangeSlider : public wxPanel, public IEventSource
{
public:
	RangeSlider(wxWindow *parent, int id, std::string name, float Min, float Max );
	~RangeSlider();
	
	wxWindow *m_parent;
	//! \return the left end of the slider
	float Left(){return _left;}
	//! \return the right end of the slider
	float Right(){return _right;}
	//! \return the min value of the slider
	float Min(){return _min;}
	//! \return the max value of the slider
	float Max(){return _max;}
	//! get/set the name of the slider
	PROPERTY(std::string , _name, Name);
	//! get/set the ide of the slider
	PROPERTY(int , _id, Id);

	//! Set the left end of the slider
	void  SetLeft (float v){_left  = v; check_bounds(); _text_left ->SetValue(wxString::Format(wxT("%.4f"),_left));}
	//! Set the right end of the slider
	void  SetRight(float v){_right = v; check_bounds(); _text_right->SetValue(wxString::Format(wxT("%.4f"),_right));}
	//! Set the min value of the slider
	void  SetMin  (float v){_min   = v; check_bounds(); _stext_left ->SetLabel(wxString::Format(wxT("%.4f"),_min));}
	//! Set the max value of the slider
	void  SetMax  (float v){_max   = v; check_bounds(); _stext_right->SetLabel(wxString::Format(wxT("%.4f"),_max));}
	//! Reset the slider to span the entire range
	void  Reset(bool bUpdate);
protected:
	//! validate the range of the slider
	void check_bounds();
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);  
	void OnLeftDown(wxMouseEvent &event);
	void OnLeftUp(wxMouseEvent &event);
	void OnMouseMove(wxMouseEvent &event);
	void OnTextEnter(wxCommandEvent& event);

	enum SELTYPE
	{
		LEFT,
		RIGHT,
		RANGE,
		NONE
	};
	SELTYPE _selection;
	int lastx, lasty;
	double _min,_max;
	double _left,_right;
	int    borderX, borderY;
	int    thumbw, thumbh;
	wxBitmap _thumb;
	wxBitmap _sldr_left,_sldr_right;
	wxImage _slider,_slider_select;

	wxStaticText* _text_label;
	wxStaticText* _stext_left;
	wxStaticText* _stext_right;
	wxStaticText* m_staticText9;
	wxTextCtrl* _text_left;
	wxTextCtrl* _text_percent;
	wxStaticText* m_staticText5;
	wxTextCtrl* _text_right;

	bool _update;

	enum PERCENTILE_BOX_STATE
	{
		PERCENTILE_STATE_TOP,
		PERCENTILE_STATE_BOT,
		PERCENTILE_STATE_OFF
	};
	PERCENTILE_BOX_STATE _pbState;
	void drawRadioButton(wxPaintDC &dc, wxRect r, bool pressed=true);
	void selectTopPercent();
	void selectBotPercent();
	bool getTextBoxValue(wxTextCtrl *tb, float &f);
};

#endif
